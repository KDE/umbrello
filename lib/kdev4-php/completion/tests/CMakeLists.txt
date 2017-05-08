set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )


include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}
)

set(completiontest_SRCS test_completion.cpp
    ../../duchain/tests/duchaintestbase.cpp)
kde4_add_unit_test(completiontest ${completiontest_SRCS})
target_link_libraries(completiontest
    kdev4phpcompletion
    kdev4phpduchain
    kdev4phpparser
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)
# hack to make ../../duchain/tests/duchaintestbase.h use the correct EXPORT tags when compiling with MinGW
if (MINGW)
    set_property(TARGET completiontest APPEND_STRING PROPERTY COMPILE_FLAGS " -DMAKE_KDEV4PHPDUCHAIN_LIB")
endif (MINGW)


set(completionbench_SRCS bench_completion.cpp
    ../../duchain/tests/duchaintestbase.cpp)
kde4_add_unit_test(completionbenchmark ${completionbench_SRCS})
target_link_libraries(completionbenchmark
    kdev4phpcompletion
    kdev4phpduchain
    kdev4phpparser
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)
# hack to make ../../duchain/tests/duchaintestbase.h use the correct EXPORT tags when compiling with MinGW
if (MINGW)
    set_property(TARGET completionbenchmark APPEND_STRING PROPERTY COMPILE_FLAGS " -DMAKE_KDEV4PHPDUCHAIN_LIB")
endif (MINGW)
