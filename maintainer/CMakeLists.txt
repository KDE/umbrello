# to regenerate icons
if(BUILD_ICONS OR BUILD_CURSOR_ICONS)
    set(svg2png_SRCS svg2png.cpp)
    add_executable(svg2png ${svg2png_SRCS})
    target_link_libraries(svg2png 
                            Qt6::Xml 
                            Qt6::Core 
                            Qt6::Svg 
                            Qt6::Widgets)
endif()

