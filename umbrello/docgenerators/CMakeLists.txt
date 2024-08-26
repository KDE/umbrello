install( FILES
    xmi1docbook.xsl
    xmi2docbook.xsl
    docbook2xhtml.xsl
    xmi.css
    common.ent
    DESTINATION ${UMBRELLO_KDE_INSTALL_DATAROOTDIR} )

install( DIRECTORY
    simple4125
    DESTINATION ${UMBRELLO_KDE_INSTALL_DATAROOTDIR} )
