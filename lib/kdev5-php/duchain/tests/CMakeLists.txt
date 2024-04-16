include_directories(${CMAKE_BINARY_DIR}/completion)
ecm_add_test(
    duchain.cpp
    duchaintestbase.cpp
    TEST_NAME duchain
    LINK_LIBRARIES
        kdevphpduchain
        kdevphpparser
        Qt6::Test
        KDev::Tests
        KDev::Language
)

set(expressionparser_SRCS)
ecm_add_test(
    expressionparser.cpp
    duchaintestbase.cpp
    TEST_NAME expressionparser
    LINK_LIBRARIES
        kdevphpduchain
        kdevphpparser
        Qt6::Test
        KDev::Tests
        KDev::Language
)

ecm_add_test(
    uses.cpp
    duchaintestbase.cpp
    TEST_NAME uses
    LINK_LIBRARIES
        kdevphpduchain
        kdevphpparser
        Qt6::Test
        KDev::Tests
        KDev::Language
        KDev::Interfaces
)
# don't run benchmark as part of the unit test suite (takes too long, doesn't increase test coverage)
add_executable(benchmarks
    benchmarks.cpp
    duchaintestbase.cpp
)
target_link_libraries(benchmarks
    kdevphpduchain
    kdevphpparser
    Qt6::Test
    KDev::Tests
    KDev::Language
    KDev::Interfaces
)

set(duchain_multiplefiles_SRCS
)
ecm_add_test(
    duchain_multiplefiles.cpp
    duchaintestbase.cpp
    TEST_NAME duchain_multiplefiles
    LINK_LIBRARIES
        kdevphpduchain
        kdevphpparser
        Qt6::Test
        KDev::Tests
        KDev::Language
        KDev::Interfaces
)

