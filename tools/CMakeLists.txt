
set(po2xmi_SRCS po2xmi.cpp shared.cpp)

kde4_add_executable(po2xmi ${po2xmi_SRCS})

target_link_libraries(po2xmi ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY})

install(TARGETS po2xmi  ${INSTALL_TARGETS_DEFAULT_ARGS})


set(xmi2pot_SRCS xmi2pot.cpp shared.cpp)

kde4_add_executable(xmi2pot ${xmi2pot_SRCS})

target_link_libraries(xmi2pot ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY})

install(TARGETS xmi2pot  ${INSTALL_TARGETS_DEFAULT_ARGS})


