ecm_add_test(
    test_completion.cpp
    ../../duchain/tests/duchaintestbase.cpp
    TEST_NAME completiontest
    LINK_LIBRARIES
        kdevphpcompletion
        kdevphpduchain
        kdevphpparser
        Qt5::Test
        KDev::Tests
        KDev::Language
)

ecm_add_test(
    bench_completion.cpp
    ../../duchain/tests/duchaintestbase.cpp
    TEST_NAME completionbenchmark
    LINK_LIBRARIES
        kdevphpcompletion
        kdevphpduchain
        kdevphpparser

        Qt5::Test
        KDev::Tests
        KDev::Language

        # TODO: KF5: KDevice is forcing us to have this dependency. I'm sure
        # we can work-around it :)
        KF5::Archive
)

