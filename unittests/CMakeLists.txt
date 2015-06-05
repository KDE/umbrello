
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
      ${SRC_PATH}/widgets
      ${CMAKE_CURRENT_BINARY_DIR}
)

if(NOT BUILD_KF5)
    set(LIBS
        ${QT_QTXML_LIBRARY}
        ${QT_QTTEST_LIBRARY}
        ${KDE4_KFILE_LIBS}
        ${LIBXML2_LIBRARIES}
        ${LIBXSLT_LIBRARIES}
        umbrello
    )
else()
    set(LIBS
        Qt5::Xml
        Qt5::Test
        Qt5::Widgets
        KF5::I18n
        ${LIBXML2_LIBRARIES}
        ${LIBXSLT_LIBRARIES}
        umbrello
    )
endif()

ecm_add_test(
    TEST_basictypes.cpp ${SRC_PATH}/basictypes.cpp
    LINK_LIBRARIES ${LIBS}
    TEST_NAME TEST_basictypes
)

# FIXME requires mostly of umbrello code -> add umbrello library target
#ecm_add_test(
#    TEST_classifier.cpp ${SRC_PATH}/classifier.cpp
#    LINK_LIBRARIES ${LIBS}
#    TEST_NAME TEST_classifier
#)

# FIXME requires mostly of umbrello code -> add umbrello library target
#ecm_add_test(
#    TEST_cppwriter.cpp ${SRC_PATH}/codegenerators/cpp/cppwriter.cpp
#    LINK_LIBRARIES ${LIBS}
#    TEST_NAME TEST_cppwriter
#)

# FIXME requires mostly of umbrello code -> add umbrello library target
#ecm_add_test(
#    TEST_pythonwriter.cpp ${SRC_PATH}/codegenerators/python/pythonwriter.cpp
#    LINK_LIBRARIES ${LIBS}
#    TEST_NAME TEST_pythonwriter
#)

set(TEST_umlroledialog_SRCS
    TEST_umlroledialog.cpp
)

ecm_add_executable(TEST_umlroledialog ${TEST_umlroledialog_SRCS})
target_link_libraries(TEST_umlroledialog ${LIBS})
