#pragma once

#include <contrib/ydb/core/kqp/opt/kqp_opt.h>
#include <contrib/ydb/core/kqp/opt/logical/kqp_opt_cbo.h>

namespace NKikimr::NKqp::NOpt {

struct TKqpOptimizeContext;

TAutoPtr<NYql::IGraphTransformer> CreateKqpLogOptTransformer(const TIntrusivePtr<TKqpOptimizeContext>& kqpCtx,
    NYql::TTypeAnnotationContext& typesCtx, const NYql::TKikimrConfiguration::TPtr& config, 
    NKikimr::NKqp::NOpt::TKqpProviderContext& pctx);

} // namespace NKikimr::NKqp::NOpt
