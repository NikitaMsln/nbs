#include "with_appended.h"
#include <contrib/ydb/core/tx/columnshard/blob_cache.h>
#include <contrib/ydb/core/tx/columnshard/columnshard_impl.h>
#include <contrib/ydb/core/tx/columnshard/engines/column_engine_logs.h>
#include <contrib/ydb/core/tx/columnshard/splitter/rb_splitter.h>

namespace NKikimr::NOlap {

void TChangesWithAppend::DoWriteIndex(NColumnShard::TColumnShard& self, TWriteIndexContext& /*context*/) {
    for (auto& portionInfo : AppendedPortions) {
        switch (portionInfo.GetPortionInfo().GetMeta().Produced) {
            case NOlap::TPortionMeta::EProduced::UNSPECIFIED:
                Y_ABORT_UNLESS(false); // unexpected
            case NOlap::TPortionMeta::EProduced::INSERTED:
                self.IncCounter(NColumnShard::COUNTER_INDEXING_PORTIONS_WRITTEN);
                break;
            case NOlap::TPortionMeta::EProduced::COMPACTED:
                self.IncCounter(NColumnShard::COUNTER_COMPACTION_PORTIONS_WRITTEN);
                break;
            case NOlap::TPortionMeta::EProduced::SPLIT_COMPACTED:
                self.IncCounter(NColumnShard::COUNTER_SPLIT_COMPACTION_PORTIONS_WRITTEN);
                break;
            case NOlap::TPortionMeta::EProduced::EVICTED:
                Y_ABORT("Unexpected evicted case");
                break;
            case NOlap::TPortionMeta::EProduced::INACTIVE:
                Y_ABORT("Unexpected inactive case");
                break;
        }
    }
    self.IncCounter(NColumnShard::COUNTER_PORTIONS_DEACTIVATED, PortionsToRemove.size());

    THashSet<TUnifiedBlobId> blobsDeactivated;
    for (auto& [_, portionInfo] : PortionsToRemove) {
        for (auto& rec : portionInfo.Records) {
            blobsDeactivated.insert(rec.BlobRange.BlobId);
        }
        self.IncCounter(NColumnShard::COUNTER_RAW_BYTES_DEACTIVATED, portionInfo.RawBytesSum());
    }

    self.IncCounter(NColumnShard::COUNTER_BLOBS_DEACTIVATED, blobsDeactivated.size());
    for (auto& blobId : blobsDeactivated) {
        self.IncCounter(NColumnShard::COUNTER_BYTES_DEACTIVATED, blobId.BlobSize());
    }
}

bool TChangesWithAppend::DoApplyChanges(TColumnEngineForLogs& self, TApplyChangesContext& context) {
    // Save new portions (their column records)
    {
        auto g = self.GranulesStorage->StartPackModification();
        THashSet<ui64> usedPortionIds;
        for (auto& [_, portionInfo] : PortionsToRemove) {
            Y_ABORT_UNLESS(!portionInfo.Empty());
            Y_ABORT_UNLESS(portionInfo.HasRemoveSnapshot());

            const TPortionInfo& oldInfo = self.GetGranuleVerified(portionInfo.GetPathId()).GetPortionVerified(portionInfo.GetPortion());
            AFL_VERIFY(usedPortionIds.emplace(portionInfo.GetPortionId()).second)("portion_info", portionInfo.DebugString(true));
            self.UpsertPortion(portionInfo, &oldInfo);

            portionInfo.SaveToDatabase(context.DB);
        }
        for (auto& portionInfoWithBlobs : AppendedPortions) {
            auto& portionInfo = portionInfoWithBlobs.GetPortionInfo();
            Y_ABORT_UNLESS(!portionInfo.Empty());
            AFL_VERIFY(usedPortionIds.emplace(portionInfo.GetPortionId()).second)("portion_info", portionInfo.DebugString(true));
            self.UpsertPortion(portionInfo);
            portionInfo.SaveToDatabase(context.DB);
        }
    }

    for (auto& [_, portionInfo] : PortionsToRemove) {
        self.CleanupPortions[portionInfo.GetRemoveSnapshot()].emplace_back(portionInfo);
    }

    return true;
}

void TChangesWithAppend::DoCompile(TFinalizationContext& context) {
    for (auto&& i : AppendedPortions) {
        i.GetPortionInfo().SetPortion(context.NextPortionId());
        i.GetPortionInfo().UpdateRecordsMeta(TPortionMeta::EProduced::INSERTED);
    }
    for (auto& [_, portionInfo] : PortionsToRemove) {
        if (!portionInfo.HasRemoveSnapshot()) {
            portionInfo.SetRemoveSnapshot(context.GetSnapshot());
        }
    }
}

std::vector<TPortionInfoWithBlobs> TChangesWithAppend::MakeAppendedPortions(const std::shared_ptr<arrow::RecordBatch> batch,
    const ui64 pathId, const TSnapshot& snapshot, const TGranuleMeta* granuleMeta, TConstructionContext& context) const {
    Y_ABORT_UNLESS(batch->num_rows());

    auto resultSchema = context.SchemaVersions.GetSchema(snapshot);

    std::shared_ptr<NOlap::TSerializationStats> stats = std::make_shared<NOlap::TSerializationStats>();
    if (granuleMeta) {
        stats = granuleMeta->BuildSerializationStats(resultSchema);
    }
    auto schema = std::make_shared<TDefaultSchemaDetails>(resultSchema, SaverContext, stats);
    std::vector<TPortionInfoWithBlobs> out;
    {
        std::vector<TBatchSerializedSlice> pages = TRBSplitLimiter::BuildSimpleSlices(batch, SplitSettings, context.Counters.SplitterCounters, schema);
        std::vector<TGeneralSerializedSlice> generalPages;
        for (auto&& i : pages) {
            std::map<ui32, std::vector<std::shared_ptr<IPortionDataChunk>>> portionColumns = i.GetPortionChunks();
            resultSchema->GetIndexInfo().AppendIndexes(portionColumns);
            generalPages.emplace_back(portionColumns, schema, context.Counters.SplitterCounters, SplitSettings);
        }

        TSimilarSlicer slicer(SplitSettings.GetExpectedPortionSize());
        auto packs = slicer.Split(generalPages);

        ui32 recordIdx = 0;
        for (auto&& i : packs) {
            TGeneralSerializedSlice slice(std::move(i));
            auto b = batch->Slice(recordIdx, slice.GetRecordsCount());
            std::vector<std::vector<std::shared_ptr<IPortionDataChunk>>> chunksByBlobs = slice.GroupChunksByBlobs();
            out.emplace_back(TPortionInfoWithBlobs::BuildByBlobs(chunksByBlobs, nullptr, pathId, snapshot, SaverContext.GetStorageOperator()));
            NArrow::TFirstLastSpecialKeys primaryKeys(slice.GetFirstLastPKBatch(resultSchema->GetIndexInfo().GetReplaceKey()));
            NArrow::TMinMaxSpecialKeys snapshotKeys(b, TIndexInfo::ArrowSchemaSnapshot());
            out.back().GetPortionInfo().AddMetadata(*resultSchema, primaryKeys, snapshotKeys, SaverContext.GetTierName());
            recordIdx += slice.GetRecordsCount();
        }
    }

    return out;
}

void TChangesWithAppend::DoStart(NColumnShard::TColumnShard& /*self*/) {
}

}
