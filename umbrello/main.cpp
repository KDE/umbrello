/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include <unistd.h>

// kde includes
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <klocale.h>
#include <ktip.h>
#include <kdebug.h>
#include <kwm.h>

// app includes
#include "uml.h"
#include "version.h"
#include "umldoc.h"
#include "cmdlineexportallviewsevent.h"


static const char description[] =
    I18N_NOOP("Umbrello UML Modeller");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE

/**
 * @todo Add options to use the documentation generators from command line.
 */
static KCmdLineOptions options[] =
    {
        { "+[File]", I18N_NOOP("File to open"), 0 },
        { "export <extension>", I18N_NOOP("export diagrams to extension and exit"), 0},
        { "directory <url>", I18N_NOOP("the local directory to save the exported diagrams in"), I18N_NOOP("the directory of the file")},
        { "use-folders", I18N_NOOP("keep the tree structure used to store the views in the document in the target directory"), 0},
        // INSERT YOUR COMMANDLINE OPTIONS HERE
        KCmdLineLastOption
    };

/**
 * Determines if the application GUI should be shown based on command line arguments.
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
 * @param cfg The application configuration.
 */
void initDocument(KCmdLineArgs *args, KConfig* cfg);

/**
 * Export all the views in the document using the command line args set by the user.
 * Errors that occurred while exporting, if any, are shown using kError().
 *
 * @param args The command line arguments given.
 * @param exportOpt A list containing all the "export" arguments given.
 */
void exportAllViews(KCmdLineArgs *args, const QByteArrayList &exportOpt);

extern "C" int flushEvents() {
    qApp->processEvents();
    return 0;
}

int main(int argc, char *argv[]) {
    KAboutData aboutData( "umbrello", I18N_NOOP("Umbrello UML Modeller"),
                          UMBRELLO_VERSION, description, KAboutData::License_GPL,
                          I18N_NOOP("(c) 2001 Paul Hensgen, (c) 2002-2006 Umbrello UML Modeller Authors"), 0,
                          "http://uml.sf.net/");
    aboutData.addAuthor("Paul Hensgen",0, "phensgen@users.sourceforge.net");
    aboutData.addAuthor(I18N_NOOP("Umbrello UML Modeller Authors"), 0, "uml-devel@lists.sourceforge.net");
    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

    KApplication app;
    if( app.isSessionRestored() ) {
        RESTORE( UMLApp );
    } else {
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        bool showGUI = getShowGUI(args);

        UMLApp *uml = new UMLApp();
        flushEvents();
        KConfig * cfg = app.sessionConfig();

       
        if (showGUI) {
            uml->show();
        }
        uml->initGenerator();

        
        initDocument(args, cfg);

        // export option
        QByteArrayList exportOpt = args->getOptionList("export");
        if (exportOpt.size() > 0) {
             exportAllViews(args, exportOpt);
        }

       
    }
    return app.exec();
}

bool getShowGUI(KCmdLineArgs *args) {
    if (args->getOptionList("export").size() > 0) {
        return false;
    }

    return true;
}


void initDocument(KCmdLineArgs *args, KConfig* cfg) {
    if ( args -> count() ) {
        UMLApp::app()->openDocumentFile( args->url( 0 ) );
    } else {
        cfg->setGroup( "General Options" );
        bool last = cfg->readEntry( "loadlast", false );
        QString file = cfg->readPathEntry( "lastFile" );
        if( last && !file.isEmpty() ) {
            UMLApp::app()->openDocumentFile( KUrl( file ) );
        } else {
            UMLApp::app()->newDocument();
        }
    }
}


void exportAllViews(KCmdLineArgs *args, const QByteArrayList &exportOpt) {
    QString extension(exportOpt.last());
    kDebug() << "extension: " << extension << endl;

    // export to the specified directory, or the directory where the file is saved
    // if no directory was specified
    KUrl directory;
    QByteArrayList directoryOpt = args->getOptionList("directory");
    if (directoryOpt.size() > 0) {
        directory = KCmdLineArgs::makeURL(directoryOpt.last());
    } else {
        directory = KUrl(UMLApp::app()->getDocument()->url().directory());
    }

    bool useFolders = args->isSet("use-folders");

    kDebug() << "directory: " << directory.prettyUrl() << endl;

    // the event is posted so when the QT loop begins it's processed. UMLApp process this event executing
    // the method it provides for exporting the views. Once all the views were exported, a quit event
    // is sent and the app finishes without user interaction
    kapp->postEvent(UMLApp::app(), new CmdLineExportAllViewsEvent(extension, directory, useFolders));
}

