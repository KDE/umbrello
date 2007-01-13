bin_PROGRAMS = umbrello

umbrello_COMPILE_FIRST = version.h
umbrello_SOURCES = activitywidget.cpp \
actor.cpp \
actorwidget.cpp \
aligntoolbar.cpp \
artifact.cpp \
artifactwidget.cpp \
association.cpp \
associationwidget.cpp \
assocrules.cpp \
attribute.cpp \
boxwidget.cpp \
classifier.cpp \
classifiercodedocument.cpp \
classifierlistitem.cpp \
classifierwidget.cpp \
cmdlineexportallviewsevent.cpp \
codeaccessormethod.cpp \
codeblock.cpp \
codeblockwithcomments.cpp \
codeclassfield.cpp \
codeclassfielddeclarationblock.cpp \
codecomment.cpp \
codedocument.cpp \
codegenerationpolicy.cpp \
codegenerator.cpp \
codegenobjectwithtextblocks.cpp \
codemethodblock.cpp \
codeoperation.cpp \
codeparameter.cpp \
component.cpp \
componentwidget.cpp \
configurable.cpp \
datatypewidget.cpp \
dialog_utils.cpp \
docwindow.cpp \
entity.cpp \
entityattribute.cpp \
entitywidget.cpp \
enum.cpp \
enumliteral.cpp \
enumwidget.cpp \
floatingtextwidget.cpp \
floatingtextwidgetcontroller.cpp \
folder.cpp \
forkjoinwidget.cpp \
kplayerslideraction.cpp \
hierarchicalcodeblock.cpp \
import_rose.cpp \
kstartuplogo.cpp \
linepath.cpp \
linkwidget.cpp \
listpopupmenu.cpp \
main.cpp \
messagewidget.cpp \
messagewidgetcontroller.cpp \
model_utils.cpp \
node.cpp \
nodewidget.cpp \
notewidget.cpp \
notewidgetcontroller.cpp \
objectwidget.cpp \
objectwidgetcontroller.cpp \
object_factory.cpp \
operation.cpp \
optionstate.cpp \
ownedcodeblock.cpp \
ownedhierarchicalcodeblock.cpp \
package.cpp \
packagewidget.cpp \
petalnode.cpp \
petaltree2uml.cpp \
plugin.cpp \
pluginloader.cpp \
seqlinewidget.cpp \
statewidget.cpp \
stereotype.cpp \
template.cpp \
textblock.cpp \
toolbarstate.cpp \
toolbarstatearrow.cpp \
toolbarstateassociation.cpp \
toolbarstatefactory.cpp \
toolbarstatemessages.cpp \
toolbarstateother.cpp \
toolbarstatepool.cpp \
uml.cpp \
umlattributelist.cpp \
umlentityattributelist.cpp \
umlcanvasobject.cpp \
umlclassifierlistitemlist.cpp \
umldoc.cpp \
umllistview.cpp \
umllistviewitem.cpp \
umlnamespace.cpp \
umlobject.cpp \
umlobjectlist.cpp \
umlrole.cpp \
umlview.cpp \
umlviewcanvas.cpp \
umlviewimageexporter.cpp \
umlviewimageexporterall.cpp \
umlviewimageexportermodel.cpp \
umlwidget.cpp \
umlwidgetcontroller.cpp \
uniqueid.cpp \
usecase.cpp \
usecasewidget.cpp \
widgetbase.cpp \
widget_factory.cpp \
widget_utils.cpp \
worktoolbar.cpp

#umbrello_LDADD   = ./refactoring/librefactoring.la ./codeimport/libcodeimport.la ./codeimport/kdevcppparser/libkdevcppparser.la ./clipboard/libclipboard.la ./dialogs/libdialogs.la ./codegenerators/libcodegenerator.la $(AUTOLAYOUT_LIBS) $(LIB_KDEPRINT) $(LIB_KIO)
umbrello_LDADD = ./refactoring/librefactoring.la ./codeimport/libcodeimport.la ./codeimport/kdevcppparser/libkdevcppparser.la ./clipboard/libclipboard.la ./dialogs/libdialogs.la ./codegenerators/libcodegenerator.la ./docgenerators/libdocgenerators.la $(LIB_KDEPRINT) $(LIB_KIO)

## See section "dnl Not GPL compatible" in ../configure.in.in
# SUBDIRS = $(AUTOLAYOUT_DIR) codeimport dialogs clipboard pics codegenerators headings refactoring
SUBDIRS = codeimport dialogs docgenerators clipboard pics codegenerators headings refactoring

KDE_ICON=AUTO

appdir=$(kde_datadir)/umbrello
app_DATA = tips umbrelloui.rc

xdg_apps_DATA = umbrello.desktop

mimedir = $(kde_mimedir)/application
mime_DATA = x-umbrello.desktop

#INCLUDES= -Idialogs -Irefactoring $(all_includes) -I/usr/include/graphviz/
INCLUDES= -Idialogs -Irefactoring $(all_includes)

METASOURCES = AUTO

umbrello_LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic

messages: rc.cpp
	$(PREPARETIPS) > tips.cpp
	$(EXTRACTRC) *.rc codegenerators/*.ui dialogs/*.ui > ./rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/umbrello.pot; \
	fi
	rm -f tips.cpp

version.h: $(top_srcdir)/umbrello/VERSION
	printf "#undef UMBRELLO_VERSION\n#define UMBRELLO_VERSION \"`cat $(top_srcdir)/umbrello/VERSION`\"\n" > version.h

CLEANFILES = version.h

noinst_HEADERS = version.h

