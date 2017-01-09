if (NOT BUILD_KF5)
    set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDE4_DATA_INSTALL_DIR}/ksgmltools2/customization")
else()
    set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDOCTOOLS_DATA_INSTALL_DIR}/kdoctools/customization")
endif()

########### install files ###############
if(BUILD_KF5)
    file(READ "index.docbook" index_file)
    string(REPLACE "4.2" "4.5" index_file "${index_file}")
    string(REPLACE "kdex.dtd" "kdedbx45.dtd" index_file "${index_file}")
    file(WRITE "index-kf5.docbook" "${index_file}")
    kdoctools_create_handbook(index-kf5.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR umbrello)
else()
    kde4_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR umbrello)
endif()

add_subdirectory(apphelp)
