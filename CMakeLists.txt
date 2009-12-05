project(umbrello)

# search packages used by KDE
set(QT_MIN_VERSION "4.6.0")
find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckLibraryExists)

# The FindKDE4.cmake module sets _KDE4_PLATFORM_DEFINITIONS with
# definitions like _GNU_SOURCE that are needed on each platform.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS} -DQT_STRICT_ITERATORS)

find_package(LibXslt REQUIRED)
find_package(LibXml2 REQUIRED)

set(Boost_MINIMUM_VERSION 1.33.1)
find_package(Boost REQUIRED)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DQT3_SUPPORT)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} )

add_subdirectory(umbrello)

