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
#include <kwin.h>

// app includes
#include "uml.h"
#include "version.h"
#include "umldoc.h"
#include "cmdlineexportallviewsevent.h"
#include "kstartuplogo.h"

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
 * Creates, shows and returns the startup logo for the application if it should be shown,
 * else returns null pointer.
 * The startup logo is shown if it is configured to be shown and also the GUI must be shown.
 *
 * @param cfg The application configuration.
 * @param showGUI If the GUI should be shown.
 * @return The startup logo for the application, or a null pointer if it shouldn't be shown.
 */
KStartupLogo* showStartupLogo(KConfig* cfg, bool showGUI);

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
void exportAllViews(KCmdLineArgs *args, const QCStringList &exportOpt);

extern "C" int flushEvents() {
    kapp->processEvents();
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
    if( app.isRestored() ) {
        RESTORE( UMLApp );
    } else {
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        bool showGUI = getShowGUI(args);

        UMLApp *uml = new UMLApp();
        flushEvents();
        KConfig * cfg = app.config();

        KStartupLogo* startLogo = showStartupLogo(cfg, showGUI);

        if (showGUI) {
            uml->show();
        }
        uml->initGenerator();

        //show tips if wanted
        if (showGUI) {
            KTipDialog::showTip();
        }

        initDocument(args, cfg);

        // export option
        QCStringList exportOpt = args->getOptionList("export");
        if (exportOpt.size() > 0) {
             exportAllViews(args, exportOpt);
        }

        if ( showGUI && (startLogo != 0L) && !startLogo->isHidden() ) {
            startLogo->raise();
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

KStartupLogo* showStartupLogo(KConfig* cfg, bool showGUI) {
    KStartupLogo* startLogo = 0L;

    cfg->setGroup( "General Options" );
    bool showLogo = cfg->readBoolEntry( "logo", true );
    if (showGUI && showLogo) {
#if KDE_IS_VERSION(3,3,90)
        startLogo = new KStartupLogo(0);
        startLogo->setHideEnabled(true);
        KWin::setMainWindow(startLogo, UMLApp::app()->winId());
#else
        startLogo = new KStartupLogo(UMLApp::app());
        startLogo->setHideEnabled(true);
#endif
        KWin::setState(startLogo->winId(), NET::KeepAbove);
        startLogo->show();
        QApplication::flushX();
    }

    return startLogo;
}

void initDocument(KCmdLineArgs *args, KConfig* cfg) {
    if ( args -> count() ) {
        UMLApp::app()->openDocumentFile( args->url( 0 ) );
    } else {
        cfg->setGroup( "General Options" );
        bool last = cfg->readBoolEntry( "loadlast", false );
        QString file = cfg->readPathEntry( "lastFile" );
        if( last && !file.isEmpty() ) {
            UMLApp::app()->openDocumentFile( KURL( file ) );
        } else {
            UMLApp::app()->newDocument();
        }
    }
}

void exportAllViews(KCmdLineArgs *args, const QCStringList &exportOpt) {
    QString extension(exportOpt.last());
    kDebug() << "extension: " << extension << endl;

    // export to the specified directory, or the directory where the file is saved
    // if no directory was specified
    KURL directory;
    QCStringList directoryOpt = args->getOptionList("directory");
    if (directoryOpt.size() > 0) {
        directory = KCmdLineArgs::makeURL(directoryOpt.last());
    } else {
        directory = KURL(UMLApp::app()->getDocument()->URL().directory());
    }

    bool useFolders = args->isSet("use-folders");

    kDebug() << "directory: " << directory.prettyURL() << endl;

    // the event is posted so when the QT loop begins it's processed. UMLApp process this event executing
    // the method it provides for exporting the views. Once all the views were exported, a quit event
    // is sent and the app finishes without user interaction
    kapp->postEvent(UMLApp::app(), new CmdLineExportAllViewsEvent(extension, directory, useFolders));
}

