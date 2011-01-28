IF( KDE4_BUILD_TESTS )

  set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

  include_directories(
      ${CMAKE_SOURCE_DIR}
      ../umbrello
      ${CMAKE_CURRENT_BINARY_DIR}
      ${KDE4_INCLUDES}
  )

  set( umbrello_SRCS
      ../umbrello/basictypes.cpp
  )

  MACRO(UNIT_TESTS)
      FOREACH(_className ${ARGN})
          MESSAGE( STATUS "unit test = ${_className}" )
          set(_testName TEST_${_className})
          kde4_add_unit_test(${_testName} TESTNAME umbrello-${_testName} NOGUI ${_testName}.cpp ${umbrello_SRCS})
          target_link_libraries(${_testName} ${QT_QTTEST_LIBRARY}
              ${KDE4_KFILE_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${LIBUUID} )
      ENDFOREACH(_className)
  ENDMACRO(UNIT_TESTS)

  unit_tests(
      basictypes
  )

ENDIF( KDE4_BUILD_TESTS )