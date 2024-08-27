# to regenerate icons
if(BUILD_ICONS OR BUILD_CURSOR_ICONS)
    set(svg2png_SRCS svg2png.cpp)
    add_executable(svg2png ${svg2png_SRCS})
    target_link_libraries(svg2png Qt5:Xml Qt5:Core Qt5::Svg)
endif()

