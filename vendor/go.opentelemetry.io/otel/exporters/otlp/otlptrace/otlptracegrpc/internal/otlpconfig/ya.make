GO_LIBRARY()

SUBSCRIBER(g:go-contrib)

LICENSE(Apache-2.0)

SRCS(
    envconfig.go
    options.go
    optiontypes.go
    tls.go
)

GO_TEST_SRCS(options_test.go)

END()

RECURSE(
    gotest
)
