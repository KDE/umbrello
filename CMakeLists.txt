project(umbrello)
add_subdirectory( umbrello ) 




########### install files ###############





#original Makefile.am contents follow:

######## kdevelop will overwrite this part!!! (begin)##########
#
#SUBDIRS = umbrello
#
######## kdevelop will overwrite this part!!! (end)############
## not a GNU package. You can remove this line, if 
## have all needed files, that a GNU package needs
#AUTOMAKE_OPTIONS = foreign 
#
#AM_CPPFLAGS = $(QT_INCLUDES)
#VERSION:=`cat ${PWD}/VERSION`
#
