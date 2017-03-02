#
# generate icons from svg source
# syntax: generate_icons("<list of icons>" <size-of-icon>)
#
macro(generate_icons icons size)
    set(icons_file_list)
    if(NOT BUILD_ICONS_INTO_SIZE_RELATED_DIRS AND ${size} STREQUAL 22)
        set(dir ${CMAKE_SOURCE_DIR}/umbrello/pics)
        set(target build_icons)
   else()
        set(dir ${CMAKE_SOURCE_DIR}/umbrello/pics/${size})
        set(target build_icons-${size})
    endif()
    file(MAKE_DIRECTORY ${dir})
    foreach(icon ${icons})
        set(SOURCE_FILE ${CMAKE_SOURCE_DIR}/umbrello/pics/sources/${icon}.svg)
        set(DESTINATION_FILE ${dir}/${icon}.png)
        if(EXISTS ${SOURCE_FILE})
            message("adding command to generate bitmap for '${icon}'")
            set_source_files_properties(${DESTINATION_FILE} PROPERTIES SYMBOLIC 1)
            add_custom_command(
                OUTPUT ${DESTINATION_FILE}
                DEPENDS ${SOURCE_FILE} svg2png
                COMMAND $<TARGET_FILE:svg2png> ${SOURCE_FILE} ${DESTINATION_FILE} ${size} ${size}
                COMMENT "creating '${DESTINATION_FILE}' from '${SOURCE_FILE}'"
            )
            list(APPEND icons_file_list ${DESTINATION_FILE})
        else()
            message(WARNING "could not generate bitmap for '${icon}', source svg file '${icon}.svg' is missing")
        endif()
    endforeach(icon)
    add_custom_target(${target} DEPENDS ${icons_file_list})
    add_dependencies(umbrello ${target})
endmacro()

#
# generate cursor icons from svg source
# syntax: generate_icons("<list of cursor icons>" <size-of-icon>)
#
macro(generate_cursor_icons cursor_icons size)
    set(TEMPLATE_FILE ${CMAKE_SOURCE_DIR}/umbrello/pics/sources/cursor-template.svg)
    set(cursor_file_list)
    if(NOT BUILD_ICONS_INTO_SIZE_RELATED_DIRS AND ${size} STREQUAL 32)
        set(dir ${CMAKE_SOURCE_DIR}/umbrello/pics)
        set(target build_cursors)
   else()
        set(dir ${CMAKE_SOURCE_DIR}/umbrello/pics/${size})
        set(target build_cursors-${size})
    endif()
    file(MAKE_DIRECTORY ${dir})
    foreach(icon ${cursor_icons})
        set(SOURCE_FILE ${CMAKE_SOURCE_DIR}/umbrello/pics/sources/${icon}.svg)
        set(DESTINATION_FILE ${dir}/cursor-${icon}.png)
        if(EXISTS ${SOURCE_FILE})
            message("adding command to generate cursor bitmap for '${icon}'")
            set_source_files_properties(${DESTINATION_FILE} PROPERTIES SYMBOLIC 1)
            add_custom_command(
                OUTPUT ${DESTINATION_FILE}
                DEPENDS ${SOURCE_FILE} ${TEMPLATE_FILE} svg2png
                COMMAND $<TARGET_FILE:svg2png> ${SOURCE_FILE} ${DESTINATION_FILE} ${size} ${size} ${TEMPLATE_FILE}
                COMMENT "creating '${DESTINATION_FILE}' from '${SOURCE_FILE}'"
            )
            list(APPEND cursor_file_list ${DESTINATION_FILE})
        else()
            message(WARNING "could not generate cursor bitmap for '${icon}', source svg file '${icon}.svg' is missing")
        endif()
    endforeach(icon)

    add_custom_target(${target} DEPENDS ${cursor_file_list})
    add_dependencies(umbrello ${target})
endmacro()
