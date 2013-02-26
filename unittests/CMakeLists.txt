IF( KDE4_BUILD_TESTS )

  set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
  set( SRC_PATH ../umbrello )

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
      ${KDE4_INCLUDES}
  )

  link_directories(
      ${CMAKE_CURRENT_BINARY_DIR}/../umbrello/umbrello
      ${CMAKE_CURRENT_BINARY_DIR}/../umbrello/umbrello/CMakeFiles/umbrello-soc.dir
      ${CMAKE_CURRENT_BINARY_DIR}/../umbrello/umbrello/CMakeFiles/umbrello-soc.dir/dialogs

  )

  MACRO(UNIT_TESTS)
      FOREACH(_className ${ARGN})
          MESSAGE(STATUS "Unit test: ${_className}")
          set(_testName TEST_${_className})
          kde4_add_unit_test(${_testName} TESTNAME umbrello-${_testName} NOGUI ${_testName}.cpp
              ${SRC_PATH}/${_className}.cpp)
          target_link_libraries(${_testName} ${QT_QTTEST_LIBRARY}
              ${KDE4_KFILE_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${LIBUUID})
      ENDFOREACH(_className)
  ENDMACRO(UNIT_TESTS)

  unit_tests(
      basictypes
#      classifier
#      cppwriter
#      pythonwriter
  )

ENDIF( KDE4_BUILD_TESTS )
