set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDOCTOOLS_DATA_INSTALL_DIR}/kdoctools/customization")

########### install files ###############
kdoctools_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR umbrello)

add_subdirectory(apphelp)
