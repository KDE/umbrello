/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.				   *
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
#include "attribute.h"

namespace IDLImport {

QStringList source;
int srcIndex = 0;
UMLPackage *scope[32];
int scopeIndex = 0;  // index 0 is reserved for global scope
UMLClass *klass = NULL;
bool isAbstract = false, isOneway = false, isReadonly = false, isAttribute = false;
Uml::Scope currentAccess = Uml::Public;

void skipStmt(QString until = ";") {
	const int srcLength = source.count();
	while (srcIndex < srcLength && source[srcIndex] != until)
		srcIndex++;
}

/// Check for split type names (e.g. unsigned long long)
QString joinTypename() {
	QString typeName = source[srcIndex];
	if (source[srcIndex] == "unsigned")
		typeName += ' ' + source[++srcIndex];
	if (source[srcIndex] == "long" &&
	    (source[srcIndex + 1] == "long" || source[srcIndex + 1] == "double"))
		typeName += ' ' + source[++srcIndex];
	return typeName;
}

/// The lexer. Tokenizes the given string and fills `source'.
void scan(QString line) {
	if (line.startsWith("#"))
		return;
	line = line.simplifyWhiteSpace();
	if (line.isEmpty())
		return;
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
	// Scan the input file into the QStringList source.
	source.clear();
	char buf[256];
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		int len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[--len] = '\0';
		scan( QString(buf) );
	}
	// Parse the QStringList source.
	const int srcLength = source.count();
	for (srcIndex = 0; srcIndex < srcLength; srcIndex++) {
		const QString& keyword = source[srcIndex];
		kdDebug() << '"' << keyword << '"' << endl;
		if (keyword == "module") {
			UMLObject *ns = importer->createUMLObject(Uml::ot_Package,
						  source[++srcIndex], scope[scopeIndex]);
			scope[++scopeIndex] = static_cast<UMLPackage*>(ns);
			scope[scopeIndex]->setStereotype("CORBAModule");
			if (source[++srcIndex] != "{") {
				kdError() << "importIDL: unexpected: " << source[srcIndex] << endl;
				skipStmt("{");
			}
			continue;
		}
		if (keyword == "interface") {
			const QString& name = source[++srcIndex];
			UMLObject *ns = importer->createUMLObject(Uml::ot_Class,
								  name, scope[scopeIndex]);
			scope[++scopeIndex] = klass = static_cast<UMLClass*>(ns);
			klass->setStereotype("CORBAInterface");
			klass->setAbstract(isAbstract);
			isAbstract = false;
			if (source[++srcIndex] == ";")   // forward declaration
				continue;
			if (source[srcIndex] == ":") {
				while (++srcIndex < srcLength && source[srcIndex] != "{") {
					const QString& baseName = source[srcIndex];
					importer->createGeneralization(klass, baseName);
					if (source[++srcIndex] != ",")
						break;
				}
			}
			if (source[srcIndex] != "{") {
				kdError() << "importIDL: ignoring excess chars at "
					  << name << endl;
				skipStmt("{");
			}
			continue;
		}
		if (keyword == "struct" || keyword == "exception") {
			const QString& name = source[++srcIndex];
			UMLObject *ns = importer->createUMLObject(Uml::ot_Class,
								  name, scope[scopeIndex]);
			scope[++scopeIndex] = klass = static_cast<UMLClass*>(ns);
			if (keyword == "struct")
				klass->setStereotype("CORBAStruct");
			else
				klass->setStereotype("CORBAException");
			if (source[++srcIndex] != "{") {
				kdError() << "importIDL: expecting '{' at " << name << endl;
				skipStmt("{");
			}
			continue;
		}
		if (keyword == "union") {
			// TBD. <gulp>
			skipStmt("}");
			srcIndex++;  // advance to ';'
			continue;
		}
		if (keyword == "enum") {
			UMLObject *ns = importer->createUMLObject(Uml::ot_Enum,
						  source[++srcIndex], scope[scopeIndex]);
			UMLEnum *enumType = static_cast<UMLEnum*>(ns);
			srcIndex++;  // skip name
			while (++srcIndex < srcLength && source[srcIndex] != "}") {
				importer->addEnumLiteral(enumType, source[srcIndex]);
				if (source[++srcIndex] != ",")
					break;
			}
			skipStmt();
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
		if (keyword == "custom") {
			continue;
		}
		if (keyword == "abstract") {
			isAbstract = true;
			continue;
		}
		if (keyword == "valuetype") {
			const QString& name = source[++srcIndex];
			UMLObject *ns = importer->createUMLObject(Uml::ot_Class,
								  name, scope[scopeIndex]);
			scope[++scopeIndex] = klass = static_cast<UMLClass*>(ns);
			klass->setAbstract(isAbstract);
			isAbstract = false;
			if (source[++srcIndex] == ";")   // forward declaration
				continue;
			if (source[srcIndex] == ":") {
				if (source[++srcIndex] == "truncatable")
					srcIndex++;
				while (srcIndex < srcLength && source[srcIndex] != "{") {
					const QString& baseName = source[srcIndex];
					importer->createGeneralization(klass, baseName);
					if (source[++srcIndex] != ",")
						break;
					srcIndex++;
				}
			}
			if (source[srcIndex] != "{") {
				kdError() << "importIDL: ignoring excess chars at "
					  << name << endl;
				skipStmt("{");
			}
			continue;
		}
		if (keyword == "public") {
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
		// At this point, we expect `keyword' to be a type name
		// (of a member of struct or valuetype, or return type
		// of an operation.) Up next is the name of the attribute
		// or operation.
		if (! keyword.contains( QRegExp("^\\w") )) {
			kdError() << "importIDL: ignoring " << keyword << endl;
			skipStmt();
			continue;
		}
		QString typeName = joinTypename();
		QString name = source[++srcIndex];
		if (name.contains( QRegExp("\\W") )) {
			kdError() << "importIDL: expecting name in " << name << endl;
			skipStmt();
			continue;
		}
		// At this point we most definitely need a class.
		if (klass == NULL) {
			kdError() << "importIDL: no class set for " << name << endl;
			continue;
		}
		if (source[++srcIndex] == "(") {
			// operation
    			UMLOperation *op = importer->makeOperation(klass, name);
			srcIndex++;
			while (srcIndex < srcLength && source[srcIndex] != ")") {
				const QString &direction = source[srcIndex++];
				QString typeName = joinTypename();
				const QString &parName = source[++srcIndex];
				UMLAttribute *att = importer->addMethodParameter(op, typeName, parName);
				Uml::Parameter_Direction dir;
				if (Umbrello::stringToDirection(direction, dir))
					att->setParmKind(dir);
				else
					kdError() << "importIDL: expecting parameter direction at "
						  << direction << endl;
				if (source[++srcIndex] != ",")
					break;
				srcIndex++;
			}
    			importer->insertMethod( klass, op, Uml::Public, typeName, false, false);
			skipStmt();  // skip possible "raises" clause
			continue;
		}
		// At this point we know it's some kind of attribute declaration.
		importer->insertAttribute( klass, currentAccess, name, typeName);
		currentAccess = Uml::Public;
		if (source[srcIndex] != ";") {
			kdError() << "importIDL: ignoring trailing items at " << name << endl;
			skipStmt();
		}
	}
	pclose(fp);
}


}  // end namespace IDLImport


