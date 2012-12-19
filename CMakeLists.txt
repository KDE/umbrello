project(umbrello)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# search packages used by KDE
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckLibraryExists)

# The FindKDE4.cmake module sets _KDE4_PLATFORM_DEFINITIONS with
# definitions like _GNU_SOURCE that are needed on each platform.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS} -DQT_STRICT_ITERATORS)

find_package(Strigi REQUIRED)

macro_optional_find_package(LibXslt)
macro_log_feature(LIBXSLT_FOUND "LibXSLT" "A library to transform XMLfiles into other XML files" "http://xmlsoft.org/XSLT" FALSE "" "Required to build Umbrello.")
macro_optional_find_package(LibXml2)
macro_log_feature(LIBXML2_FOUND "LibXML2" "Libraries used to develop XML applications" "http://xmlsoft.org" FALSE "" "Required to build Umbrello.")

set(Boost_MINIMUM_VERSION 1.33.1)
macro_optional_find_package(Boost)
macro_log_feature(Boost_FOUND "boost" "Boost C++ Libraries" "http://www.boost.org" FALSE "1.33.1" "Required to build Umbrello.")

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
)

macro_display_feature_log()

KDE4_NO_ENABLE_FINAL(umbrello)

if(LIBXSLT_FOUND AND LIBXML2_FOUND AND Boost_FOUND)
    add_subdirectory( umbrello )
    add_subdirectory( doc )
endif(LIBXSLT_FOUND AND LIBXML2_FOUND AND Boost_FOUND)
