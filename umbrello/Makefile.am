bin_PROGRAMS = umbrello

umbrello_COMPILE_FIRST = version.h
umbrello_SOURCES = activitywidget.cpp \
actor.cpp \
actorwidget.cpp \
artifact.cpp \
artifactwidget.cpp \
association.cpp \
associationwidget.cpp \
assocrules.cpp \
attribute.cpp \
boxwidget.cpp \
class.cpp \
classifier.cpp \
classifiercodedocument.cpp \
classifierlistitem.cpp \
classimport.cpp \
classwidget.cpp \
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
datatype.cpp \
datatypewidget.cpp \
docwindow.cpp \
enum.cpp \
enumliteral.cpp \
enumwidget.cpp \
floatingtext.cpp \
kplayerslideraction.cpp \
hierarchicalcodeblock.cpp \
infowidget.cpp \
interface.cpp \
interfacewidget.cpp \
kstartuplogo.cpp \
linepath.cpp \
listpopupmenu.cpp \
main.cpp \
messagewidget.cpp \
node.cpp \
nodewidget.cpp \
notewidget.cpp \
objectwidget.cpp \
operation.cpp \
ownedcodeblock.cpp \
ownedhierarchicalcodeblock.cpp \
package.cpp \
packagewidget.cpp \
plugin.cpp \
pluginloader.cpp \
seqlinewidget.cpp \
statewidget.cpp \
stereotype.cpp \
template.cpp \
textblock.cpp \
uml.cpp \
umlcanvasobject.cpp \
umldoc.cpp \
umllistview.cpp \
umllistviewitem.cpp \
umlnamespace.cpp \
umlobject.cpp \
umlrole.cpp \
umlview.cpp \
umlviewcanvas.cpp \
umlwidget.cpp \
usecase.cpp \
usecasewidget.cpp \
worktoolbar.cpp

umbrello_LDADD   = ./refactoring/librefactoring.la ./classparser/libclassparser.la ./clipboard/libclipboard.la ./dialogs/libdialogs.la ./codegenerators/libcodegenerator.la -lfl $(LIB_KDEPRINT) $(LIB_KIO)

SUBDIRS = classparser dialogs clipboard pics codegenerators headings diagram refactoring plugins

KDE_ICON=AUTO

appdir=$(kde_datadir)/umbrello
app_DATA = tips umbrelloui.rc

xdg_apps_DATA = umbrello.desktop

mimedir = $(kde_mimedir)/application
mime_DATA = x-umbrello.desktop

INCLUDES= -Idialogs -Irefactoring $(all_includes)

METASOURCES = AUTO

umbrello_LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic

messages: rc.cpp
	$(PREPARETIPS) > tips.cpp
	$(EXTRACTRC) *.rc dialogs/*.ui > ./rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/umbrello.pot; \
	fi
	rm -f tips.cpp

version.h: $(top_srcdir)/umbrello/VERSION
	printf "#undef UMBRELLO_VERSION\n#define UMBRELLO_VERSION \"`cat $(top_srcdir)/umbrello/VERSION`\"\n" > version.h

CLEANFILES = version.h

noinst_HEADERS = version.h

