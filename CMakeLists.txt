cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project(umbrello)

# KDE Application Version, managed by release script
set (KDE_APPLICATIONS_VERSION_MAJOR "19")
set (KDE_APPLICATIONS_VERSION_MINOR "04")
set (KDE_APPLICATIONS_VERSION_MICRO "1")

set(UMBRELLO_VERSION_MAJOR "2")
MATH(EXPR UMBRELLO_VERSION_MINOR "30-${KDE_APPLICATIONS_VERSION_MINOR}")
set(UMBRELLO_VERSION_PATCH ${KDE_APPLICATIONS_VERSION_MICRO})

option(BUILD_ICONS "Create icons from svg source" OFF)

option(BUILD_CURSOR_ICONS "Create cursor icons from svg source" OFF)

option(BUILD_ICONS_INTO_SIZE_RELATED_DIRS "place icons into separate directories (for debugging purpose)" OFF)

option(BUILD_KF5 "Build for KDE Frameworks 5" ON)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
include(Macros)

set(CMAKE_AUTORCC ON)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fno-delete-null-pointer-checks")
    # TODO: after removing kdelibs4support this could also be removed
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if(NOT BUILD_KF5)
    set(APP_SUFFIX)
    set(LIB_PREFIX Qt4::Qt)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    include(ECMKDE4Macros)
    set(QT_MIN_VERSION "4.8.0")
    set(KDEV_MIN_VERSION 1.5.2)
    set(KDEV_SOURCE_DIR lib/kdev4-php)
    set(KDEV_COMPONENTS)
else()
    set(APP_SUFFIX 5)
    set(LIB_PREFIX Qt5::)
    # we need some parts of the ECM CMake helpers
    find_package(ECM 1.1.0 REQUIRED NO_MODULE)
    list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

    include(KDEInstallDirs)
    include(KDECompilerSettings NO_POLICY_SCOPE)
    include(KDECMakeSettings)
    include(FeatureSummary)
    include(ECMInstallIcons)
    include(ECMSetupVersion)
    include(ECMOptionalAddSubdirectory)
    include(ECMAddAppIcon)
    include(ECMAddTests)
    set(QT_MIN_VERSION "5.2.0")
    set(KDEV_MIN_VERSION 5.1.2)
    set(KDEV_SOURCE_DIR lib/kdev5-php)
    set(KDEV_COMPONENTS KDev::Interfaces KDev::Language KDev::Tests)
endif()

# search packages used by Qt
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    Gui
    PrintSupport
    Svg
    Test
    Widgets
    Xml
    WebKitWidgets
)

# search packages used by KDE
find_package(KF5 REQUIRED COMPONENTS
    Archive
    Completion
    Config
    CoreAddons
    Crash
    DocTools
    I18n
    IconThemes
    KDELibs4Support
    KIO
    TextEditor
    WidgetsAddons
    WindowSystem
    XmlGui
)

if(NOT DISABLE_PHP_IMPORT)
    find_package(KDevelop-PG-Qt)
    find_package(KDevPlatform ${KDEV_MIN_VERSION} COMPONENTS ${KDEV_COMPONENTS})
endif()
if(BUILD_KF5 AND NOT TARGET KDev::Tests)
    message(STATUS "Could not find required KDevPlatform component KDev::Tests - disabling php import support")
    set(KDEVPLATFORM_FOUND 0)
