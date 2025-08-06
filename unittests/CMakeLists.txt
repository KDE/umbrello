set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(SRC_PATH ../umbrello)
include(ECMAddTests)

set(ENVIRONMENT ENVIRONMENT LANG=C.UTF-8)

include_directories(
      ${LIBXML2_INCLUDE_DIR}
      ${LIBXSLT_INCLUDE_DIR}
      ${CMAKE_SOURCE_DIR}
      ${SRC_PATH}
      ${SRC_PATH}/debug/
      ${SRC_PATH}/dialogs/
      ${SRC_PATH}/dialogs/pages
      ${SRC_PATH}/dialogs/widgets
      ${SRC_PATH}/clipboard
      ${SRC_PATH}/cmds
      ${SRC_PATH}/codegenerators
      ${SRC_PATH}/codegenerators/ada/
      ${SRC_PATH}/codegenerators/as/
      ${SRC_PATH}/codegenerators/cpp/
      ${SRC_PATH}/codegenerators/csharp/
      ${SRC_PATH}/codegenerators/d/
      ${SRC_PATH}/codegenerators/idl/
      ${SRC_PATH}/codegenerators/java/
      ${SRC_PATH}/codegenerators/js/
      ${SRC_PATH}/codegenerators/pascal/
      ${SRC_PATH}/codegenerators/perl/
      ${SRC_PATH}/codegenerators/php/
      ${SRC_PATH}/codegenerators/python/
      ${SRC_PATH}/codegenerators/ruby/
      ${SRC_PATH}/codegenerators/sql/
      ${SRC_PATH}/codegenerators/tcl/
      ${SRC_PATH}/codegenerators/vala/
      ${SRC_PATH}/codegenerators/xml/
      ${SRC_PATH}/codegenwizard
      ${SRC_PATH}/codeimport
      ${SRC_PATH}/debug
      ${SRC_PATH}/dialogs
      ${SRC_PATH}/docgenerators
      ${SRC_PATH}/menus/
      ${SRC_PATH}/refactoring
      ${SRC_PATH}/umlmodel/
      ${SRC_PATH}/umlwidgets/
      ${CMAKE_CURRENT_BINARY_DIR}
)

set(LIBS
    Qt5::Xml
    Qt5::Test
    Qt5::Widgets
    KF5::I18n
    KF5::Crash
    ${LIBXML2_LIBRARIES}
    ${LIBXSLT_LIBRARIES}
    libumbrello
)

ecm_add_test(
    testbasictypes.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testbasictypes
    ${ENVIRONMENT}
)

ecm_add_test(
    testumlobject.cpp
    testbase.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testumlobject
    ${ENVIRONMENT}
)

ecm_add_test(
    testassociation.cpp
    testbase.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testassociation
    ${ENVIRONMENT}
)

ecm_add_test(
    testclassifier.cpp
    testbase.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testclassifier
    ${ENVIRONMENT}
)

ecm_add_test(
    testpackage.cpp
    testbase.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testpackage
    ${ENVIRONMENT}
)

ecm_add_test(
    testcppwriter.cpp
    testbase.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testcppwriter
    ${ENVIRONMENT}
)

ecm_add_test(
    testpythonwriter.cpp
    testbase.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testpythonwriter
    ${ENVIRONMENT}
)

ecm_add_test(
    testoptionstate.cpp
    testbase.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testoptionstate
    ${ENVIRONMENT}
)

ecm_add_test(
    testumlcanvasobject.cpp
    testbase.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testumlcanvasobject
    ${ENVIRONMENT}
)

ecm_add_test(
    testpreconditionwidget.cpp
    testbase.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testpreconditionwidget
    ${ENVIRONMENT}
)

ecm_add_test(
    testwidgetbase.cpp
    testbase.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testwidgetbase
    ${ENVIRONMENT}
)

set(testumlroledialog_SRCS
    testumlroledialog.cpp
)

add_executable(testumlroledialog ${testumlroledialog_SRCS})
target_link_libraries(testumlroledialog ${LIBS})

add_executable(testcrashhandler testcrashhandler.cpp)
target_link_libraries(testcrashhandler ${LIBS})

add_executable(testlistpopupmenu testlistpopupmenu.cpp testbase.cpp)
target_link_libraries(testlistpopupmenu ${LIBS})

find_package(LLVM CONFIG)
find_package(Clang CONFIG)
if(NOT Clang_FOUND)
    find_package(CLANG QUIET)
endif()
if(LLVM_FOUND AND (Clang_FOUND OR CLANG_FOUND))
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

    message(STATUS "Found CLANG ${CLANG_PACKAGE_VERSION}")
    if(LLVM_PACKAGE_VERSION VERSION_LESS "9.0.0")
        set(ENABLE_LLVM_TESTCASES 1)
    endif()
endif()
if(ENABLE_LLVM_TESTCASES AND (NOT WIN32))
    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})

    include_directories(${CLANG_INCLUDE_DIRS})
    add_definitions(${CLANG_DEFINITIONS})

    # Now build our tools
    add_executable(testllvm testllvm.cpp)

    # Find the libraries that correspond to the LLVM components
    # that we wish to use
    if(LLVM_PACKAGE_VERSION VERSION_GREATER_EQUAL "7.0.0")
        set(llvm_libs LLVM)
    else()
        llvm_map_components_to_libnames(llvm_libs support core irreader analysis)
    endif()

    # Link against LLVM libraries
    target_link_libraries(testllvm ${llvm_libs} clangFrontend clangTooling clangBasic clangSerialization)

    add_executable(testllvmparser testllvmparser.cpp)
    if(LLVM_PACKAGE_VERSION VERSION_LESS "7.0.0")
        llvm_map_components_to_libnames(llvm_libs support)
    endif()
    target_link_libraries(testllvmparser ${llvm_libs} clangFrontend clangTooling clangAST clangBasic clangSerialization ${LIBS})
    ecm_mark_nongui_executable(testllvm testllvmparser)
endif()

add_custom_target(check COMMAND ${CMAKE_BUILD_TOOL} test)
