#pragma once

#include <contrib/ydb/core/protos/tx_columnshard.pb.h>
#include <contrib/libs/apache/arrow/cpp/src/arrow/scalar.h>
#include <contrib/libs/apache/arrow/cpp/src/arrow/type.h>
#include <memory>

namespace NKikimr::NOlap {

void ScalarToConstant(const arrow::Scalar& scalar, NKikimrSSA::TProgram_TConstant& value);
std::shared_ptr<arrow::Scalar> ConstantToScalar(const NKikimrSSA::TProgram_TConstant& value,
                                                const std::shared_ptr<arrow::DataType>& type);

TString SerializeKeyScalar(const std::shared_ptr<arrow::Scalar>& key);
std::shared_ptr<arrow::Scalar> DeserializeKeyScalar(const TString& key, const std::shared_ptr<arrow::DataType>& type);

}
