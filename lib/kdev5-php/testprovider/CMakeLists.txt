########### next target ###############

set(kdevphpunitprovider_PLUGIN_SRCS
    phpunitprovider.cpp
    phpunittestsuite.cpp
    phpunitrunjob.cpp
    testdoxdelegate.cpp
    testproviderdebug.cpp
)

add_library(kdevphpunitprovider MODULE ${kdevphpunitprovider_PLUGIN_SRCS})

target_link_libraries(kdevphpunitprovider LINK_PRIVATE
    KDev::Interfaces
    KDev::Language
    KDev::Project
    KDev::OutputView
    KDev::Util
)

install(TARGETS kdevphpunitprovider DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )

########### install files ###############

configure_file(kdevphpunitprovider.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevphpunitprovider.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevphpunitprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install( FILES phpunitdeclarations.php DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kdevphpsupport )