endif()
if(KDEVPGQT_FOUND AND KDEVPLATFORM_FOUND)
    include_directories(
        ${KDEVPGQT_INCLUDE_DIR}
        ${KDEVPLATFORM_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/${KDEV_SOURCE_DIR}/parser
        ${CMAKE_BINARY_DIR}/${KDEV_SOURCE_DIR}/parser
    )
    set(NO_INSTALL 1)
    set(BUILD_PARSER_ONLY 1)
    set(BUILD_PHP_IMPORT 1)
    add_definitions(
        -DKDEVPHPPARSER_EXPORT=
        -DENABLE_PHP_IMPORT
    )
    add_subdirectory(${KDEV_SOURCE_DIR})
endif()

set(UMBRELLO_DATA_INSTALL_DIR ${DATA_INSTALL_DIR}/umbrello${APP_SUFFIX})

if(NOT BUILD_KF5)
    macro_display_feature_log()
    KDE4_NO_ENABLE_FINAL(umbrello)
else()
    set(BUILD_UNITTESTS 1)

    ki18n_install(po)
    kdoctools_install(po)
endif()

if(POLICY CMP0071)
  cmake_policy(SET CMP0071 OLD)
endif()

# use some compile flags
add_definitions(
    -DQT_NO_SIGNAL_SLOTS_KEYWORDS
    -DQT_NO_URL_CAST_FROM_STRING
)

# set default umbrello version
# umbrello version could be overridden by cmake command line using -DUMBRELLO_VERSION_STRING=major.minor.patch
if(NOT UMBRELLO_VERSION_STRING)
    set(UMBRELLO_VERSION_STRING "${UMBRELLO_VERSION_MAJOR}.${UMBRELLO_VERSION_MINOR}.${UMBRELLO_VERSION_PATCH}")
    if(EXISTS ${CMAKE_SOURCE_DIR}/.git)
        execute_process(
            COMMAND git rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE _hash
        )
        if(_hash)
           string(REPLACE "\n" "" hash ${_hash})
           set(UMBRELLO_VERSION_STRING "${UMBRELLO_VERSION_STRING}-${hash}")
        endif()
    endif()
endif()

message(STATUS "Using umbrello version ${UMBRELLO_VERSION_STRING}")
add_definitions(-DUMBRELLO_VERSION_STRING="${UMBRELLO_VERSION_STRING}")

message(STATUS "Using automoc4 version ${AUTOMOC4_VERSION}")
message(STATUS "Using cmake version ${CMAKE_VERSION}")
message(STATUS "Using Qt version ${QTVERSION}")

find_package(LibXslt)
set_package_properties(LibXslt PROPERTIES
    DESCRIPTION "A library to transform XMLfiles into other XML files."
    URL "http://xmlsoft.org/XSLT"
    TYPE REQUIRED)

find_package(LibXml2)
set_package_properties(LibXml2 PROPERTIES
    DESCRIPTION "Libraries used to develop XML applications."
    URL "http://xmlsoft.org"
    TYPE REQUIRED)

include_directories(
    ${CMAKE_BINARY_DIR}
)

#
# Unstable features should not be enabled in stable branches
# This macro limits unstable feature to development branches
# by only adding -DENABLE_<name> to the compile command when
# the patch level is >= 70 which includes releases from master
# beta and rc releases. On cmake configure time a related status
# note is displayed.
#
# syntax:  add_unstable_feature(<name>)
#
# In code wrap feature related code with
#
#  #ifdef ENABLE_<name>
#    <feature code>
#  #endif
#
macro(add_unstable_feature name)
    if(UMBRELLO_VERSION_PATCH GREATER 69)
        add_definitions(-DENABLE_${name})
        set(ENABLE_${name} 1)
        message(STATUS "Enable unstable feature ${name} - use '#ifdef ENABLE_${name}' in related code")
    else()
        set(ENABLE_${name} 0)
        message(STATUS "Disable unstable feature ${name}")
    endif()
endmacro(add_unstable_feature)

add_unstable_feature(WIDGET_SHOW_DOC) # show documentation in classes ticket xxx
add_unstable_feature(NEW_CODE_GENERATORS) # new c++ code generator
add_unstable_feature(UML_OBJECTS_WINDOW) # objects dock window
add_unstable_feature(XMIRESOLUTION) # see https://bugs.kde.org/show_bug.cgi?id=90103

# Current object diagram implementation does not follow UML 1.4 standard
# because of using umbrello specific XMI tag (UML:InstanceAttribute).
add_unstable_feature(OBJECT_DIAGRAM) # see https://bugs.kde.org/show_bug.cgi?id=123633

configure_file(Doxyfile ${CMAKE_BINARY_DIR}/Doxyfile)
find_program(DOXYGEN_EXECUTABLE doxygen)
if(DOXYGEN_EXECUTABLE)
    add_custom_target(apidoc
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
    )
endif()

if(LIBXSLT_FOUND AND LIBXML2_FOUND)
    add_subdirectory(umbrello)
    add_subdirectory(doc)
    add_subdirectory(tools)
endif(LIBXSLT_FOUND AND LIBXML2_FOUND)

add_subdirectory(maintainer)

# for automatic unit tests
if(BUILD_UNITTESTS)
    ecm_optional_add_subdirectory(unittests)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
