// own header
#include "idlimport.h"
// qt/kde includes
#include <qprocess.h>
#include <qstringlist.h>
#include <kdebug.h>
// app includes
#include "classimport.h"
#include "uml.h"
//#include "umldoc.h"

namespace IDLImport {

void parseFile(QString filename) {
	ClassImport *importer = UMLApp::app()->classImport();
	QStringList includePaths = importer->includePathList();
	//QProcess cpp("cpp", this);
	QString command("cpp");  // C preprocessor
	for (QStringList::Iterator pathIt = includePaths.begin();
				   pathIt != includePaths.end(); ++pathIt) {
		QString path = (*pathIt);
		command += " -I" + path;
	}
	command += " " + filename;
	kdDebug() << "importIDL: " << command << endl;
	//QProcess::launch( command );
	//while (QProcess::readLineStdout
	//m_umldoc->writeToStatusBar(i18n("Importing file: %1").arg(fileName));
}


}  // end namespace IDLImport


