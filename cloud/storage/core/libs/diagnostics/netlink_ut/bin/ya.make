UNITTEST_FOR(cloud/storage/core/libs/diagnostics)

IF (OS_LINUX)
    SRCS(
        stats_fetcher_ut.cpp
    )
ENDIF()

END()