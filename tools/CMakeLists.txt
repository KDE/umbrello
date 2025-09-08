
set(LIBS Qt::Xml Qt::Core)

set(po2xmi_SRCS po2xmi.cpp shared.cpp)

add_executable(po2xmi ${po2xmi_SRCS})
target_link_libraries(po2xmi ${LIBS})
set_target_properties(po2xmi PROPERTIES OUTPUT_NAME po2xmi${APP_SUFFIX})

install(TARGETS po2xmi  ${INSTALL_TARGETS_DEFAULT_ARGS})


set(xmi2pot_SRCS xmi2pot.cpp shared.cpp)

add_executable(xmi2pot ${xmi2pot_SRCS})
target_link_libraries(xmi2pot ${LIBS})
set_target_properties(xmi2pot PROPERTIES OUTPUT_NAME xmi2pot${APP_SUFFIX})

install(TARGETS xmi2pot  ${INSTALL_TARGETS_DEFAULT_ARGS})

ecm_mark_nongui_executable(po2xmi xmi2pot)
