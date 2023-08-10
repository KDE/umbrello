cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(umbrello)

if(POLICY CMP0028)
    cmake_policy(SET CMP0028 OLD)
endif()

if(POLICY CMP0057)
    cmake_policy(SET CMP0057 NEW)
endif()

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "23")
set (RELEASE_SERVICE_VERSION_MINOR "07")
set (RELEASE_SERVICE_VERSION_MICRO "90")

set(UMBRELLO_VERSION_MAJOR "2")
set(UMBRELLO_VERSION_MINOR "38")
if (RELEASE_SERVICE_VERSION_MICRO GREATER_EQUAL 70)
    math(EXPR UMBRELLO_VERSION_MINOR "${UMBRELLO_VERSION_MINOR}-1")
endif()

set(UMBRELLO_VERSION_PATCH ${RELEASE_SERVICE_VERSION_MICRO})

option(BUILD_APIDOC "Create api documentation" ON)

option(BUILD_ICONS "Create icons from svg source" OFF)

option(BUILD_CURSOR_ICONS "Create cursor icons from svg source" OFF)

option(BUILD_ICONS_INTO_SIZE_RELATED_DIRS "place icons into separate directories (for debugging purpose)" OFF)

option(BUILD_PHP_IMPORT "Build with php import support" ON)

option(BUILD_QCH "Build with QCH support" ON)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
include(Macros)

set(CMAKE_AUTORCC ON)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fno-delete-null-pointer-checks")
    # TODO: after removing kdelibs4support this could also be removed
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(APP_SUFFIX 5)
set(LIB_PREFIX Qt5::)
set(KF5_MIN_VERSION "5.2.0")
# we need some parts of the ECM CMake helpers
find_package(ECM ${KF5_MIN_VERSION} NO_MODULE REQUIRED)

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
set(KDEV_COMPONENTS KDev::Interfaces KDev::Language KDev::Shell)
if(NOT QTQCHDIR)
    set(QTQCHDIR ${KDE_INSTALL_QTQCHDIR})
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
    KIO
    TextEditor
    WidgetsAddons
    WindowSystem
    XmlGui
)

if(CMAKE_COMPILER_IS_GNUCXX)
    string(REPLACE "-Wsuggest-override" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()

if(BUILD_PHP_IMPORT)
    find_package(KDevelop-PG-Qt)
    find_package(KDevPlatform ${KDEV_MIN_VERSION} COMPONENTS ${KDEV_COMPONENTS})
endif()
if(KDEVPGQT_FOUND AND KDEVPLATFORM_FOUND)
    message(STATUS "requirements available - enable building php code import")
    include_directories(
        ${KDEVPGQT_INCLUDE_DIR}
        ${KDEVPLATFORM_INCLUDE_DIR}
    )
    set(BUILD_PHP_IMPORT 1)
    add_definitions(
        -DENABLE_PHP_IMPORT
    )
    add_subdirectory(lib/kdevplatform)
    find_package(KDevPHP)
    if(NOT KDevPHP_FOUND)
        set(NO_INSTALL 1)
        set(BUILD_PARSER_ONLY 1)
        set(BUILD_PHP_IMPORT 1)
        add_definitions(
            -DKDEVPHPPARSER_EXPORT=
        )
        include_directories(
            ${CMAKE_SOURCE_DIR}/${KDEV_SOURCE_DIR}
            ${CMAKE_BINARY_DIR}/${KDEV_SOURCE_DIR}
        )
        add_subdirectory(${KDEV_SOURCE_DIR})
    endif()
else()
    set(BUILD_PHP_IMPORT 0)
endif()

set(UMBRELLO_DATA_INSTALL_DIR ${DATA_INSTALL_DIR}/umbrello${APP_SUFFIX})

set(BUILD_UNITTESTS 1)

ki18n_install(po)
kdoctools_install(po)

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
set(APPLICATIONS_VERSION_STRING "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

message(STATUS "Using umbrello version ${UMBRELLO_VERSION_STRING}")
add_definitions(-DUMBRELLO_VERSION_STRING="${UMBRELLO_VERSION_STRING}")
add_definitions(-DAPPLICATIONS_VERSION_STRING="${APPLICATIONS_VERSION_STRING}")

message(STATUS "Using automoc4 version ${AUTOMOC4_VERSION}")
message(STATUS "Using cmake version ${CMAKE_VERSION}")

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
add_unstable_feature(COMBINED_STATE_DIRECT_EDIT) # experimental feature

# Current object diagram implementation does not follow UML 1.4 standard
# because of using umbrello specific XMI tag (UML:InstanceAttribute).
add_unstable_feature(OBJECT_DIAGRAM) # see https://bugs.kde.org/show_bug.cgi?id=123633

# requires cmake >= 3.9.6
find_package(Doxygen OPTIONAL_COMPONENTS dot)
if(DOXYGEN_FOUND AND BUILD_APIDOC)
    message(STATUS "found doxygen package")
    find_program(QHELPGENERATOR_EXECUTABLE NAMES qhelpgenerator qhelpgenerator-qt5)
    message(STATUS "found qhelpgenerator ${QHELPGENERATOR_EXECUTABLE}")
    set(DOXYGEN_ABBREVIATE_BRIEF "The $name class" "The $name widget")
    set(DOXYGEN_CALL_GRAPH YES)
    set(DOXYGEN_EXCLUDE_PATTERNS */lib/*)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_PACKAGE YES)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    set(DOXYGEN_EXTRACT_STATIC YES)
    set(DOXYGEN_GENERATE_HTMLHELP YES)
    set(DOXYGEN_HAVE_DOT ${DOXYGEN_HAVE_DOT})
    set(DOXYGEN_DOT_IMAGE_FORMAT svg)
    set(DOXYGEN_MARKDOWN_SUPPORT NO)
    set(DOT_GRAPH_MAX_NODES 0)
    set(DOXYGEN_MAX_DOT_GRAPH_DEPTH 0)
    set(DOXYGEN_PROJECT_BRIEF "Umbrello UML Modeller is a Unified Modelling Language (UML) diagram program based on KDE Technology")
    set(DOXYGEN_PROJECT_NUMBER ${UMBRELLO_VERSION_STRING})
    #set(DOXYGEN_PROJECT_LOGO ${CMAKE_SOURCE_DIR}/umbrello/pics/global/hi32-apps-umbrello.png)
    set(DOXYGEN_SOURCE_BROWSER NO)
    set(DOXYGEN_UML_LOOK YES)
    set(DOXYGEN_USE_PDFLATEX NO)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/apidoc)

    if(BUILD_QCH AND QHELPGENERATOR_EXECUTABLE)
        set(DOXYGEN_GENERATE_QHP YES)
        set(DOXYGEN_QCH_FILE ${CMAKE_BINARY_DIR}/umbrello.qch)
        set(DOXYGEN_QHP_NAMESPACE org.kde.umbrello)
        set(DOXYGEN_QHG_LOCATION ${QHELPGENERATOR_EXECUTABLE})
        set(DOXYGEN_QHP_VIRTUAL_FOLDER doc)
        install(FILES ${DOXYGEN_QCH_FILE} DESTINATION ${QTQCHDIR} COMPONENT Devel)
    endif()

    doxygen_add_docs(apidoc
        ALL
        ${CMAKE_SOURCE_DIR}
        COMMENT "generating api doc"
    )

    install(DIRECTORY ${CMAKE_BINARY_DIR}/apidoc DESTINATION ${UMBRELLO_DATA_INSTALL_DIR})
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
