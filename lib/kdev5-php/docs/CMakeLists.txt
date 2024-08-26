########## shared settings ##########
set(kdevphpdocs_settings_SRCS
)

kconfig_add_kcfg_files( kdevphpdocs_settings_SRCS phpdocssettings.kcfgc )

########## Documentation Plugin #####

set(kdevphpdocs_SRCS
    phpdocsdebug.cpp
    phpdocumentationwidget.cpp
    phpdocumentation.cpp
    phpdocsplugin.cpp
    phpdocsmodel.cpp
    ${kdevphpdocs_settings_SRCS}
)

kdevplatform_add_plugin(kdevphpdocs JSON kdevphpdocs.json
    SOURCES ${kdevphpdocs_SRCS})
target_link_libraries(kdevphpdocs
    KDev::Interfaces
    KDev::Language
    KDev::Documentation
    KF6::KCMUtils
)

### Configuration module for PHP documentation plugin

set(kcm_kdevphpdocs_SRCS
    phpdocsconfig.cpp
    ${kdevphpdocs_settings_SRCS}
)

ki18n_wrap_ui(kcm_kdevphpdocs_SRCS phpdocsconfig.ui)

add_library( kcm_kdevphpdocs MODULE ${kcm_kdevphpdocs_SRCS} )
target_link_libraries( kcm_kdevphpdocs
    KF6::ConfigWidgets
    KF6::KIOWidgets
    KF6::I18n
    KF6::KCMUtils
    Qt6::Core
)

configure_file(kcm_kdevphpdocs.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdevphpdocs.desktop)
kcoreaddons_desktop_to_json(kcm_kdevphpdocs ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdevphpdocs.desktop)
install( TARGETS kcm_kdevphpdocs DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION}/kcm )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdevphpdocs.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
