
set(po2xmi_SRCS po2xmi.cpp shared.cpp)

add_executable(po2xmi ${po2xmi_SRCS})

target_link_libraries(po2xmi Qt5::Xml Qt5::Core)

install(TARGETS po2xmi  ${INSTALL_TARGETS_DEFAULT_ARGS})


set(xmi2pot_SRCS xmi2pot.cpp shared.cpp)

add_executable(xmi2pot ${xmi2pot_SRCS})

target_link_libraries(xmi2pot Qt5::Xml Qt5::Core)

install(TARGETS xmi2pot  ${INSTALL_TARGETS_DEFAULT_ARGS})


