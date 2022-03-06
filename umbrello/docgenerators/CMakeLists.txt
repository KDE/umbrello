install( FILES
    xmi1docbook.xsl
    xmi2docbook.xsl
    docbook2xhtml.xsl
    xmi.css
    common.ent
    DESTINATION ${UMBRELLO_DATA_INSTALL_DIR} )

install( DIRECTORY
    simple4125
    DESTINATION ${UMBRELLO_DATA_INSTALL_DIR} )
