/***************************************************************************
                          factory.cpp  -  description
                             -------------------
    begin                : Mon Jun 17 2002
    copyright            : (C) 2002 by Luis De la Parra Blum
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
#include "cppwriter.h"
#include "javawriter.h"
#include "phpwriter.h"
#include "perlwriter.h"
#include "pythonwriter.h"
#include "adawriter.h"
#include "qstringlist.h"

#include <kdebug.h>


WriterFactory::WriterFactory( QObject* parent, const char* name )
		:KLibFactory( parent, name ) {
	s_instance = new KInstance("WriterFactory");
	kdDebug()<<"WriterFactory created"<<endl;
}

WriterFactory::~WriterFactory() {
	delete s_instance;
}

QStringList WriterFactory::languagesAvailable() {
	kdDebug()<<"Querying languages available"<<endl;
	QStringList l;
	l.append("Ada");
	l.append("Cpp");
	l.append("Java");
 	l.append("Perl");
	l.append("PHP");
	l.append("Python");
	return l;
}

QString WriterFactory::generatorName(const QString &l) {
	kdDebug()<<"Looking up generator for language "<<l<<endl;
	if(l=="Cpp")
		return "CppWriter";
	if(l=="Java")
		return "JavaWriter";
	if (l == "PHP")
		return "PHPWriter";
	if (l == "Ada")
		return "AdaWriter";
 	if (l == "Perl")
 		return "PerlWriter";
 	if (l == "Python")
 		return "PythonWriter";
	//else...
	kdDebug()<<"WriterFactory::Error: no generator for language "<<l<<endl;
	return "";
}

QObject* WriterFactory::createObject( QObject* parent, const char* name, const char *cname, const QStringList & ) {

	QString n(cname);

	kdDebug()<<"Trying to create object of type "<<n<<endl;

	QObject *obj = 0;
	if(n == "CppWriter") {
		obj = new CppWriter( parent, name );
	} else if(n =="JavaWriter") {
		obj = new JavaWriter(parent, name);
	} else if (n == "PHPWriter") {
		obj = new PhpWriter(parent, name);
	} else if (n == "AdaWriter") {
		obj = new AdaWriter(parent, name);
 	} else if (n == "PerlWriter") {
		obj = new PerlWriter(parent, name);
 	} else if (n == "PythonWriter") {
		obj = new PythonWriter(parent, name);
	} else {
		kdDebug()<<"WriterFactory:: cannot create object of type "<<n<<". Type unknown"<<endl;
	}

	return obj;
}



//this function should be named after the library name
extern "C" {
	void* init_libcodegenerator() {
		return new WriterFactory;
	}

}

KInstance* WriterFactory::s_instance = 0L;

