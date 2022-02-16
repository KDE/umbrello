#
# ecm compatible build system wrapper for KDE4
#

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

macro(add_executable)
    # avoid recursive loops
    if(NOT DEFINED in_add_executable)
        set(in_add_executable 0)
    endif()
    math(EXPR in_add_executable "${in_add_executable} + 1")
    if(${in_find_package} OR ${in_add_executable} GREATER 1)
        _add_executable(${ARGV})
    else()
        kde4_add_executable(${ARGV})
    endif()
    math(EXPR in_add_executable "${in_add_executable} - 1")
endmacro(add_executable)

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

macro(ecm_install_icons)
    kde4_install_icons(${ICON_INSTALL_DIR})
endmacro()


macro(find_package package)
    # avoid recursive loops
    if(NOT DEFINED in_find_package)
        set(in_find_package 0)
    endif()
    math(EXPR in_find_package "${in_find_package} + 1")
    #message("-- ${ARGV} --")
    if("${package}" MATCHES "^(Qt5)")
        #message("-- replaced by finding Qt4 --")
        _find_package(Qt4 4.4.3 REQUIRED QtCore QtGui QtXml QtTest)
        _find_package(Qt4QtWebKit 4.4.3)
        if(Qt4QtWebKit_FOUND)
            add_definitions(-DWEBKIT_WELCOMEPAGE)
            set(WEBKIT_WELCOMEPAGE 1)
        endif()
    elseif("${package}" MATCHES "^(KF5)$")
        #message("-- replaced by finding KDE4 --")
        _find_package(KDE4 REQUIRED)
        include(KDE4Defaults)
        include(MacroLibrary)

        # The FindKDE4.cmake module sets _KDE4_PLATFORM_DEFINITIONS with
        # definitions like _GNU_SOURCE that are needed on each platform.
        set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS} -DQT_STRICT_ITERATORS)
        add_definitions (${KDE4_DEFINITIONS})
        include_directories(${KDE4_INCLUDES})
        if(KDE4_BUILD_TESTS)
            enable_testing()
            set(BUILD_UNITTESTS 1)
        endif()
    elseif("${package}" MATCHES "^(KDevPlatform)$")
        _find_package(${ARGV})
        if ("${KDevPlatform_VERSION_MAJOR}" STREQUAL "5")
            message(FATAL_ERROR "Could not compile with KF5 based kdevplatform. Please uninstall or specify -DBUILD_PHP_IMPORT=0")
        endif()
    else()
        _find_package(${ARGV})
    endif()
    math(EXPR in_find_package "${in_find_package} - 1")
endmacro()
