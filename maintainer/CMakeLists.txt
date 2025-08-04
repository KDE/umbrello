# to regenerate icons
if(BUILD_ICONS OR BUILD_CURSOR_ICONS)
    set(svg2png_SRCS svg2png.cpp)
    add_executable(svg2png ${svg2png_SRCS})
    target_link_libraries(svg2png Qt${QT_MAJOR_VERSION}::Xml Qt${QT_MAJOR_VERSION}::Core Qt${QT_MAJOR_VERSION}::Svg)
endif()

