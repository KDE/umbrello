/***************************************************************************
                          codegenfactory.cpp  -  description
                             -------------------
    begin                : Mon Jun 17 2002
    copyright            : (C) 2002 by Luis De la Parra Blum
					and Brian Thomas
    email                : luis@delaparra.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../codegenerator.h"

// the new 
#include "cppcodegenerator.h"
#include "javacodegenerator.h"
#include "xmlschemacodegenerator.h"

// the old
#include "adawriter.h"
// #include "cswriter.h" // missing in action?!? 
#include "idlwriter.h"
#include "phpwriter.h"
#include "perlwriter.h"
#include "pythonwriter.h"
#include "sqlwriter.h"
#include "aswriter.h"
#include "jswriter.h"

#include "qstringlist.h"
#include <kdebug.h>

CodeGeneratorFactory::CodeGeneratorFactory( QObject* parent, const char* name )
		:KLibFactory( parent, name ) {
	s_instance = new KInstance("CodeGeneratorFactory");
	kdDebug()<<"CodeGeneratorFactory created"<<endl;
}

CodeGeneratorFactory::~CodeGeneratorFactory() {
	delete s_instance;
}

QStringList CodeGeneratorFactory::languagesAvailable() {
	kdDebug()<<"Querying languages available"<<endl;

	QStringList l;
	l.append("Ada");
	l.append("ActionScript");
	l.append("Cpp");
//	l.append("C#");
	l.append("IDL");
	l.append("Java");
	l.append("JavaScript");
 	l.append("Perl");
	l.append("PHP");
	l.append("Python");
	l.append("SQL");
	l.append("XMLSchema");
	return l;
}

QString CodeGeneratorFactory::generatorName(const QString &l) {
	kdDebug()<<"Looking up generator for language "<<l<<endl;
	if (l=="Java")
		return "JavaCodeGenerator";
	if (l=="Cpp")
		return "CppCodeGenerator";
	if (l == "Ada")
		return "AdaWriter";
	if (l == "ActionScript")
		return "ASWriter";
/*
	if (l=="C#")
		return "C#Writer";
*/
	if (l=="IDL")
		return "IDLWriter";
	if (l == "JavaScript")
		return "JSWriter";
	if (l == "PHP")
		return "PHPWriter";
 	if (l == "Perl")
 		return "PerlWriter";
 	if (l == "Python")
 		return "PythonWriter";
 	if (l == "SQL")
 		return "SQLWriter";
 	if (l == "XMLSchema")
 		return "XMLSchemaCodeGenerator";
	//else...
	kdDebug()<<"CodeGeneratorFactory::Error: no generator for language "<<l<<endl;
	return "";
}

QObject* CodeGeneratorFactory::createObject ( QObject* parent, const char* name, 
                                                 const char *cname, const QStringList & 
                                            ) 
{

	QString n(cname);

	kdDebug()<<"Trying to create object of type "<<n<<endl;

	QObject *obj = 0;
	UMLDoc * doc = dynamic_cast<UMLDoc *>(parent);

	if(doc) {

		if(n =="JavaCodeGenerator") {
			obj = new JavaCodeGenerator(doc, name);
		} else if( n == "CppCodeGenerator") {
			obj = new CPPCodeGenerator(doc, name);
 		} else if (n == "XMLSchemaCodeGenerator") {
			obj = new XMLSchemaCodeGenerator(doc, name);
		} else if (n == "AdaWriter") {
			obj = new AdaWriter(doc, name);
		} else if(n == "ASWriter") {
			obj = new ASWriter( doc, name );
//		} else if(n == "C#Writer") {
//			obj = new CsWriter( doc, name );
		} else if(n == "IDLWriter") {
			obj = new IDLWriter( doc, name );
		} else if(n == "JSWriter") {
			obj = new JSWriter( doc, name );
		} else if (n == "PHPWriter") {
			obj = new PhpWriter( doc, name);
 		} else if (n == "PerlWriter") {
			obj = new PerlWriter( doc, name);
 		} else if (n == "PythonWriter") {
			obj = new PythonWriter( doc, name);
 		} else if (n == "SQLWriter") {
			obj = new SQLWriter( doc, name);
		} else {
			kdDebug()<<"CodeGeneratorFactory:: cannot create object of type "<<n<<". Type unknown"<<endl;
		}

	} else {
		kdDebug()<<"CodeGeneratorFactory:: cannot create parent UML document"<<endl;
	}
	return obj;
}

//this function should be named after the library name
extern "C" {
	void* init_libcodegenerator() {
		return new CodeGeneratorFactory;
	}

}

KInstance* CodeGeneratorFactory::s_instance = 0L;

