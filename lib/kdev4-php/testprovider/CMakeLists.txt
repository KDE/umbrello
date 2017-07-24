
# add_definitions( -DKDE_DEFAULT_DEBUG_AREA=90xx )

########### next target ###############

set(kdevphpunitprovider_PLUGIN_SRCS
    phpunitprovider.cpp
    phpunittestsuite.cpp
    phpunitrunjob.cpp
    testdoxdelegate.cpp
)

kde4_add_plugin(kdevphpunitprovider ${kdevphpunitprovider_PLUGIN_SRCS})
target_link_libraries(kdevphpunitprovider
        ${KDEVPLATFORM_INTERFACES_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_LANGUAGE_LIBRARIES})

install(TARGETS kdevphpunitprovider DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

configure_file(kdevphpunitprovider.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevphpunitprovider.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevphpunitprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install( FILES phpunitdeclarations.php DESTINATION ${DATA_INSTALL_DIR}/kdevphpsupport )


