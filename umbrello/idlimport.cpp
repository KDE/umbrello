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
#include "operation.h"

namespace IDLImport {

QStringList source;
int srcIndex = 0;
UMLPackage *scope[32];
int scopeIndex = 0;  // index 0 is reserved for global scope
UMLClassifier *classifier = NULL;
bool isAbstract = false, isOneway = false, isReadonly = false, isAttribute = false;
Uml::Scope currentAccess = Uml::Protected;  // Protected is used as the undefined value.

void skipStmt() {
	const int srcLength = source.count();
	while (srcIndex < srcLength && source[srcIndex] != ";")
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
			QString tmp = (*tmpIt).stripWhiteSpace();
			if (tmp.isEmpty())
				continue;
			QString word;
			for (uint i = 0; i < tmp.length(); i++) {
				QChar c = tmp[i];
				if (c.isLetterOrNumber() || c == '_') {
					word += c;
				} else if (c == ':' && tmp[i + 1] == ':') {
					word += "::";
					i++;
				} else {
					if (!word.isEmpty()) {
						source.append(word);
						word = QString::null;
					}
					source.append(c);
				}
			}
			if (!word.isEmpty())
				source.append(word);
		}
	}
	const int srcLength = source.count();
	for (srcIndex = 0; srcIndex < srcLength; srcIndex++) {
		const QString& keyword = source[srcIndex];
		kdDebug() << '"' << keyword << '"' << endl;
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
			while (++srcIndex < srcLength) {
				if (source[srcIndex] == "}") {
					srcIndex++;  // skip ';'
					break;
				}
			}
			continue;
		}
		if (keyword == "enum") {
			UMLObject *ns = importer->createUMLObject(Uml::ot_Enum,
						  source[++srcIndex], scope[scopeIndex]);
			UMLEnum *enumType = static_cast<UMLEnum*>(ns);
			classifier = enumType;
			while (++srcIndex < srcLength) {
				if (source[srcIndex] == "}") {
					srcIndex++;  // skip ';'
					break;
				}
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
		if (keyword == "}") {
			if (scopeIndex)
				scopeIndex--;
			else
				kdError() << "importIDL: too many }" << endl;
			srcIndex++;  // skip ';'
			continue;
		}
		if (keyword == ";")
			continue;
		// At this point, we expect a type name (of a member of struct
		// or valuetype, or return type of an operation.)
		if (! keyword.contains( QRegExp("^\\w") )) {
			kdError() << "importIDL: ignoring " << keyword << endl;
			skipStmt();
			continue;
		}
		//UMLObject *type = NULL;
		//if (keyword != "void")
		//	type = importer->createUMLObject(Uml::ot_UMLObject,
		//					 keyword, scope[scopeIndex]);
		const QString& name = source[++srcIndex];
		if (name.contains( QRegExp("\\W") )) {
			kdError() << "importIDL: expecting name in " << name << endl;
			skipStmt();
			continue;
		}
		const QString& nxttok = source[++srcIndex];
		if (nxttok == "(") {
			// operation
			if (classifier == NULL) {
				kdError() << "importIDL: no classifier set for operation "
					  << name << endl;
				continue;
			}
    			UMLOperation *op = importer->makeOperation(classifier, name);
			if (nxttok == "(")
				srcIndex++;
			while (srcIndex < srcLength && source[srcIndex] != ")") {
				const QString &direction = source[srcIndex];
				const QString &typeName = source[++srcIndex];
				if (typeName == ")")
					break;
				const QString &parName = source[++srcIndex];
				importer->addMethodParameter( op, typeName, parName );
				if (source[++srcIndex] == ",")
					srcIndex++;
			}
    			importer->insertMethod( classifier, op, currentAccess, keyword, false, false);
			continue;
		}
		// At this point we know it's some kind of attribute declaration.
		if (currentAccess != Uml::Protected) {
			// state member of valuetype
			// TBD
			// fill in the code here...


			currentAccess = Uml::Protected;
		} else if (isAttribute) {
			// attribute of an interface
			// TBD
			// fill in the code here...


			isAttribute = false;
		} else {
			// data member of a struct
			// TBD
		}
		if (nxttok != ";") {
			kdError() << "importIDL: ignoring trailing items in " << nxttok << endl;
			skipStmt();
		}
	}
	pclose(fp);
}


}  // end namespace IDLImport


