project(umbrello)

set(VERSION_MAJOR "2")
set(VERSION_MINOR "14")
set(VERSION_PATCH "99")

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

# we need some parts of the ECM CMake helpers
find_package(ECM 1.1.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMInstallIcons)
include(ECMSetupVersion)

# search packages used by Qt
set(QT_MIN_VERSION "5.2.0")
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    DBus
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
    Auth
    Completion
    Config
    CoreAddons
    DocTools
    GuiAddons
    I18n
    IconThemes
    Init
    KDELibs4Support
    KIO
    Parts
    Service
    TextEditor
    WindowSystem
    WidgetsAddons
    XmlGui
)

# use some compile flags
add_definitions(
    -DQT_NO_SIGNAL_SLOTS_KEYWORDS
    -DQT_NO_URL_CAST_FROM_STRING
)

# set default umbrello version
# umbrello version could be overridden by cmake command line using -DUMBRELLO_VERSION_STRING=major.minor.patch
if(NOT UMBRELLO_VERSION_STRING)
    set(UMBRELLO_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
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
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    /opt/project-neon5/include/KF5/KConfigCore      #FIXME
    /opt/project-neon5/include/KF5/KConfigGui       #FIXME
    /opt/project-neon5/include/KF5/KConfigWidgets   #FIXME
    /opt/project-neon5/include/KF5/KDELibs4Support  #FIXME
    /opt/project-neon5/include/KF5/KIOCore          #FIXME
    /opt/project-neon5/include/KF5/KIOFileWidgets   #FIXME
    /opt/project-neon5/include/KF5/KIOWidgets       #FIXME
    /opt/project-neon5/include/KF5/KTextWidgets     #FIXME
    /opt/project-neon5/include/KF5/SonnetUi         #FIXME
)

if(LIBXSLT_FOUND AND LIBXML2_FOUND)
    add_subdirectory(umbrello)
    add_subdirectory(doc)
    # do we build the unit tests
    if(KDE4_BUILD_TESTS)
        ENABLE_TESTING()
        add_subdirectory(unittests)
    endif(KDE4_BUILD_TESTS)
endif(LIBXSLT_FOUND AND LIBXML2_FOUND)

set(KDE4_BUILD_TESTS ON)  # for unit tests

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
