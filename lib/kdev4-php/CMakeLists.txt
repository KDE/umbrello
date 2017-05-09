PROJECT(php)

cmake_minimum_required(VERSION 2.8.9)

set(KDEVPHP_VERSION_MAJOR 1)
set(KDEVPHP_VERSION_MINOR 7)
set(KDEVPHP_VERSION_PATCH 3)
# KDevplatform dependency version
set( KDEVPLATFORM_VERSION "1.${KDEVPHP_VERSION_MINOR}.${KDEVPHP_VERSION_PATCH}" )

configure_file( "${php_SOURCE_DIR}/kdevphpversion.h.cmake" "${php_BINARY_DIR}/kdevphpversion.h" @ONLY )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${php_SOURCE_DIR}/cmake/)
enable_testing()

#To properly find kdevelop-pg add -DKDEVPG_DATA_DIR=<kdevpg-install-path>/share to the
#cmake run if you installed it in a different directory than this plugin
#TODO: a cmake run is needed after changing one of the CMakeLists.txt, make will
#complain about the FindKDevelop-PG.cmake file not findable.
find_package(KDE4 REQUIRED)

find_package(KDevPlatform ${KDEVPLATFORM_VERSION} REQUIRED)
include_directories(${KDEVPLATFORM_INCLUDE_DIR})

include_directories(
    ${KDE4_INCLUDES}
    ${KDE4_INCLUDE_DIR}/threadweaver
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/duchain
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
    ${CMAKE_CURRENT_BINARY_DIR}/parser
)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9043 )

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

find_package(KDevelop-PG-Qt REQUIRED)

include_directories(
    ${CMAKE_BINARY_DIR}/parser
    ${KDEVPGQT_INCLUDE_DIR}
)

if(NOT BUILD_PARSER_ONLY)
add_subdirectory(app_templates)
endif()
add_subdirectory(parser)
if(NOT BUILD_PARSER_ONLY)
add_subdirectory(duchain)
add_subdirectory(completion)
add_subdirectory(testprovider)

set(kdevphplanguagesupport_PART_SRCS
    phplanguagesupport.cpp
    phpparsejob.cpp
    phphighlighting.cpp

    codegen/refactoring.cpp
)

kde4_add_plugin(kdevphplanguagesupport ${kdevphplanguagesupport_PART_SRCS})

target_link_libraries(kdevphplanguagesupport
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDE4_THREADWEAVER_LIBRARIES}
    ${KDE4_KTEXTEDITOR_LIBS}
    kdev4phpduchain
    kdev4phpparser
    kdev4phpcompletion
)

install(TARGETS kdevphplanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR})
endif()


configure_file(kdevphpsupport.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevphpsupport.desktop)
if(NOT NO_INSTALL)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevphpsupport.desktop DESTINATION ${SERVICES_INSTALL_DIR})
    # not writeable so that the refactoring actions get hidden
    install(FILES phpfunctions.php DESTINATION ${DATA_INSTALL_DIR}/kdevphpsupport PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
endif()

include(CTest)

# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake)
