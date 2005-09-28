/***************************************************************************
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

#include "codegenfactory.h"
#include "../codegenerator.h"
#include "../umldoc.h"
#include "../uml.h"
#include "../optionstate.h"

// the new
#include "cppcodegenerator.h"
#include "javacodegenerator.h"
#include "rubycodegenerator.h"
#include "xmlschemawriter.h"

#include "adawriter.h"
#include "cppwriter.h"
// #include "cswriter.h" // missing in action?!?
#include "idlwriter.h"
#include "javawriter.h"
#include "phpwriter.h"
#include "php5writer.h"
#include "perlwriter.h"
#include "pythonwriter.h"
#include "rubywriter.h"
#include "sqlwriter.h"
#include "aswriter.h"
#include "jswriter.h"
#include "tclwriter.h"

#include "qstringlist.h"
#include <kdebug.h>

CodeGeneratorFactory::CodeGeneratorFactory()  {
    kdDebug()<<"CodeGeneratorFactory created"<<endl;
}

CodeGeneratorFactory::~CodeGeneratorFactory() {
}

QStringList CodeGeneratorFactory::languagesAvailable() {
    kdDebug()<<"Querying languages available"<<endl;

    QStringList l;
    l.append("ActionScript");
    l.append("Ada");
    l.append("Cpp");
    //  l.append("C#");
    l.append("IDL");
    l.append("Java");
    l.append("JavaScript");
    l.append("Perl");
    l.append("PHP");
    l.append("PHP5");
    l.append("Python");
    l.append("Ruby");
    l.append("SQL");
    l.append("Tcl");
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
    if (l == "PHP5")
        return "PHP5Writer";
    if (l == "Perl")
        return "PerlWriter";
    if (l == "Python")
        return "PythonWriter";
    if (l == "Ruby")
        return "RubyWriter";
    if (l == "SQL")
        return "SQLWriter";
    if (l == "Tcl")
        return "TclWriter";
    if (l == "XMLSchema")
        return "XMLSchemaWriter";
    //else...
    kdDebug()<<"CodeGeneratorFactory::Error: no generator for language "<<l<<endl;
    return "";
}

CodeGenerator* CodeGeneratorFactory::createObject(UMLDoc* doc, const char* name)  {
    CodeGenerator* obj = 0;
    QString cname(name);

    if (doc) {
        Settings::OptionState optionState = UMLApp::app()->getOptionState();
        if (cname == "AdaWriter") {
            obj = new AdaWriter(doc, name);
        } else if(cname == "ASWriter") {
            obj = new ASWriter( doc, name );
        } else if(cname == "CppCodeGenerator") {
            if (optionState.generalState.newcodegen)
                obj = new CPPCodeGenerator(doc, name);
            else
                obj = new CppWriter(doc, name);
            //          } else if(cname == "C#Writer") {
            //                  obj = new CsWriter( doc, name );
        } else if(cname == "IDLWriter") {
            obj = new IDLWriter( doc, name );
        } else if(cname =="JavaCodeGenerator") {
            if (optionState.generalState.newcodegen)
                obj = new JavaCodeGenerator(doc, name);
            else
                obj = new JavaWriter(doc, name);
        } else if(cname == "JSWriter") {
            obj = new JSWriter( doc, name );
        } else if (cname == "PHPWriter") {
            obj = new PhpWriter( doc, name);
        } else if (cname == "PHP5Writer") {
            obj = new Php5Writer( doc, name);
        } else if (cname == "PerlWriter") {
            obj = new PerlWriter( doc, name);
        } else if (cname == "PythonWriter") {
            obj = new PythonWriter( doc, name);
        } else if (cname == "RubyWriter") {
            if (optionState.generalState.newcodegen)
                obj = new RubyCodeGenerator(doc, name);
            else
                obj = new RubyWriter(doc, name);
        } else if (cname == "SQLWriter") {
            obj = new SQLWriter( doc, name);
        } else if (cname == "TclWriter") {
            obj = new TclWriter( doc, name);
        } else if (cname == "XMLSchemaWriter") {
            obj = new XMLSchemaWriter( doc, name);
        } else {
            kdWarning() << "cannot create object of type " << name <<
            ". Type unknown" << endl;
        }

    } else {
        kdWarning() << "cannot create parent UML document" << endl;
    }
    return obj;
}
