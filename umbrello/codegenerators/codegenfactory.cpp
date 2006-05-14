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

// own header
#include "codegenfactory.h"

// qt/kde includes
#include <qstringlist.h>
#include <kdebug.h>

// app includes
#include "../codegenerator.h"
#include "../umldoc.h"
#include "../uml.h"
#include "../optionstate.h"
#include "../operation.h"
#include "../attribute.h"
#include "../umlrole.h"

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
#include "xmlschemawriter.h"

// the new
#include "cppcodegenerator.h"
#include "javacodegenerator.h"
#include "rubycodegenerator.h"

#include "cppheadercodedocument.h"
#include "cppsourcecodedocument.h"
#include "javaclassifiercodedocument.h"
#include "rubyclassifiercodedocument.h"
#include "javaantcodedocument.h"

#include "cppheadercodeoperation.h"
#include "cppsourcecodeoperation.h"
#include "javacodeoperation.h"
#include "rubycodeoperation.h"

#include "cppcodeclassfield.h"
#include "javacodeclassfield.h"
#include "rubycodeclassfield.h"

#include "cppheadercodeaccessormethod.h"
#include "cppsourcecodeaccessormethod.h"
#include "javacodeaccessormethod.h"
#include "rubycodeaccessormethod.h"

#include "cppheadercodeclassfielddeclarationblock.h"
#include "cppsourcecodeclassfielddeclarationblock.h"
#include "javacodeclassfielddeclarationblock.h"
#include "rubycodeclassfielddeclarationblock.h"

#include "cppcodedocumentation.h"
#include "javacodecomment.h"
#include "rubycodecomment.h"
#include "xmlcodecomment.h"

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
    obj->initFromParentDocument();
    return obj;
}

CodeOperation *newCodeOperation(ClassifierCodeDocument *ccd, UMLOperation * op) {
    CodeOperation *retval = NULL;
    switch (UMLApp::app()->getActiveLanguage()) {
        case Uml::pl_Cpp:
            {
                CPPHeaderCodeDocument *hcd = dynamic_cast<CPPHeaderCodeDocument*>(ccd);
                if (hcd)
                    retval = new CPPHeaderCodeOperation(hcd, op);
                else
                    retval = new CPPSourceCodeOperation(dynamic_cast<CPPSourceCodeDocument*>(ccd), op);
            }
            break;
        case Uml::pl_Java:
            retval = new JavaCodeOperation(dynamic_cast<JavaClassifierCodeDocument*>(ccd), op);
            break;
        case Uml::pl_Ruby:
            retval = new RubyCodeOperation(dynamic_cast<RubyClassifierCodeDocument*>(ccd), op);
            break;
        default:
            break;
    }
    return retval;
}


CodeClassField * newCodeClassField(ClassifierCodeDocument *ccd, UMLAttribute *at) {
    CodeClassField *retval = NULL;
    switch (UMLApp::app()->getActiveLanguage()) {
        case Uml::pl_Cpp:
            retval = new CPPCodeClassField(ccd, at);
            break;
        case Uml::pl_Java:
            retval = new JavaCodeClassField(dynamic_cast<JavaClassifierCodeDocument*>(ccd), at);
            break;
        case Uml::pl_Ruby:
            retval = new RubyCodeClassField(dynamic_cast<RubyClassifierCodeDocument*>(ccd), at);
            break;
        default:
            break;
    }
    return retval;
}

CodeClassField * newCodeClassField(ClassifierCodeDocument *ccd, UMLRole *role) {
    CodeClassField *retval = NULL;
    switch (UMLApp::app()->getActiveLanguage()) {
        case Uml::pl_Cpp:
            retval = new CPPCodeClassField(ccd, role);
            break;
        case Uml::pl_Java:
            retval = new JavaCodeClassField(dynamic_cast<JavaClassifierCodeDocument*>(ccd), role);
            break;
        case Uml::pl_Ruby:
            retval = new RubyCodeClassField(dynamic_cast<RubyClassifierCodeDocument*>(ccd), role);
            break;
        default:
            break;
    }
    return retval;
}

CodeAccessorMethod * newCodeAccessorMethod(ClassifierCodeDocument *ccd,
                                           CodeClassField *cf,
                                           CodeAccessorMethod::AccessorType type) {
    CodeAccessorMethod *retval = NULL;
    switch (UMLApp::app()->getActiveLanguage()) {
        case Uml::pl_Cpp:
            {
                //CPPCodeClassField *ccf = dynamic_cast<CPPCodeClassField*>(cf);
                CPPCodeClassField *ccf = (CPPCodeClassField*)(cf);
                CPPHeaderCodeDocument *hcd = dynamic_cast<CPPHeaderCodeDocument*>(ccd);
                if (hcd) {
                    retval = new CPPHeaderCodeAccessorMethod(ccf, type);
                } else {
                    retval = new CPPSourceCodeAccessorMethod(ccf, type);
                }
            }
            break;
        case Uml::pl_Java:
            retval = new JavaCodeAccessorMethod(dynamic_cast<JavaCodeClassField*>(cf), type);
            retval->setOverallIndentationLevel(1);
            break;
        case Uml::pl_Ruby:
            retval = new RubyCodeAccessorMethod(dynamic_cast<RubyCodeClassField*>(cf), type);
            retval->setOverallIndentationLevel(1);
            break;
        default:
            break;
    }
    return retval;
}

CodeClassFieldDeclarationBlock * newDeclarationCodeBlock (ClassifierCodeDocument *cd,
                                                          CodeClassField * cf) {
    CodeClassFieldDeclarationBlock *retval = NULL;
    switch (UMLApp::app()->getActiveLanguage()) {
        case Uml::pl_Cpp:
            {
                CPPHeaderCodeDocument *hcd = dynamic_cast<CPPHeaderCodeDocument*>(cd);
                if (hcd)
                    return new CPPHeaderCodeClassFieldDeclarationBlock(cf);
                CPPSourceCodeDocument *scd = dynamic_cast<CPPSourceCodeDocument*>(cd);
                if (scd)
                    return new CPPSourceCodeClassFieldDeclarationBlock(cf);
            }
            break;
        case Uml::pl_Java:
            retval = new JavaCodeClassFieldDeclarationBlock(cf);
            break;
        case Uml::pl_Ruby:
            retval = new RubyCodeClassFieldDeclarationBlock(cf);
            break;
        default:
            break;
    }
    return retval;
}

CodeComment * newCodeComment (CodeDocument *cd) {
    switch (UMLApp::app()->getActiveLanguage()) {
        case Uml::pl_Cpp:
            if (dynamic_cast<CPPHeaderCodeDocument*>(cd) ||
                dynamic_cast<CPPSourceCodeDocument*>(cd))
                return new CPPCodeDocumentation(cd);
            break;
        case Uml::pl_Java:
            if (dynamic_cast<JavaClassifierCodeDocument*>(cd))
                return new JavaCodeComment(cd);
            break;
        case Uml::pl_Ruby:
            if (dynamic_cast<RubyClassifierCodeDocument*>(cd))
                return new RubyCodeComment(cd);
            break;
        default:
            break;
    }
    if (dynamic_cast<JavaANTCodeDocument*>(cd))
        return new XMLCodeComment(cd);
    return new CodeComment(cd);
}

}  // end namespace CodeGenFactory

