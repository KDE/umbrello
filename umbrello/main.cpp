/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "uml.h"
#include "version.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <klocale.h>
#include "kstartuplogo.h"
#include <ktip.h>
#include <unistd.h>

static const char *description =
    I18N_NOOP("Umbrello UML Modeller");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


static KCmdLineOptions options[] =
    {
        { "+[File]", I18N_NOOP("file to open"), 0 },
        { 0, 0, 0 }
        // INSERT YOUR COMMANDLINE OPTIONS HERE
    };

int main(int argc, char *argv[]) {

	KAboutData aboutData( "umbrello", I18N_NOOP("Umbrello UML Modeller"),
	                      VERSION, description, KAboutData::License_GPL,
	                      "(c) 2001 Paul Hensgen, (c) 2002 Umbrello UML Modeller Authors", 0,
	                      "http://uml.sf.net/");
	aboutData.addAuthor("Paul Hensgen",0, "phensgen@users.sourceforge.net");
	aboutData.addAuthor("Umbrello UML Modeller Authors",0, "uml-devel@lists.sourceforge.net");
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
			start_logo = new KStartupLogo();
			start_logo->setHideEnabled(true);
			start_logo->show();
			start_logo->raise();
			QApplication::flushX();
		}
		uml->show();
		uml->initLibraries();

		//show tips if wanted
		KTipDialog::showTip();

		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
		if ( args -> count() ) {
			uml -> openDocumentFile( args -> url( 0 ) );
			args -> clear();
		} else {
			cfg -> setGroup( "General Options" );
			bool last = cfg -> readBoolEntry( "loadlast", false );
			QString file = cfg -> readEntry( "lastFile", "" );
			if( last && !file.isEmpty() ) {
				uml->openDocumentFile( KURL( file ) );
			} else {
				uml->newDocument();
			}
		}
		if ( showLogo && !start_logo->isHidden() ) {
			start_logo->raise();
		}
	}
	return app.exec();
}
