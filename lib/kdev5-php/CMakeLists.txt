cmake_minimum_required(VERSION 3.16.0)

project(php)

if(POLICY CMP0028)
    cmake_policy(SET CMP0028 OLD)
endif()

if(POLICY CMP0071)
    cmake_policy(SET CMP0071 OLD)
endif()

set(KDEVPHP_VERSION_MAJOR 5)
set(KDEVPHP_VERSION_MINOR 1)
set(KDEVPHP_VERSION_PATCH 2)
# KDevplatform dependency version
set( KDEVPLATFORM_VERSION "${KDEVPHP_VERSION_MAJOR}.${KDEVPHP_VERSION_MINOR}.${KDEVPHP_VERSION_PATCH}" )

configure_file( "${php_SOURCE_DIR}/kdevphpversion.h.cmake" "${php_BINARY_DIR}/kdevphpversion.h" @ONLY )

find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(GenerateExportHeader)
include(FeatureSummary)

find_package(Qt6 REQUIRED Core Widgets Test)
find_package(KF6 REQUIRED COMPONENTS Archive ThreadWeaver TextEditor I18n ItemModels KCMUtils)
find_package(KDevPlatform ${KDEVPLATFORM_VERSION} REQUIRED)
find_package(KDevelop-PG-Qt REQUIRED)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdocumentation")
endif()

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9043 )
add_definitions( -DTRANSLATION_DOMAIN=\"umbrello_kdevphp5\" )

include_directories(
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
add_subdirectory(docs)

set(kdevphplanguagesupport_PART_SRCS
    phplanguagesupport.cpp
    phpparsejob.cpp
    phphighlighting.cpp
    phpdebug.cpp

    codegen/refactoring.cpp
)

kdevplatform_add_plugin(kdevphplanguagesupport JSON kdevphpsupport.json
    SOURCES ${kdevphplanguagesupport_PART_SRCS})
target_link_libraries(kdevphplanguagesupport
    KDev::Interfaces
    KDev::Language
    KDev::Tests
    kdevphpduchain
    kdevphpparser
    kdevphpcompletion

    KF6::I18n
    KF6::TextEditor
)
endif()

# not writeable so that the refactoring actions get hidden
if(NOT NO_INSTALL)
    install(FILES phpfunctions.php DESTINATION ${DATA_INSTALL_DIR}/kdevphpsupport PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
