project(umbrello)

# KDE Application Version, managed by release script
set (KDE_APPLICATIONS_VERSION_MAJOR "17")
set (KDE_APPLICATIONS_VERSION_MINOR "07")
set (KDE_APPLICATIONS_VERSION_MICRO "70")

set(UMBRELLO_VERSION_MAJOR "2")
MATH(EXPR UMBRELLO_VERSION_MINOR "15+${KDE_APPLICATIONS_VERSION_MINOR}")
set(UMBRELLO_VERSION_PATCH ${KDE_APPLICATIONS_VERSION_MICRO})

option(BUILD_ICONS "Create icons from svg source" OFF)

option(BUILD_CURSOR_ICONS "Create cursor icons from svg source" OFF)

option(BUILD_ICONS_INTO_SIZE_RELATED_DIRS "place icons into separate directories (for debugging purpose)" OFF)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
include(Macros)

set(CMAKE_AUTORCC ON)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fno-delete-null-pointer-checks")
endif()

    cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

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

    # search packages used by Qt
    set(QT_MIN_VERSION "5.2.0")
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
        DocTools
        I18n
        IconThemes
        KIO
        TextEditor
        WidgetsAddons
        WindowSystem
        XmlGui
    )

    macro(ecm_add_executable)
        add_executable(${ARGN})
    endmacro(ecm_add_executable)

    set(BUILD_UNITTESTS 1)
    set(UMBRELLO_DATA_INSTALL_DIR ${DATA_INSTALL_DIR}/umbrello5)

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
message(STATUS "Using Qt version ${QT_MIN_VERSION}")

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
