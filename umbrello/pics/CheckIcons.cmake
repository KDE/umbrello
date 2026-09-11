# CheckIcons.cmake

if(NOT DEFINED ICONS)
    message(FATAL_ERROR "ICONS was not provided")
endif()

if(NOT DEFINED ICONS_QRC)
    message(FATAL_ERROR "ICONS_QRC was not provided")
endif()

file(READ "${ICONS_QRC}" QRC_CONTENT)

foreach(icon IN LISTS ICONS)
    set(png "${icon}.png")

    string(FIND "${QRC_CONTENT}" "${png}" pos)

    if(pos EQUAL -1)
        message(SEND_ERROR "Missing ${png} from ${ICONS_QRC}")
    endif()
endforeach()

if(CMAKE_ERROR_DETECTED)
    message(FATAL_ERROR "Icon validation failed")
endif()
