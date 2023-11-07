#pragma once

#include <contrib/ydb/library/yql/providers/common/gateway/yql_provider_gateway.h>
#include <contrib/ydb/library/yql/providers/dq/api/grpc/api.grpc.pb.h>

#include <util/generic/string.h>

#include <library/cpp/actors/core/actor.h>
#include <library/cpp/threading/future/future.h>
#include <library/cpp/grpc/client/grpc_client_low.h>

namespace NYql {

class IServiceNodeResolver {
public:
    using TPtr = std::shared_ptr<IServiceNodeResolver>;

    struct TConnectionResult : public NCommon::TOperationResult {

        TConnectionResult() {}

        TConnectionResult(
            std::unique_ptr<NGrpc::TServiceConnection<Yql::DqsProto::DqService>>&& connection,
            std::shared_ptr<NGrpc::IQueueClientContext>&& ctx = nullptr,
            ui32 nodeId = 0,
            const TString& location = "")
            : Connection(connection.release())
            , GRpcContext(std::move(ctx))
            , NodeId(nodeId)
            , Location(location)
        {
            if (GRpcContext) {
                SetSuccess();
            }
        }

        std::shared_ptr<NGrpc::TServiceConnection<Yql::DqsProto::DqService>> Connection;
        std::shared_ptr<NGrpc::IQueueClientContext> GRpcContext;
        ui32 NodeId;
        TString Location;
    };

    virtual ~IServiceNodeResolver() = default;
    virtual NThreading::TFuture<TConnectionResult> GetConnection() = 0;
    virtual void InvalidateCache() = 0;
    virtual void Stop() = 0;
};

struct TDynamicResolverOptions {
    NActors::TActorId YtWrapper;
    TString Prefix;
    TDuration UpdatePeriod = TDuration::Seconds(5);
    TDuration RetryPeriod = TDuration::Seconds(10);
};

class TSingleNodeResolver: public IServiceNodeResolver {
public:
    TSingleNodeResolver();

    ~TSingleNodeResolver();

    NThreading::TFuture<IServiceNodeResolver::TConnectionResult> GetConnection() override;

    void SetLeaderHostPort(const TString& leaderHostPort);

    void InvalidateCache() override { }

    void Stop() override { }

private:
    NGrpc::TGRpcClientLow ClientLow;
    NGrpc::TChannelPool ChannelPool;

    TString LeaderHostPort;
};

IServiceNodeResolver::TPtr CreateStaticResolver(const TVector<TString>& hostPortPairs);
IServiceNodeResolver::TPtr CreateDynamicResolver(NActors::TActorSystem* actorSystem, const TDynamicResolverOptions& options);

} // namespace NYql