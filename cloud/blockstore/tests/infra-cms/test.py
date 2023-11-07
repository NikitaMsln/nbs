import logging
import os
import pytest
import requests
import subprocess
import time

import cloud.blockstore.public.sdk.python.protos as protos

from cloud.blockstore.public.sdk.python.client.error_codes import EResult

from cloud.blockstore.config.server_pb2 import TServerConfig, TServerAppConfig, \
    TKikimrServiceConfig
from cloud.blockstore.config.storage_pb2 import TStorageServiceConfig

from cloud.blockstore.public.sdk.python.client import CreateClient

from cloud.blockstore.tests.python.lib.nbs_runner import LocalNbs
from cloud.blockstore.tests.python.lib.test_base import thread_count, wait_for_nbs_server, \
    wait_for_secure_erase, get_sensor_by_name, get_nbs_counters
from cloud.blockstore.tests.python.lib.nonreplicated_setup import setup_nonreplicated, \
    create_file_devices, setup_disk_registry_config_simple, enable_writable_state

from contrib.ydb.tests.library.harness.kikimr_cluster import kikimr_cluster_factory
from contrib.ydb.tests.library.harness.kikimr_config import KikimrConfigGenerator

import yatest.common as yatest_common

DEFAULT_BLOCK_SIZE = 4096
DEFAULT_DEVICE_COUNT = 4
DEFAULT_BLOCK_COUNT_PER_DEVICE = 262144
DISK_SIZE = 1048576


class CMS:

    def __init__(self, nbs_client):
        self.__nbs_client = nbs_client

    def remove_agent(self, host):
        logging.info('[CMS] Try to remove agent "{}"'.format(host))

        request = protos.TCmsActionRequest()
        action = request.Actions.add()
        action.Type = protos.TAction.REMOVE_HOST
        action.Host = host
        r = self.__nbs_client.cms_action(request)
        return r

    def remove_device(self, host, path):
        logging.info('[CMS] Try to remove device "{}":"{}"'.format(host, path))

        request = protos.TCmsActionRequest()
        action = request.Actions.add()
        action.Type = protos.TAction.REMOVE_DEVICE
        action.Host = host
        action.Device = path
        r = self.__nbs_client.cms_action(request)
        return r


class SessionWithUrlBase(requests.Session):

    def __init__(self, url_base=None, *args, **kwargs):
        super(SessionWithUrlBase, self).__init__(*args, **kwargs)
        self.url_base = url_base

    def request(self, method, url, **kwargs):
        modified_url = self.url_base + url
        return super(SessionWithUrlBase, self).request(method, modified_url, **kwargs)


class TestCmsRemoveAgentNoUserDisks:

    def __init__(self, name):
        self.name = name

    def run(self, storage, nbs, disk_agent, cms, devices):
        response = cms.remove_agent("localhost")

        assert response.ActionResults[0].Timeout == 0

        nbs.create_volume("vol0", return_code=1)

        time.sleep(storage.NonReplicatedInfraTimeout / 1000)

        response = cms.remove_agent("localhost")

        assert response.ActionResults[0].Timeout == 0

        disk_agent.stop()
        nbs.wait_for_stats(AgentsInUnavailableState=1)

        time.sleep(storage.NonReplicatedAgentMaxTimeout / 1000)

        disk_agent.start()
        wait_for_nbs_server(disk_agent.nbs_port)
        nbs.wait_for_stats(AgentsInWarningState=1)

        nbs.change_agent_state("localhost", 0)
        nbs.create_volume("vol0")
        nbs.read_blocks("vol0", start_index=0, block_count=32)

        return True


class TestCmsRemoveAgent:

    def __init__(self, name):
        self.name = name

    def run(self, storage, nbs, disk_agent, cms, devices):
        nbs.create_volume("vol0")

        response = cms.remove_agent("localhost")

        assert response.ActionResults[0].Result.Code == EResult.E_TRY_AGAIN.value
        assert response.ActionResults[0].Timeout != 0

        time.sleep(storage.NonReplicatedInfraTimeout / 1000)

        response = cms.remove_agent("localhost")

        assert response.ActionResults[0].Result.Code == EResult.E_TRY_AGAIN.value
        assert response.ActionResults[0].Timeout != 0

        nbs.destroy_volume("vol0", sync=True)

        response = cms.remove_agent("localhost")

        assert response.ActionResults[0].Timeout == 0

        disk_agent.stop()
        nbs.wait_for_stats(AgentsInUnavailableState=1)

        disk_agent.start()
        wait_for_nbs_server(disk_agent.nbs_port)
        nbs.wait_for_stats(AgentsInWarningState=1)

        nbs.change_agent_state("localhost", 0)

        nbs.create_volume("vol1")
        nbs.read_blocks("vol1", start_index=0, block_count=32)

        return True


