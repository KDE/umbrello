if (NOT BUILD_KF5)
    set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDE4_DATA_INSTALL_DIR}/ksgmltools2/customization")
    set(KDOCTOOLS_MEINPROC_EXECUTABLE meinproc4)
else()
    set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDOCTOOLS_DATA_INSTALL_DIR}/kdoctools/customization")
endif()

function(_kdoctools_create_target_name out in)
    string(REGEX REPLACE "^${CMAKE_BINARY_DIR}/?" "" in "${in}")
    string(REGEX REPLACE "[^0-9a-zA-Z]+" "-" tmp "${in}")
    set(${out} ${tmp} PARENT_SCOPE)
endfunction()

function (kdoctools_create_article docbook)
    # Parse arguments
    set(options)
    set(oneValueArgs INSTALL_DESTINATION SUBDIR)
    set(multiValueArgs)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT DEFINED ARGS_SUBDIR)
        message(FATAL_ERROR "SUBDIR needs to be defined when calling kdoctools_create_article")
    endif()

    # Init vars
    get_filename_component(docbook ${docbook} ABSOLUTE)
    file(RELATIVE_PATH src_doc ${CMAKE_CURRENT_SOURCE_DIR} ${docbook})
    get_filename_component(src_dir ${src_doc} DIRECTORY)
    get_filename_component(_name ${docbook} NAME_WE)
    set(build_dir ${CMAKE_CURRENT_BINARY_DIR}/${src_dir})
    set(build_html ${build_dir}/${_name}.html)

    # current directory is the docbook directory, but if this is empty, the
    # globs which finds the docbooks and the images will be empty too as
    # they will expand into "/*.docbook" and "/*.png"
    if (src_dir STREQUAL "")
        set(src_dir ".")
    endif ()

    # Create some place to store our files
    file(MAKE_DIRECTORY ${build_dir})

    #Bootstrap
    if (_kdoctoolsBootStrapping)
        set(_bootstrapOption "--srcdir=${KDocTools_BINARY_DIR}/src")
    elseif (CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(_bootstrapOption "--srcdir=${KDOCTOOLS_DATA_INSTALL_DIR}/kdoctools")
    else ()
        set(_bootstrapOption)
    endif ()
    set(_ssheet "${KDOCTOOLS_CUSTOMIZATION_DIR}/kde-chunk.xsl")

    add_custom_command(OUTPUT ${build_html}
        COMMAND ${KDOCTOOLS_MEINPROC_EXECUTABLE} --check ${_bootstrapOption} -o ${build_html} ${src_doc}
        DEPENDS ${src_doc} ${_ssheet}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    _kdoctools_create_target_name(_targ_html ${build_html})
    add_custom_target(${_targ_html} ALL DEPENDS ${build_html})

    set(installDest "${ARGS_INSTALL_DESTINATION}")
    if(installDest)
        set(subdir "${ARGS_SUBDIR}")
#        file(GLOB images ${src_dir}/*.png)
#        install(FILES ${build_doc} ${src_docs} ${images} DESTINATION ${installDest}/${subdir})
        install(FILES ${build_html} DESTINATION ${installDest}/${subdir})
    endif()
endfunction()

########### install files ###############
if(BUILD_KF5)
    file(READ "index.docbook" index_file)
    string(REPLACE "4.2" "4.5" index_file "${index_file}")
    string(REPLACE "kdex.dtd" "kdedbx45.dtd" index_file "${index_file}")
    file(WRITE "index-kf5.docbook" "${index_file}")
    kdoctools_create_handbook(index-kf5.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR umbrello)

    file(READ "welcome.docbook" index_file)
    string(REPLACE "4.2" "4.5" index_file "${index_file}")
    string(REPLACE "kdex.dtd" "kdedbx45.dtd" index_file "${index_file}")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/welcome.docbook "${index_file}")
    kdoctools_create_article(${CMAKE_CURRENT_BINARY_DIR}/welcome.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR umbrello)

else()
    kde4_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR umbrello)
    kdoctools_create_article(welcome.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR umbrello)
endif()

