PY3TEST()

INCLUDE(${ARCADIA_ROOT}/cloud/storage/core/tests/recipes/medium.inc)
SPLIT_FACTOR(1)

DEPENDS(
    cloud/storage/core/libs/endpoints/keyring/ut/bin
    cloud/storage/core/libs/diagnostics/netlink_ut/bin
)

TEST_SRCS(
    test.py
)

INCLUDE(${ARCADIA_ROOT}/cloud/storage/core/tests/recipes/qemu.inc)

END()
