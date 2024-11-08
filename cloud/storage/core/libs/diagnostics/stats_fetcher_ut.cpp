#include "cgroup_stats_fetcher.h"

#include "critical_events.h"

#include <cloud/storage/core/libs/diagnostics/monitoring.h>
#include <cloud/storage/core/libs/diagnostics/logging.h>

#include <library/cpp/monlib/dynamic_counters/counters.h>
#include <library/cpp/testing/unittest/registar.h>

#include <util/system/tempfile.h>

namespace NCloud::NStorage {

namespace {

////////////////////////////////////////////////////////////////////////////////

const TString ComponentName = "STORAGE_CGROUPS";

};  //namespace

////////////////////////////////////////////////////////////////////////////////

Y_UNIT_TEST_SUITE(StatFetcherTest)
{
    Y_UNIT_TEST(ShouldGetCpuDelay)
    {
        auto monitoring = CreateMonitoringServiceStub();
        auto fetcher = CreateKernelTaskDelayAcctStatsFetcher(
            ComponentName,
            CreateLoggingService("console"),
            monitoring);
        fetcher->Start();
        auto cpuWait = fetcher->GetCpuWait();
        UNIT_ASSERT_C(!HasError(cpuWait), cpuWait.GetError());
        fetcher->Stop();
    }
}

}   // namespace NCloud::NStorage
