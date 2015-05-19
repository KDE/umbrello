if(NOT BUILD_KF5)
    kde4_install_icons(${ICON_INSTALL_DIR})
else()
    ecm_install_icons(
        ICONS hi16-apps-umbrello.png
              hi16-mimetypes-application-x-uml.png
              hi22-apps-umbrello.png
              hi32-apps-umbrello.png
              hi32-mimetypes-application-x-uml.png
              hi48-apps-umbrello.png
              hi64-apps-umbrello.png
              hi128-apps-umbrello.png
              hisc-apps-umbrello.svgz
        THEME hicolor
        DESTINATION ${ICON_INSTALL_DIR}
    )
endif()
