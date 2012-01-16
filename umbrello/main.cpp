/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// app includes
#include "debug_utils.h"
#include "uml.h"
#include "version.h"
#include "umldoc.h"
#include "cmdlineexportallviewsevent.h"
#include "umlviewimageexportermodel.h"
#include "umbrellosettings.h"

// kde includes
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <klocale.h>
#include <ktip.h>
#include <kwindowsystem.h>

#include <unistd.h>
#include <stdio.h>

/**
 * Description for this application
 */
static const char description[] =
    I18N_NOOP("Umbrello UML Modeller");

/**
 * Determines if the application GUI should be shown based on command line arguments.
 * @todo Add options to use the documentation generators from command line.
 *
 * @param args The command line arguments given.
 * @return True if the GUI should be shown, false otherwise.
 */
bool getShowGUI(KCmdLineArgs *args);

/**
 * Initializes the document used by the application.
 * If a file was specified in command line arguments, opens that file. Else, it
 * opens the last opened file, or a new file if there isn't any "last file used"
 * in the configuration.
 *
 * @param args The command line arguments given.
 */
void initDocument(KCmdLineArgs *args);

/**
 * Export all the views in the document using the command line args set by the user.
 * Errors that occurred while exporting, if any, are shown using uError().
 *
 * @param args The command line arguments given.
 * @param exportOpt A list containing all the "export" arguments given.
 */
void exportAllViews(KCmdLineArgs *args, const QStringList &exportOpt);

int main(int argc, char *argv[])
{
    KAboutData aboutData( "umbrello", 0, ki18n("Umbrello UML Modeller"),
                          umbrelloVersion(), ki18n(description), KAboutData::License_GPL,
                          ki18n("(c) 2001 Paul Hensgen, (c) 2002-2012 Umbrello UML Modeller Authors"), KLocalizedString(),
                          "http://uml.sf.net/");
    aboutData.addAuthor(ki18n("Paul Hensgen"),KLocalizedString(), "phensgen@users.sourceforge.net");
    aboutData.addAuthor(ki18n("Umbrello UML Modeller Authors"), KLocalizedString(), "uml-devel@lists.sourceforge.net");
    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("+[File]", ki18n("File to open"));
    options.add("export <extension>", ki18n("export diagrams to extension and exit"));
    options.add("export-formats", ki18n("list available export extensions"));
    options.add("directory <url>", ki18n("the local directory to save the exported diagrams in"), I18N_NOOP("the directory of the file"));
    options.add("use-folders", ki18n("keep the tree structure used to store the views in the document in the target directory"));
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

    KApplication app;
    if (app.isSessionRestored()) {
        kRestoreMainWindows< UMLApp >();
    } else {
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        bool showGUI = getShowGUI(args);

        UMLApp* uml = new UMLApp();
        app.processEvents();

        if (showGUI) {
            uml->show();
        }

        initDocument(args);

        if (args->isSet("export-formats")) {
            foreach(const QString& type, UMLViewImageExporterModel::supportedImageTypes())
                fprintf(stderr, "%s\n", qPrintable(type));
            return 0;
        }
        // export option
        QStringList exportOpt = args->getOptionList("export");
        if (exportOpt.size() > 0) {
             exportAllViews(args, exportOpt);
        }
    }
    return app.exec();
}

bool getShowGUI(KCmdLineArgs *args)
{
    if (args->getOptionList("export").size() > 0) {
        return false;
    }
    return true;
}

void initDocument(KCmdLineArgs *args)
{
    if ( args->count() ) {
        UMLApp::app()->openDocumentFile( args->url( 0 ) );
    } else {
        bool last = UmbrelloSettings::loadlast();
        QString file = UmbrelloSettings::lastFile();
        if( last && !file.isEmpty() ) {
            UMLApp::app()->openDocumentFile( KUrl( file ) );
        } else {
            UMLApp::app()->newDocument();
        }
    }
}

void exportAllViews(KCmdLineArgs *args, const QStringList &exportOpt)
{
    QString extension(exportOpt.last());
    uDebug() << "extension: " << extension;

    // export to the specified directory, or the directory where the file is saved
    // if no directory was specified
    KUrl directory;
    QStringList directoryOpt = args->getOptionList("directory");
    if (directoryOpt.size() > 0) {
        directory = KCmdLineArgs::makeURL(directoryOpt.last().toLocal8Bit());
    } else {
        directory = KUrl(UMLApp::app()->document()->url().directory());
    }

    bool useFolders = args->isSet("use-folders");

    uDebug() << "directory: " << directory.prettyUrl();

    // the event is posted so when the Qt loop begins it's processed. UMLApp process this event executing
    // the method it provides for exporting the views. Once all the views were exported, a quit event
    // is sent and the app finishes without user interaction
    kapp->postEvent(UMLApp::app(), new CmdLineExportAllViewsEvent(extension, directory, useFolders));
}

