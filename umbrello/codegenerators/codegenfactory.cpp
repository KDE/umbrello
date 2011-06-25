/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002       Luis De la Parra Blum <luis@delaparra.org>   *
                              Brian Thomas <thomas@mail630.gsfc.nasa.gov>  *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codegenfactory.h"

// app includes
#include "attribute.h"
#include "codegenerator.h"
#include "debug_utils.h"
#include "optionstate.h"
#include "operation.h"
#include "uml.h"
#include "umldoc.h"
#include "umlrole.h"

#include "adawriter.h"
#include "cppwriter.h"
#include "csharpwriter.h"
#include "dwriter.h"
#include "idlwriter.h"
#include "javawriter.h"
#include "mysqlwriter.h"
#include "pascalwriter.h"
#include "perlwriter.h"
#include "phpwriter.h"
#include "php5writer.h"
#include "postgresqlwriter.h"
#include "pythonwriter.h"
#include "rubywriter.h"
#include "sqlwriter.h"
#include "aswriter.h"
#include "jswriter.h"
#include "tclwriter.h"
#include "valawriter.h"
#include "xmlschemawriter.h"

// the new
#include "cppcodegenerator.h"
#include "dcodegenerator.h"
#include "javacodegenerator.h"
#include "rubycodegenerator.h"

#include "cppheadercodedocument.h"
#include "cppsourcecodedocument.h"
#include "dclassifiercodedocument.h"
#include "javaclassifiercodedocument.h"
#include "rubyclassifiercodedocument.h"
#include "javaantcodedocument.h"

#include "cppheadercodeoperation.h"
#include "cppsourcecodeoperation.h"
#include "dcodeoperation.h"
#include "javacodeoperation.h"
#include "rubycodeoperation.h"

#include "cppcodeclassfield.h"
#include "dcodeclassfield.h"
#include "javacodeclassfield.h"
#include "rubycodeclassfield.h"

#include "cppheadercodeaccessormethod.h"
#include "cppsourcecodeaccessormethod.h"
#include "dcodeaccessormethod.h"
#include "javacodeaccessormethod.h"
#include "rubycodeaccessormethod.h"

#include "cppheadercodeclassfielddeclarationblock.h"
#include "cppsourcecodeclassfielddeclarationblock.h"
#include "dcodeclassfielddeclarationblock.h"
#include "javacodeclassfielddeclarationblock.h"
#include "rubycodeclassfielddeclarationblock.h"

#include "cppcodedocumentation.h"
#include "dcodecomment.h"
#include "javacodecomment.h"
#include "rubycodecomment.h"
#include "xmlcodecomment.h"

#include "cppcodegenerationpolicy.h"
#include "dcodegenerationpolicy.h"
#include "javacodegenerationpolicy.h"
#include "rubycodegenerationpolicy.h"

