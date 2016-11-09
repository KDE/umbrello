
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(SRC_PATH ../umbrello)

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
      ${SRC_PATH}/refactoring
      ${SRC_PATH}/umlmodel/
      ${SRC_PATH}/umlwidgets/
      ${CMAKE_CURRENT_BINARY_DIR}
)
    set(LIBS
        Qt5::Xml
        Qt5::Test
        Qt5::Widgets
        Qt5::WebKitWidgets
        KF5::I18n
        ${LIBXML2_LIBRARIES}
        ${LIBXSLT_LIBRARIES}
        libumbrello
    )

ecm_add_test(
    testbasictypes.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testbasictypes
)

ecm_add_test(
    testumlobject.cpp
    testbase.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testumlobject
)

#ecm_add_test(
#    testclassifier.cpp
#    testbase.cpp
#    LINK_LIBRARIES ${LIBS}
#    TEST_NAME testclassifier
#)

ecm_add_test(
    testcppwriter.cpp
    testbase.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testcppwriter
)

ecm_add_test(
    testpythonwriter.cpp
    testbase.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testpythonwriter
)

ecm_add_test(
    testoptionstate.cpp
    testbase.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME testoptionstate
)

set(testumlroledialog_SRCS
    testumlroledialog.cpp
)

ecm_add_executable(testumlroledialog ${testumlroledialog_SRCS})
target_link_libraries(testumlroledialog ${LIBS})

find_package(LLVM CONFIG)
find_package(CLANG QUIET)
if(LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

    message(STATUS "Found CLANG ${CLANG_PACKAGE_VERSION}")

    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})

    if(CLANG_FOUND)
        include_directories(${CLANG_INCLUDE_DIRS})
        add_definitions(${CLANG_DEFINITIONS})
    endif()

    # Now build our tools
    add_executable(testllvm testllvm.cpp)

    # Find the libraries that correspond to the LLVM components
    # that we wish to use
    llvm_map_components_to_libnames(llvm_libs support core irreader analysis)

    # Link against LLVM libraries
    target_link_libraries(testllvm ${llvm_libs} clangFrontend clangTooling)

    add_executable(testllvmparser testllvmparser.cpp)
    llvm_map_components_to_libnames(llvm_libs support)
    target_link_libraries(testllvmparser ${llvm_libs} clangFrontend clangTooling clangAST clangBasic ${LIBS})
    ecm_mark_nongui_executable(testllvm testllvmparser)
endif()
