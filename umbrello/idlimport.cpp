/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2005                                                     *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "idlimport.h"

#include <stdio.h>
// qt/kde includes
// #include <qprocess.h>  //should use this instead of popen()
#include <qstringlist.h>
#include <qregexp.h>
#include <kdebug.h>
// app includes
#include "classimport.h"
#include "uml.h"
#include "umlpackagelist.h"
#include "package.h"
#include "interface.h"
#include "class.h"
#include "enum.h"

namespace IDLImport {

QStringList source;
int srcIndex = 0;
UMLPackage *scope[32];
int scopeIndex = 0;  // index 0 is reserved for global scope
UMLClassifier *classifier = NULL;
bool isAbstract = false, isOneway = false, isReadonly = false, isAttribute = false;
Uml::Scope currentAccess = Uml::Public;

void skipStmt() {
	const int srcLength = source.count();
	while (srcIndex < srcLength && !source[srcIndex].contains(";"))
		srcIndex++;
}

void parseFile(QString filename) {
	ClassImport *importer = UMLApp::app()->classImport();
	QStringList includePaths = importer->includePathList();
	//QProcess command("cpp", UMLAp::app());
	QString command("cpp");
	for (QStringList::Iterator pathIt = includePaths.begin();
				   pathIt != includePaths.end(); ++pathIt) {
		QString path = (*pathIt);
		//command.addArgument(" -I" + path);
		command += " -I" + path;
	}
	command += " " + filename;
	kdDebug() << "importIDL: " << command << endl;
	FILE *fp = popen(command.ascii(), "r");
	if (fp == NULL) {
		kdError() << "IDLImport::parseFile: cannot popen(" << command << ")" << endl;
		return;
	}
	source.clear();
	char buf[256];
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		int len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[--len] = '\0';
		QString line(buf);
		if (line.startsWith("#"))
			continue;
		line = line.simplifyWhiteSpace();
		if (line.isEmpty())
			continue;
		//kdDebug() << "line: " << line << endl;

		// doesn't work:
		// QStringList tmpList = QStringList::split( QRegExp("\\b"), line );
		// instead:
		QStringList tmpList = QStringList::split( QRegExp("\\s+"), line );
		for (QStringList::Iterator tmpIt = tmpList.begin(); tmpIt != tmpList.end(); ++tmpIt) {
			QString tmp = *tmpIt;
			kdDebug() << '"' << tmp << '"' << endl;
			tmp = tmp.stripWhiteSpace();
			if (tmp.isEmpty())
				continue;
			while (tmp == "::") {
				source.last() += tmp;   // append "::"
				tmp = *++tmpIt;
				source.last() += tmp;   // append the following identifier
				tmp = *++tmpIt;
			}
			source.append(tmp);
		}
		kdDebug() << "------------------" << endl;
	}
	const int srcLength = source.count();
	for (srcIndex = 0; srcIndex < srcLength; srcIndex++) {
		const QString& keyword = source[srcIndex];
		if (keyword == "module") {
			UMLObject *ns = importer->createUMLObject(Uml::ot_Package,
						  source[++srcIndex], scope[scopeIndex]);
			scope[++scopeIndex] = static_cast<UMLPackage*>(ns);
			if (source[++srcIndex] != "{")
				kdError() << "importIDL: unexpected: " << source[srcIndex] << endl;
			continue;
		}
		if (keyword == "interface") {
			UMLObject *ns = importer->createUMLObject(Uml::ot_Interface,
						  source[++srcIndex], scope[scopeIndex]);
			scope[++scopeIndex] = classifier = static_cast<UMLClassifier*>(ns);
			// inheritance: TBD
			while (++srcIndex < srcLength && source[srcIndex] != "{")
				;
			continue;
		}
		if (keyword == "struct") {
			UMLObject *ns = importer->createUMLObject(Uml::ot_Class,
						  source[++srcIndex], scope[scopeIndex]);
			scope[++scopeIndex] = classifier = static_cast<UMLClassifier*>(ns);
			continue;
		}
		if (keyword == "union" || keyword == "exception") {
			// TBD. <gulp>
			while (++srcIndex < srcLength)
				if (source[srcIndex].contains("}"))
					break;
			continue;
		}
		if (keyword == "enum") {
			UMLObject *ns = importer->createUMLObject(Uml::ot_Enum,
						  source[++srcIndex], scope[scopeIndex]);
			UMLEnum *enumType = static_cast<UMLEnum*>(ns);
			classifier = enumType;
			while (++srcIndex < srcLength) {
				if (source[srcIndex].contains("}"))
					break;
				if (source[srcIndex] == ",")
					srcIndex++;
				importer->addEnumLiteral(enumType, source[srcIndex]);
			}
			continue;
		}
		if (keyword == "typedef") {
			skipStmt();  // TBD.
			continue;
		}
		if (keyword == "const") {
			skipStmt();
			continue;
		}
		if (keyword == "abstract") {
			isAbstract = true;
			continue;
		}
		if (keyword == "valuetype") {
			UMLObject *ns = importer->createUMLObject(Uml::ot_Class,
						  source[++srcIndex], scope[scopeIndex]);
			scope[++scopeIndex] = classifier = static_cast<UMLClassifier*>(ns);
			classifier->setAbstract(isAbstract);
			isAbstract = false;
			// inheritance: TBD
			while (++srcIndex < srcLength && source[srcIndex] != "{")
				;
			continue;
		}
		if (keyword == "public") {
			currentAccess = Uml::Public;
			continue;
		}
		if (keyword == "private") {
			currentAccess = Uml::Private;
			continue;
		}
		if (keyword == "readonly") {
			isReadonly = true;
			continue;
		}
		if (keyword == "attribute") {
			isAttribute = true;
			continue;
		}
		if (keyword == "oneway") {
			isOneway = true;
			continue;
		}
		if (keyword.contains("}")) {
			if (scopeIndex)
				scopeIndex--;
			else
				kdError() << "importIDL: too many }" << endl;
			continue;
		}
		// At this point, we expect a type name (of a member of struct
		// or valuetype, or return type of an operation.)
		skipStmt();  // TBD
	}
	pclose(fp);
}


}  // end namespace IDLImport


