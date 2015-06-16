
if(NOT BUILD_KF5)
    set(LIBS Qt4::QtXml Qt4::QtCore)
else()
    set(LIBS Qt5::Xml Qt5::Core)
endif()

set(po2xmi_SRCS po2xmi.cpp shared.cpp)

ecm_add_executable(po2xmi ${po2xmi_SRCS})
target_link_libraries(po2xmi ${LIBS})

install(TARGETS po2xmi  ${INSTALL_TARGETS_DEFAULT_ARGS})


set(xmi2pot_SRCS xmi2pot.cpp shared.cpp)

ecm_add_executable(xmi2pot ${xmi2pot_SRCS})
target_link_libraries(xmi2pot ${LIBS})

install(TARGETS xmi2pot  ${INSTALL_TARGETS_DEFAULT_ARGS})


