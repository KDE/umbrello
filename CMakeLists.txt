project(umbrello)

# KDE Application Version, managed by release script
set (KDE_APPLICATIONS_VERSION_MAJOR "18")
set (KDE_APPLICATIONS_VERSION_MINOR "12")
set (KDE_APPLICATIONS_VERSION_MICRO "2")

set(UMBRELLO_VERSION_MAJOR "2")
MATH(EXPR UMBRELLO_VERSION_MINOR "15+${KDE_APPLICATIONS_VERSION_MINOR}")
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
endif()

find_package(KDevelop-PG-Qt)

if(NOT BUILD_KF5)
    cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    find_package(Qt4 4.4.3 REQUIRED QtCore QtGui QtXml QtTest QtWebKit)

    # search packages used by KDE
    find_package(KDE4 REQUIRED)
    include(KDE4Defaults)
    include(MacroLibrary)

    # The FindKDE4.cmake module sets _KDE4_PLATFORM_DEFINITIONS with
    # definitions like _GNU_SOURCE that are needed on each platform.
    set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS} -DQT_STRICT_ITERATORS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

    add_definitions (
        ${KDE4_DEFINITIONS}
    )

    include_directories(
        ${KDE4_INCLUDES}
    )

    if(KDE4_BUILD_TESTS)
        enable_testing()
        set(BUILD_UNITTESTS 1)
    endif()

    macro(ki18n_wrap_ui)
        kde4_add_ui_files(${ARGN})
    endmacro(ki18n_wrap_ui)

    macro(kconfig_add_kcfg_files)
        kde4_add_kcfg_files(${ARGN})
    endmacro(kconfig_add_kcfg_files)

    macro(ecm_optional_add_subdirectory)
        add_subdirectory(${ARGN})
    endmacro(ecm_optional_add_subdirectory)

    macro(ecm_add_test)
        set(sources)
        set(libs)
        set(name_prefix)
        set(test_name)
        set(mode 0)
        set(guimode "NOGUI")
        foreach(a ${ARGN})
            if(a STREQUAL "LINK_LIBRARIES")
                set(mode 1)
            elseif(a STREQUAL "NAME_PREFIX")
                set(mode 2)
            elseif(a STREQUAL "GUI")
                set(guimode "GUI")
                set(mode 3)
            elseif(a STREQUAL "TEST_NAME")
                set(mode 4)
            elseif (mode EQUAL 0)
                list(APPEND sources ${a})
            elseif(mode EQUAL 1)
                list(APPEND libs ${a})
            elseif(mode EQUAL 2)
                set(name_prefix ${a})
            elseif(mode EQUAL 4)
                set(test_name ${a})
            endif()
        endforeach(a)

        set(targetname ${test_name})
        if(name_prefix)
            set(testname ${name_prefix}-${targetname})
        else()
            set(testname ${targetname})
        endif()
        kde4_add_unit_test(${targetname} TESTNAME ${testname} ${guimode} ${sources})
        target_link_libraries(${targetname} ${libs})
    endmacro(ecm_add_test)

    macro(ecm_add_executable)
        kde4_add_executable(${ARGN})
    endmacro(ecm_add_executable)

    macro(ecm_mark_nongui_executable)
        foreach(_target ${ARGN})
            set_target_properties(${_target}
                                  PROPERTIES
                                  WIN32_EXECUTABLE FALSE
                                  MACOSX_BUNDLE FALSE
                                 )
        endforeach()
    endmacro(ecm_mark_nongui_executable)

    macro(kdoctools_create_handbook)
        set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDE4_DATA_INSTALL_DIR}/ksgmltools2/customization")
        set(KDEX_DTD ${KDE4_DATA_DIR}/ksgmltools2/customization/dtd/kdex.dtd)
        set(kdedbx45_dtd "
    <!ENTITY % kdexDTD       PUBLIC
      \"-//OASIS//DTD DocBook XML V4.5//EN\"
      \"${KDEX_DTD}\">
    %kdexDTD;
    ")
        set(CUSTOM_DTD ${CMAKE_CURRENT_SOURCE_DIR}/dtd/kdedbx45.dtd)
        if (NOT EXISTS ${CUSTOM_DTD})
            file(WRITE ${CUSTOM_DTD} "${kdedbx45_dtd}")
        endif()
        kde4_create_handbook(${ARGN})
    endmacro()
    if(NOT DISABLE_PHP_IMPORT)
        find_package(KDevelop-PG-Qt)
        find_package(KDevPlatform)
        if ("${KDevPlatform_VERSION_MAJOR}" STREQUAL "5")
            message(FATAL_ERROR "Could not compile with KF5 based kdevplatform. Please uninstall or specify -DDISABLE_PHP_IMPORT=1")
        endif()
    endif()
    if(KDEVPGQT_FOUND AND KDEVPLATFORM_FOUND)
        include_directories(
            ${KDEVPGQT_INCLUDE_DIR}
            ${KDEVPLATFORM_INCLUDE_DIR}
            ${CMAKE_SOURCE_DIR}/lib/kdev4-php/parser
            ${CMAKE_BINARY_DIR}/lib/kdev4-php/parser
        )
        set(NO_INSTALL 1)
        set(BUILD_PARSER_ONLY 1)
        set(BUILD_PHP_IMPORT 1)
        add_definitions(
            -DKDEVPHPPARSER_EXPORT=
            -DENABLE_PHP_IMPORT
        )
        add_subdirectory(lib/kdev4-php)
    endif()

    macro_display_feature_log()
    KDE4_NO_ENABLE_FINAL(umbrello)
    set(UMBRELLO_DATA_INSTALL_DIR ${DATA_INSTALL_DIR}/umbrello)
else()
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

    macro(ecm_add_executable)
        add_executable(${ARGN})
    endmacro(ecm_add_executable)

    set(BUILD_UNITTESTS 1)
    set(UMBRELLO_DATA_INSTALL_DIR ${DATA_INSTALL_DIR}/umbrello5)

    ki18n_install(po)
    kdoctools_install(po)

    find_package(KDevPlatform "5.1.2" COMPONENTS
        KDev::Interfaces
        KDev::Language
        KDev::Tests
    )
    if(KDEVPGQT_FOUND AND KDEVPLATFORM_FOUND AND TARGET KDev::Tests)
        include_directories(
            ${KDEVPGQT_INCLUDE_DIR}
            ${KDEVPLATFORM_INCLUDE_DIR}
            ${CMAKE_SOURCE_DIR}/lib/kdev5-php/parser
            ${CMAKE_BINARY_DIR}/lib/kdev5-php/parser
        )
        set(NO_INSTALL 1)
        set(BUILD_PARSER_ONLY 1)
        set(BUILD_PHP_IMPORT 1)
        add_definitions(
            -DKDEVPHPPARSER_EXPORT=
            -DENABLE_PHP_IMPORT
        )
        add_subdirectory(lib/kdev5-php)
    elseif(NOT TARGET KDev::Tests)
        message(STATUS "Could not find required KDevPlatform component KDev::Tests - disabling php import support")
    endif()
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
if (BUILD_KF5)
    set(QTVERSION ${Qt5Widgets_VERSION})
endif()
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
