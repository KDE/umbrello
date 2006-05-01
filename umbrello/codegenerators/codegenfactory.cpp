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
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                 *
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

namespace CodeGenFactory {

CodeGenerator* createObject(Uml::Programming_Language pl)  {
    CodeGenerator* obj = 0;
    Settings::OptionState optionState = UMLApp::app()->getOptionState();
    switch (pl) {
        case Uml::pl_Ada:
            obj = new AdaWriter();
            break;
        case Uml::pl_ActionScript:
            obj = new ASWriter();
            break;
        case Uml::pl_Cpp:
            if (optionState.generalState.newcodegen)
                obj = new CPPCodeGenerator();
            else
                obj = new CppWriter();
            break;
        // case Uml::pl_Csharp:
        //     obj = new CsWriter();
        //     break;
        case Uml::pl_IDL:
            obj = new IDLWriter();
            break;
        case Uml::pl_Java:
            if (optionState.generalState.newcodegen)
                obj = new JavaCodeGenerator();
            else
                obj = new JavaWriter();
            break;
        case Uml::pl_JavaScript:
            obj = new JSWriter();
            break;
        case Uml::pl_PHP:
            obj = new PhpWriter();
            break;
        case Uml::pl_PHP5:
            obj = new Php5Writer();
            break;
        case Uml::pl_Perl:
            obj = new PerlWriter();
            break;
        case Uml::pl_Python:
            obj = new PythonWriter();
            break;
        case Uml::pl_Ruby:
            if (optionState.generalState.newcodegen)
                obj = new RubyCodeGenerator();
            else
                obj = new RubyWriter();
            break;
        case Uml::pl_SQL:
            obj = new SQLWriter();
            break;
        case Uml::pl_Tcl:
            obj = new TclWriter();
            break;
        case Uml::pl_XMLSchema:
            obj = new XMLSchemaWriter();
            break;
        default:
            kdWarning() << "cannot create object of type " << pl
                        << ". Type unknown" << endl;
            break;
    }
    return obj;
}

}  // end namespace CodeGenFactory

