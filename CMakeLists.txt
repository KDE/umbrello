project(umbrello)

set(VERSION_MAJOR "2")
set(VERSION_MINOR "14")
set(VERSION_PATCH "99")

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

# we need some parts of the ECM CMake helpers
find_package(ECM 1.1.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
#set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(QT_MIN_VERSION "5.2.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Xml Test) 

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMInstallIcons)
include(ECMSetupVersion)

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

# search packages used by KDE
find_package(KF5 REQUIRED COMPONENTS
  Config
  DocTools
  GuiAddons
  I18n
  Init
  WindowSystem
  XmlGui
)

#include(KDE4Defaults)
#include(MacroLibrary)

message(STATUS "Using automoc4 version ${AUTOMOC4_VERSION}")
message(STATUS "Using cmake version ${CMAKE_VERSION}")

# The FindKDE4.cmake module sets _KDE4_PLATFORM_DEFINITIONS with
# definitions like _GNU_SOURCE that are needed on each platform.
#set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS} -DQT_STRICT_ITERATORS)

find_package(LibXslt)
#macro_log_feature(LIBXSLT_FOUND "LibXSLT" "A library to transform XMLfiles into other XML files" #"http://xmlsoft.org/XSLT" FALSE "" "Required to build Umbrello.")
find_package(LibXml2)
#macro_log_feature(LIBXML2_FOUND "LibXML2" "Libraries used to develop XML applications" "http://xmlsoft.org" FALSE "" #"Required to build Umbrello.")

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
    
)

#macro_display_feature_log()

#KDE4_NO_ENABLE_FINAL(umbrello)

if(LIBXSLT_FOUND AND LIBXML2_FOUND)
    add_subdirectory( umbrello )
    add_subdirectory( doc )
    # do we build the unit tests
    if(KDE4_BUILD_TESTS)
        ENABLE_TESTING()
        add_subdirectory(unittests)
    endif(KDE4_BUILD_TESTS)
endif(LIBXSLT_FOUND AND LIBXML2_FOUND)

set(KDE4_BUILD_TESTS ON)  # for unit tests
