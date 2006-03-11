kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/umbrello/headings FILES heading.adb  heading.ads  heading.as  heading.cpp  heading.cs  heading.h  heading.idl  heading.java  heading.js  heading.php  heading.pm  heading.py  heading.rb  heading.sql  heading.xsd )

kde4_footer()



#original Makefile.am contents follow:

#mydir = $(kde_datadir)/umbrello/headings
#my_DATA = heading.adb \
#heading.ads \
#heading.as \
#heading.cpp \
#heading.cs \
#heading.h \
#heading.idl \
#heading.java \
#heading.js \
#heading.php \
#heading.pm \
#heading.py \
#heading.rb \
#heading.sql \
#heading.xsd