namespace CodeGenFactory
{

CodeGenerator* createObject(Uml::ProgrammingLanguage pl)
{
    CodeGenerator* obj = 0;
    Settings::OptionState optionState = Settings::optionState();
    switch (pl) {
        case Uml::ProgrammingLanguage::Ada:
            obj = new AdaWriter();
            break;
        case Uml::ProgrammingLanguage::ActionScript:
            obj = new ASWriter();
            break;
        case Uml::ProgrammingLanguage::Cpp:
            if (optionState.generalState.newcodegen) {
                obj = new CPPCodeGenerator();
                obj->connect_newcodegen_slots();
            } else {
                obj = new CppWriter();
            }
            break;
        case Uml::ProgrammingLanguage::CSharp:
            obj = new CSharpWriter();
            break;
        case Uml::ProgrammingLanguage::D:
            if (optionState.generalState.newcodegen) {
                obj = new DCodeGenerator();
                obj->connect_newcodegen_slots();
            } else {
                obj = new DWriter();
            }
            break;
        case Uml::ProgrammingLanguage::IDL:
            obj = new IDLWriter();
            break;
        case Uml::ProgrammingLanguage::Java:
            if (optionState.generalState.newcodegen) {
                obj = new JavaCodeGenerator();
                obj->connect_newcodegen_slots();
            } else {
                obj = new JavaWriter();
            }
            break;
        case Uml::ProgrammingLanguage::JavaScript:
            obj = new JSWriter();
            break;
        case Uml::ProgrammingLanguage::MySQL:
            obj = new MySQLWriter();
            break;
        case Uml::ProgrammingLanguage::PHP:
            obj = new PhpWriter();
            break;
        case Uml::ProgrammingLanguage::PHP5:
            obj = new Php5Writer();
            break;
        case Uml::ProgrammingLanguage::Pascal:
            obj = new PascalWriter();
            break;
        case Uml::ProgrammingLanguage::Perl:
            obj = new PerlWriter();
            break;
        case Uml::ProgrammingLanguage::PostgreSQL:
            obj = new PostgreSQLWriter();
            break;
        case Uml::ProgrammingLanguage::Python:
            obj = new PythonWriter();
            break;
        case Uml::ProgrammingLanguage::Ruby:
            if (optionState.generalState.newcodegen) {
                obj = new RubyCodeGenerator();
                obj->connect_newcodegen_slots();
            } else {
                obj = new RubyWriter();
            }
            break;
        case Uml::ProgrammingLanguage::SQL:
            obj = new SQLWriter();
            break;
        case Uml::ProgrammingLanguage::Tcl:
            obj = new TclWriter();
            break;
        case Uml::ProgrammingLanguage::Vala:
            obj = new ValaWriter();
            break;
        case Uml::ProgrammingLanguage::XMLSchema:
            obj = new XMLSchemaWriter();
            break;
        default:
            uWarning() << "cannot create object of type " << pl << ". Type unknown";
            break;
    }

    UMLApp::app()->setPolicyExt(CodeGenFactory::newCodeGenPolicyExt(pl));
    if (obj) {
        obj->initFromParentDocument();
    }
    return obj;
}

CodeDocument * newClassifierCodeDocument(UMLClassifier * c)
{
    Settings::OptionState optionState = Settings::optionState();
    if (!optionState.generalState.newcodegen) {
        return NULL;
    }
    ClassifierCodeDocument *retval = NULL;
    switch (UMLApp::app()->activeLanguage()) {
        case Uml::ProgrammingLanguage::Cpp:
            retval = new CPPSourceCodeDocument(c);
            break;
        case Uml::ProgrammingLanguage::D:
            retval = new DClassifierCodeDocument(c);
            break;
        case Uml::ProgrammingLanguage::Java:
            retval = new JavaClassifierCodeDocument(c);
            break;
        case Uml::ProgrammingLanguage::Ruby:
            retval = new RubyClassifierCodeDocument(c);
            break;
        default:
            break;
    }
    retval->initCodeClassFields();
    retval->synchronize();
    return retval;
}

CodeOperation *newCodeOperation(ClassifierCodeDocument *ccd, UMLOperation * op)
{
    CodeOperation *retval = NULL;
    switch (UMLApp::app()->activeLanguage()) {
        case Uml::ProgrammingLanguage::Cpp:
            {
                CPPHeaderCodeDocument *hcd = dynamic_cast<CPPHeaderCodeDocument*>(ccd);
                if (hcd) {
                    CPPHeaderCodeOperation *cpphcd = new CPPHeaderCodeOperation(hcd, op);
                    cpphcd->updateMethodDeclaration();
                    cpphcd->updateContent();
                    return cpphcd;
                }
                CPPSourceCodeDocument *scd = dynamic_cast<CPPSourceCodeDocument*>(ccd);
                if (scd) {
                    CPPSourceCodeOperation *cppscd = new CPPSourceCodeOperation(scd, op);
                    cppscd->updateMethodDeclaration();
                    cppscd->updateContent();
                    return cppscd;
                }
            }
            break;
        case Uml::ProgrammingLanguage::D:
            retval = new DCodeOperation(dynamic_cast<DClassifierCodeDocument*>(ccd), op);
            retval->updateMethodDeclaration();
            retval->updateContent();
            break;
        case Uml::ProgrammingLanguage::Java:
            retval = new JavaCodeOperation(dynamic_cast<JavaClassifierCodeDocument*>(ccd), op);
            retval->updateMethodDeclaration();
            retval->updateContent();
            break;
        case Uml::ProgrammingLanguage::Ruby:
            retval = new RubyCodeOperation(dynamic_cast<RubyClassifierCodeDocument*>(ccd), op);
            retval->updateMethodDeclaration();
            retval->updateContent();
            break;
        default:
            break;
    }
    return retval;
}

CodeClassField * newCodeClassField(ClassifierCodeDocument *ccd, UMLAttribute *at)
{
    CodeClassField *retval = NULL;
    switch (UMLApp::app()->activeLanguage()) {
        case Uml::ProgrammingLanguage::Cpp:
            retval = new CPPCodeClassField(ccd, at);
            break;
        case Uml::ProgrammingLanguage::D:
            retval = new DCodeClassField(ccd, at);
            break;
        case Uml::ProgrammingLanguage::Java:
            retval = new JavaCodeClassField(ccd, at);
            break;
        case Uml::ProgrammingLanguage::Ruby:
            retval = new RubyCodeClassField(ccd, at);
            break;
        default:
            break;
    }
    retval->finishInitialization();
    return retval;
}

CodeClassField * newCodeClassField(ClassifierCodeDocument *ccd, UMLRole *role)
{
    CodeClassField *retval = NULL;
    switch (UMLApp::app()->activeLanguage()) {
        case Uml::ProgrammingLanguage::Cpp:
            retval = new CPPCodeClassField(ccd, role);
            break;
        case Uml::ProgrammingLanguage::D:
            retval = new DCodeClassField(ccd, role);
            break;
        case Uml::ProgrammingLanguage::Java:
            retval = new JavaCodeClassField(ccd, role);
            break;
        case Uml::ProgrammingLanguage::Ruby:
            retval = new RubyCodeClassField(ccd, role);
            break;
        default:
            break;
    }
    retval->finishInitialization();
    return retval;
}

CodeAccessorMethod * newCodeAccessorMethod(ClassifierCodeDocument *ccd,
                                           CodeClassField *cf,
                                           CodeAccessorMethod::AccessorType type)
{
    CodeAccessorMethod *retval = NULL;
    switch (UMLApp::app()->activeLanguage()) {
        case Uml::ProgrammingLanguage::Cpp:
            {
                CPPHeaderCodeDocument *hcd = dynamic_cast<CPPHeaderCodeDocument*>(ccd);
                if (hcd) {
                    CPPHeaderCodeAccessorMethod *chcam = new CPPHeaderCodeAccessorMethod(cf, type);
                    chcam->update();
                    retval = chcam;
                } else {
                    CPPSourceCodeAccessorMethod *cscam = new CPPSourceCodeAccessorMethod(cf, type);
                    cscam->update();
                    retval = cscam;
                }
            }
            break;
        case Uml::ProgrammingLanguage::D:
            {
                DCodeAccessorMethod *jcam = new DCodeAccessorMethod(cf, type);
                jcam->update();
                retval = jcam;
                retval->setOverallIndentationLevel(1);
            }
            break;
        case Uml::ProgrammingLanguage::Java:
            {
                JavaCodeAccessorMethod *jcam = new JavaCodeAccessorMethod(cf, type);
                jcam->update();
                retval = jcam;
                retval->setOverallIndentationLevel(1);
            }
            break;
        case Uml::ProgrammingLanguage::Ruby:
            {
                RubyCodeAccessorMethod *rcam = new RubyCodeAccessorMethod(cf, type);
                rcam->update();
                retval = rcam;
                retval->setOverallIndentationLevel(1);
            }
            break;
        default:
            break;
    }
    return retval;
}

CodeClassFieldDeclarationBlock * newDeclarationCodeBlock(ClassifierCodeDocument *cd,
                                                         CodeClassField * cf)
{
    CodeClassFieldDeclarationBlock *retval = NULL;
    switch (UMLApp::app()->activeLanguage()) {
        case Uml::ProgrammingLanguage::Cpp:
            {
                CPPHeaderCodeDocument *hcd = dynamic_cast<CPPHeaderCodeDocument*>(cd);
                if (hcd) {
                    CPPHeaderCodeClassFieldDeclarationBlock * cpphcd = new CPPHeaderCodeClassFieldDeclarationBlock(cf);
                    cpphcd->updateContent();
                    return cpphcd;
                }
                CPPSourceCodeDocument *scd = dynamic_cast<CPPSourceCodeDocument*>(cd);
                if (scd) {
                    CPPSourceCodeClassFieldDeclarationBlock *cppscd = new CPPSourceCodeClassFieldDeclarationBlock(cf);
                    cppscd->updateContent();
                    return cppscd;
                }
            }
            break;
        case Uml::ProgrammingLanguage::D:
            retval = new DCodeClassFieldDeclarationBlock(cf);
            retval->updateContent();
            break;
        case Uml::ProgrammingLanguage::Java:
            retval = new JavaCodeClassFieldDeclarationBlock(cf);
            retval->updateContent();
            break;
        case Uml::ProgrammingLanguage::Ruby:
            retval = new RubyCodeClassFieldDeclarationBlock(cf);
            retval->updateContent();
            break;
        default:
            break;
    }
    return retval;
}

CodeComment * newCodeComment(CodeDocument *cd)
{
    switch (UMLApp::app()->activeLanguage()) {
        case Uml::ProgrammingLanguage::Cpp:
            if (dynamic_cast<CPPHeaderCodeDocument*>(cd) ||
                dynamic_cast<CPPSourceCodeDocument*>(cd)) {
                return new CPPCodeDocumentation(cd);
            }
            break;
        case Uml::ProgrammingLanguage::D:
            if (dynamic_cast<DClassifierCodeDocument*>(cd)) {
                return new DCodeComment(cd);
            }
            break;
        case Uml::ProgrammingLanguage::Java:
            if (dynamic_cast<JavaClassifierCodeDocument*>(cd)) {
                return new JavaCodeComment(cd);
            }
            break;
        case Uml::ProgrammingLanguage::Ruby:
            if (dynamic_cast<RubyClassifierCodeDocument*>(cd)) {
                return new RubyCodeComment(cd);
            }
            break;
        default:
            break;
    }
    if (dynamic_cast<JavaANTCodeDocument*>(cd)) {
        return new XMLCodeComment(cd);
    }
    return new CodeComment(cd);
}

CodeGenPolicyExt* newCodeGenPolicyExt(Uml::ProgrammingLanguage pl)
{
    Settings::OptionState optionState = Settings::optionState();

    if ( pl == Uml::ProgrammingLanguage::Cpp ) {
        return new CPPCodeGenerationPolicy();
    }

    if ( optionState.generalState.newcodegen ) {
       switch( pl ) {
          case Uml::ProgrammingLanguage::Java:
              return new JavaCodeGenerationPolicy();
              break;
          case Uml::ProgrammingLanguage::D:
              return new DCodeGenerationPolicy();
              break;
          case Uml::ProgrammingLanguage::Ruby:
              return new RubyCodeGenerationPolicy();
              break;
          default:
              return NULL;
        }
    }

    return NULL;
}

}  // end namespace CodeGenFactory


