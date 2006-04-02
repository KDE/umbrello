/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                 *
 ***************************************************************************/

#include "uml.h"
#include "version.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlviewimageexportermodel.h"
#include "kstartuplogo.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <klocale.h>
#include <ktip.h>
#include <kdebug.h>
#include <kwin.h>

#include <unistd.h>

static const char description[] =
    I18N_NOOP("Umbrello UML Modeller");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


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
 * Export all the views in the document using the command line args set by the user.
 * The errors occured while exporting, if any, are shown using kdError().
 */
void exportAllViews(KCmdLineArgs *args, const QCStringList &exportOpt);

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
        KStartupLogo* start_logo = 0L;
        UMLApp *uml = new UMLApp();
        KConfig * cfg = app.config();
        //see if we want a logo shown
        cfg -> setGroup( "General Options" );
        bool showLogo = cfg -> readBoolEntry( "logo", true );
        if (showLogo) {
#if KDE_IS_VERSION(3,3,90)
            start_logo = new KStartupLogo(0);
            start_logo->setHideEnabled(true);
            KWin::setMainWindow(start_logo, uml->winId());
#else
            start_logo = new KStartupLogo(uml);
            start_logo->setHideEnabled(true);
#endif
            KWin::setState(start_logo->winId(), NET::KeepAbove);
            start_logo->show();
            QApplication::flushX();
        }
        uml->show();
        uml->initGenerators();

        //show tips if wanted
        KTipDialog::showTip();

        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        if ( args -> count() ) {
            uml -> openDocumentFile( args -> url( 0 ) );
            // args -> clear();  // Why is this necessary?
        } else {
            cfg -> setGroup( "General Options" );
            bool last = cfg -> readBoolEntry( "loadlast", false );
            QString file = cfg -> readPathEntry( "lastFile" );
            if( last && !file.isEmpty() ) {
                uml->openDocumentFile( KURL( file ) );
            } else {
                uml->newDocument();
            }
        }

        // export option
        QCStringList exportOpt = args->getOptionList("export");
        if (exportOpt.size() > 0) {
             exportAllViews(args, exportOpt);
             kapp->processEvents();
             return 0;
        }

        if ( showLogo && !start_logo->isHidden() ) {
            start_logo->raise();
        }
    }
    return app.exec();
}

void exportAllViews(KCmdLineArgs *args, const QCStringList &exportOpt) {
    QString extension(exportOpt.last());
    kdDebug() << "extension: " << extension << endl;

    // export to the specified directory, or the directory where the file is saved
    // if no directory was specified
    KURL directory;
    QCStringList directoryOpt = args->getOptionList("directory");
    if (directoryOpt.size() > 0) {
        directory = KURL(directoryOpt.last());
    } else {
        directory = KURL(UMLApp::app()->getDocument()->URL().directory());
    }

    kdDebug() << "directory: " << directory.prettyURL() << endl;
    QStringList errors = UMLViewImageExporterModel().exportAllViews(extension, directory, args->isSet("use-folders"));
    if (!errors.isEmpty()) {
        kdError() << "Errors while exporting:" << endl;
        for (QStringList::Iterator it = errors.begin(); it != errors.end(); ++it) {
            kdError() << *it << endl;
        }
    }
}