class TestCmsRemoveDevice:

    def __init__(self, name):
        self.name = name

    def run(self, storage, nbs, disk_agent, cms, devices):
        nbs.create_volume("vol0")

        device = devices[0]

        response = cms.remove_device("localhost", device.path)

        assert response.ActionResults[0].Result.Code == EResult.E_TRY_AGAIN.value
        assert response.ActionResults[0].Timeout != 0

        time.sleep(storage.NonReplicatedInfraTimeout / 1000)

        response = cms.remove_device("localhost", device.path)

        assert response.ActionResults[0].Result.Code == EResult.E_TRY_AGAIN.value
        assert response.ActionResults[0].Timeout != 0

        nbs.change_device_state(device.uuid, 2)

        response = cms.remove_device("localhost", device.path)

        assert response.ActionResults[0].Timeout == 0

        nbs.change_device_state(device.uuid, 0)

        nbs.destroy_volume("vol0", sync=True)

        nbs.create_volume("vol1")
        nbs.read_blocks("vol1", start_index=0, block_count=32)

        return True


class TestCmsRemoveDeviceNoUserDisks:

    def __init__(self, name):
        self.name = name

    def run(self, storage, nbs, disk_agent, cms, devices):
        device = devices[0]

        response = cms.remove_device("localhost", device.path)

        assert response.ActionResults[0].Timeout == 0

        nbs.create_volume("vol0", return_code=1)
        nbs.destroy_volume("vol0")

        time.sleep(storage.NonReplicatedInfraTimeout / 1000)

        response = cms.remove_device("localhost", device.path)

        assert response.ActionResults[0].Timeout == 0

        nbs.change_device_state(device.uuid, 2)
        nbs.change_device_state(device.uuid, 0)

        nbs.create_volume("vol1")
        nbs.read_blocks("vol1", start_index=0, block_count=32)

        return True


TESTS = [
    TestCmsRemoveAgentNoUserDisks("removeagentnodisks"),
    TestCmsRemoveAgent("removeagent"),
    TestCmsRemoveDevice("removedevice"),
    TestCmsRemoveDeviceNoUserDisks("removedevicenodisks")
]


class Nbs(LocalNbs):
    BINARY_PATH = yatest_common.binary_path("cloud/blockstore/apps/client/blockstore-client")

    def create_volume(self, disk_id, return_code=0):
        logging.info('[NBS] create volume "%s"' % disk_id)

        self.__rpc(
            "createvolume",
            "--disk-id", disk_id,
            "--blocks-count", str(DISK_SIZE),
            "--storage-media-kind", "nonreplicated",
            return_code=return_code)

    def destroy_volume(self, disk_id, sync=False):
        logging.info('[NBS] destroy volume "%s"' % disk_id)

        args = ["destroyvolume", "--disk-id", disk_id]

        if sync:
            args.append("--sync")

        self.__rpc(*args, input=disk_id, stdout=subprocess.PIPE)

    def read_blocks(self, disk_id, start_index, block_count):
        logging.info('[NBS] read blocks (%d:%d) from "%s"' % (start_index, block_count, disk_id))

        self.__rpc(
            "readblocks",
            "--disk-id", disk_id,
            "--start-index", str(start_index),
            "--blocks-count", str(block_count))

    def change_agent_state(self, agent_id, state):
        logging.info('[NBS] change agent "%s" state to %d' % (agent_id, state))

        self.__rpc(
            "executeaction",
            "--action", "diskregistrychangestate",
            "--input-bytes", "{'Message': 'test', 'ChangeAgentState': {'AgentId': '%s', 'State': %d}}" % (agent_id, state),
            stdout=subprocess.PIPE)

    def change_device_state(self, device_id, state):
        logging.info('[NBS] change device "%s" state to %d' % (device_id, state))

        self.__rpc(
            "executeaction",
            "--action", "diskregistrychangestate",
            "--input-bytes", "{'Message': 'test', 'ChangeDeviceState': {'DeviceUUID': '%s', 'State': %d}}" % (device_id, state),
            stdout=subprocess.PIPE)

    def __rpc(self, *args, **kwargs):
        input = kwargs.get("input")
        return_code = kwargs.get("return_code", 0)

        if input is not None:
            input = (input + "\n").encode("utf8")

        args = [self.BINARY_PATH] + list(args) + [
            "--host", "localhost",
            "--port", str(self.nbs_port),
            "--verbose", "error"]

        process = subprocess.Popen(
            args,
            stdin=subprocess.PIPE,
            stdout=kwargs.get("stdout"),
            stderr=subprocess.PIPE)

        outs, errs = process.communicate(input=input)

        if outs:
            logging.info("blockstore-client's output: {}".format(outs))

        if errs:
            logging.error("blockstore-client's errors: {}".format(errs))

        assert process.returncode == return_code

    def wait_for_stats(self, **kwargs):
        logging.info('wait for stats: {} ...'.format(kwargs))

        while True:
            sensors = get_nbs_counters(self.mon_port)['sensors']
            satisfied = set()

            for name, expected in kwargs.items():
                val = get_sensor_by_name(sensors, 'disk_registry', name)
                if val != expected:
                    break
                satisfied.add(name)

            if len(kwargs) == len(satisfied):
                break
            time.sleep(1)


