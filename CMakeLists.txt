project(umbrello)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

set(KDE4_BUILD_TESTS ON)  # for unit tests

set(VERSION_MAJOR "2")
set(VERSION_MINOR "12")
set(VERSION_PATCH "99")

# set default umbrello version
# umbrello version could be overriden by cmake command line using -DUMBRELLO_VERSION_STRING=major.minor.patch
if(NOT UMBRELLO_VERSION_STRING)
    set(UMBRELLO_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    if(EXISTS ${CMAKE_SOURCE_DIR}/.git)
        execute_process(
            COMMAND git rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE _hash
        )
        if(hash)
           string(REPLACE "\n" "" hash ${_hash})
           set(UMBRELLO_VERSION_STRING "${UMBRELLO_VERSION_STRING}-${hash}")
        endif()
    endif()
endif()

message(STATUS "Using umbrello version ${UMBRELLO_VERSION_STRING}")
add_definitions(-DUMBRELLO_VERSION_STRING="${UMBRELLO_VERSION_STRING}")

# search packages used by KDE
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

# The FindKDE4.cmake module sets _KDE4_PLATFORM_DEFINITIONS with
# definitions like _GNU_SOURCE that are needed on each platform.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS} -DQT_STRICT_ITERATORS)

macro_optional_find_package(LibXslt)
macro_log_feature(LIBXSLT_FOUND "LibXSLT" "A library to transform XMLfiles into other XML files" "http://xmlsoft.org/XSLT" FALSE "" "Required to build Umbrello.")
macro_optional_find_package(LibXml2)
macro_log_feature(LIBXML2_FOUND "LibXML2" "Libraries used to develop XML applications" "http://xmlsoft.org" FALSE "" "Required to build Umbrello.")

set(Boost_MINIMUM_VERSION 1.33.1)
macro_optional_find_package(Boost)
macro_log_feature(Boost_FOUND "boost" "Boost C++ Libraries" "http://www.boost.org" FALSE "1.33.1" "Required to build Umbrello.")

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
    ${KDE4_INCLUDES}
)

macro_display_feature_log()

KDE4_NO_ENABLE_FINAL(umbrello)

if(LIBXSLT_FOUND AND LIBXML2_FOUND AND Boost_FOUND)
    add_subdirectory( umbrello )
    add_subdirectory( doc )
    # do we build the unit tests
    if(KDE4_BUILD_TESTS)
        ENABLE_TESTING()
        add_subdirectory(unittests)
    endif(KDE4_BUILD_TESTS)
endif(LIBXSLT_FOUND AND LIBXML2_FOUND AND Boost_FOUND)
