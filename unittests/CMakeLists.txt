include(ECMMarkAsTest)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(SRC_PATH ../umbrello)

include_directories(
      ${LIBXML2_INCLUDE_DIR}
      ${LIBXSLT_INCLUDE_DIR}
      ${CMAKE_SOURCE_DIR}
      ${SRC_PATH}
      ${SRC_PATH}/debug/
      ${SRC_PATH}/dialogs/
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

# unit test for basictypes.cpp
message(STATUS "Unit test: basictypes")
add_executable(TEST_basictypes  ${SRC_PATH}/basictypes.cpp TEST_basictypes.cpp)    #FIXME KF5
target_link_libraries(TEST_basictypes Qt5::Test Qt5::Widgets KF5::I18n)
add_test(TEST_basictypes TEST_basictypes)

# unit test for classifier.cpp
#message(STATUS "Unit test: classifier")
#add_executable(TEST_classifier  ${SRC_PATH}/classifier.cpp TEST_classifier.cpp)    #FIXME KF5
#target_link_libraries(TEST_classifier Qt5::Test Qt5::Widgets Qt5::Xml KF5::I18n KF5::XmlGui)
#add_test(TEST_classifier TEST_classifier)

# unit test for cppwriter.cpp
#message(STATUS "Unit test: cppwriter")
#add_executable(TEST_cppwriter  ${SRC_PATH}/codegenerators/cpp/cppwriter.cpp TEST_cppwriter.cpp)    #FIXME KF5
#target_link_libraries(TEST_cppwriter Qt5::Test Qt5::Widgets Qt5::Xml KF5::I18n KF5::XmlGui)
#add_test(TEST_cppwriter TEST_cppwriter)

# unit test for pythonwriter.cpp
#message(STATUS "Unit test: pythonwriter")
#add_executable(TEST_pythonwriter  ${SRC_PATH}/codegenerators/python/pythonwriter.cpp TEST_pythonwriter.cpp)    #FIXME KF5
#target_link_libraries(TEST_pythonwriter Qt5::Test Qt5::Widgets Qt5::Xml KF5::I18n KF5::XmlGui)
#add_test(TEST_pythonwriter TEST_pythonwriter)

# run ctest after building all unit tests
macro(add_unit_test target unittests)
    set(UNIT_TEST_TARGETS ${UNIT_TEST_TARGETS} ${unittests} PARENT_SCOPE)
    add_test(target ${CMAKE_CURRENT_BINARY_DIR}/unittests)
endmacro(add_unit_test)

add_custom_target(all_tests ALL
                  DEPENDS ${UNIT_TEST_TARGETS})
add_custom_command(TARGET all_tests
    COMMENT "Run unit tests -----------------------------"
    POST_BUILD
    COMMAND ctest ARGS
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