def __run_test(test_case):
    kikimr_binary_path = yatest_common.binary_path("contrib/ydb/apps/ydbd/ydbd")

    configurator = KikimrConfigGenerator(
        erasure=None,
        binary_path=kikimr_binary_path,
        has_cluster_uuid=False,
        use_in_memory_pdisks=True,
        dynamic_storage_pools=[
            dict(name="dynamic_storage_pool:1", kind="hdd", pdisk_user_kind=0),
            dict(name="dynamic_storage_pool:2", kind="ssd", pdisk_user_kind=0)
        ])

    nbs_binary_path = yatest_common.binary_path("cloud/blockstore/apps/server/nbsd")

    kikimr_cluster = kikimr_cluster_factory(configurator=configurator)
    kikimr_cluster.start()

    kikimr_port = list(kikimr_cluster.nodes.values())[0].port

    devices = create_file_devices(
        None,   # dir
        DEFAULT_DEVICE_COUNT,
        DEFAULT_BLOCK_SIZE,
        DEFAULT_BLOCK_COUNT_PER_DEVICE)

    setup_nonreplicated(kikimr_cluster.client, [devices])

    server_app_config = TServerAppConfig()
    server_app_config.ServerConfig.CopyFrom(TServerConfig())
    server_app_config.ServerConfig.ThreadsCount = thread_count()
    server_app_config.ServerConfig.StrictContractValidation = False
    server_app_config.ServerConfig.NodeType = 'main'
    server_app_config.KikimrServiceConfig.CopyFrom(TKikimrServiceConfig())

    certs_dir = yatest_common.source_path('cloud/blockstore/tests/certs')

    server_app_config.ServerConfig.RootCertsFile = os.path.join(certs_dir, 'server.crt')
    cert = server_app_config.ServerConfig.Certs.add()
    cert.CertFile = os.path.join(certs_dir, 'server.crt')
    cert.CertPrivateKeyFile = os.path.join(certs_dir, 'server.key')

    storage = TStorageServiceConfig()
    storage.AllocationUnitNonReplicatedSSD = 1
    storage.AcquireNonReplicatedDevices = True
    storage.ClientRemountPeriod = 1000
    storage.NonReplicatedInfraTimeout = 60000
    storage.NonReplicatedAgentMinTimeout = 3000
    storage.NonReplicatedAgentMaxTimeout = 3000
    storage.NonReplicatedDiskRecyclingPeriod = 5000
    storage.DisableLocalService = False

    nbs = Nbs(
        kikimr_port,
        configurator.domains_txt,
        server_app_config=server_app_config,
        storage_config_patches=[storage],
        enable_tls=True,
        kikimr_binary_path=kikimr_binary_path,
        nbs_binary_path=nbs_binary_path)

    nbs.start()
    wait_for_nbs_server(nbs.nbs_port)

    nbs_client_binary_path = yatest_common.binary_path("cloud/blockstore/apps/client/blockstore-client")
    enable_writable_state(nbs.nbs_port, nbs_client_binary_path)
    setup_disk_registry_config_simple(
        devices,
        nbs.nbs_port,
        nbs_client_binary_path)

    nbs_client = CreateClient("localhost:{}".format(nbs.nbs_port))

    # node with DiskAgent

    server_app_config.ServerConfig.NodeType = 'disk-agent'
    storage.DisableLocalService = True

    disk_agent = LocalNbs(
        kikimr_port,
        configurator.domains_txt,
        server_app_config=server_app_config,
        storage_config_patches=[storage],
        enable_tls=True,
        kikimr_binary_path=kikimr_binary_path,
        nbs_binary_path=nbs_binary_path)

    disk_agent.start()
    wait_for_nbs_server(disk_agent.nbs_port)

    # wait for DiskAgent registration & secure erase
    wait_for_secure_erase(nbs.mon_port)

    try:
        ret = test_case.run(storage, nbs, disk_agent, CMS(nbs_client), devices)
    finally:
        nbs.stop()
        disk_agent.stop()
        kikimr_cluster.stop()
        logging.info("Remove temporary device files")
        for d in devices:
            d.handle.close()
            os.unlink(d.path)

    return ret


@pytest.mark.parametrize("test_case", TESTS, ids=[x.name for x in TESTS])
def test_cms(test_case):
    assert __run_test(test_case) is True