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

CodeGenerator* CodeGeneratorFactory::createObject(UMLDoc* doc, const char* name)  {
    CodeGenerator* obj = 0;
    QString cname(name);

    if (doc) {
        Settings::OptionState optionState = UMLApp::app()->getOptionState();
        if (cname == "Ada") {
            obj = new AdaWriter(doc, name);
        } else if (cname == "ActionScript") {
            obj = new ASWriter( doc, name );
        } else if (cname == "C++") {
            if (optionState.generalState.newcodegen)
                obj = new CPPCodeGenerator(doc, name);
            else
                obj = new CppWriter(doc, name);
        // } else if (cname == "C#") {
        //     obj = new CsWriter( doc, name );
        } else if (cname == "IDL") {
            obj = new IDLWriter( doc, name );
        } else if (cname =="Java") {
            if (optionState.generalState.newcodegen)
                obj = new JavaCodeGenerator(doc, name);
            else
                obj = new JavaWriter(doc, name);
        } else if (cname == "JavaScript") {
            obj = new JSWriter( doc, name );
        } else if (cname == "PHP") {
            obj = new PhpWriter( doc, name);
        } else if (cname == "PHP5") {
            obj = new Php5Writer( doc, name);
        } else if (cname == "Perl") {
            obj = new PerlWriter( doc, name);
        } else if (cname == "Python") {
            obj = new PythonWriter( doc, name);
        } else if (cname == "Ruby") {
            if (optionState.generalState.newcodegen)
                obj = new RubyCodeGenerator(doc, name);
            else
                obj = new RubyWriter(doc, name);
        } else if (cname == "SQL") {
            obj = new SQLWriter( doc, name);
        } else if (cname == "Tcl") {
            obj = new TclWriter( doc, name);
        } else if (cname == "XMLSchema") {
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
