LIBRARY()

SRCS(
    activity.h
    common.h
    defs.h
    env.h
    node_warden_mock_bsc.cpp
    node_warden_mock.h
    node_warden_mock_pipe.cpp
    node_warden_mock_state.cpp
    node_warden_mock_state.h
    node_warden_mock_vdisk.h
)

PEERDIR(
    library/cpp/digest/md5
    library/cpp/testing/unittest
    contrib/ydb/core/base
    contrib/ydb/core/blob_depot
    contrib/ydb/core/blobstorage/backpressure
    contrib/ydb/core/blobstorage/dsproxy/mock
    contrib/ydb/core/blobstorage/nodewarden
    contrib/ydb/core/blobstorage/pdisk
    contrib/ydb/core/blobstorage/pdisk/mock
    contrib/ydb/core/blobstorage/vdisk/common
    contrib/ydb/core/mind
    contrib/ydb/core/mind/bscontroller
    contrib/ydb/core/mind/hive
    contrib/ydb/core/sys_view/service
    contrib/ydb/core/tx/scheme_board
    contrib/ydb/core/tx/tx_allocator
    contrib/ydb/core/tx/mediator
    contrib/ydb/core/tx/coordinator
    contrib/ydb/core/tx/scheme_board
    contrib/ydb/core/util
    contrib/ydb/library/yql/public/udf/service/exception_policy
    contrib/ydb/library/yql/sql/pg_dummy
)

END()