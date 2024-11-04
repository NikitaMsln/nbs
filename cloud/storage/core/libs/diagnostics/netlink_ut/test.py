import yatest.common as common

tests_bin = "storage-core-libs-diagnostics-netlink_ut-bin"
tests_bin_path = "cloud/storage/core/libs/diagnostics/netlink_ut/bin/" + tests_bin


def test_qemu_netlink_ut():
    test_tool = common.binary_path(tests_bin_path)
    common.execute(test_tool)
