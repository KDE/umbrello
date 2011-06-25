/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   copyright (C) 2008-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "dcodegenerator.h"

// qt/kde includes
#include <QtCore/QRegExp>

#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>

// local includes
#include "codeviewerdialog.h"
#include "dcodecomment.h"
#include "uml.h"

/**
 * Constructor.
 * @param elem   DOM element
 */
DCodeGenerator::DCodeGenerator(QDomElement & elem)
  : CodeGenerator(elem)
{
    init();
}

/**
 * Constructor.
 */
DCodeGenerator::DCodeGenerator()
{
    init();
}

/**
 * Destructor.
 */
DCodeGenerator::~DCodeGenerator()
{
}

/**
 * Return our language.
 * @return language identifier
 */
Uml::ProgrammingLanguage DCodeGenerator::language() const
{
    return Uml::ProgrammingLanguage::D;
}

/**
 * Get the editing dialog for this code document.
 * In the D version, we make the ANT build file also available.
 */
CodeViewerDialog * DCodeGenerator::getCodeViewerDialog ( QWidget* parent, CodeDocument *doc,
        Settings::CodeViewerState state)
{
    CodeViewerDialog *dialog = new CodeViewerDialog(parent, doc, state);
    return dialog;
}

/**
 * Utility function for getting the d code generation policy.
 * @return generation policy object
 */
DCodeGenerationPolicy * DCodeGenerator::getDPolicy()
{
    return dynamic_cast<DCodeGenerationPolicy*>(UMLApp::app()->policyExt());
}

/**
 * A utility method to get the dCodeGenerationPolicy()->getAutoGenerateAttribAccessors() value.
 * @return value of flag
 */
bool DCodeGenerator::getAutoGenerateAttribAccessors ( )
{
    return getDPolicy()->getAutoGenerateAttribAccessors ();
}

/**
 * A utility method to get the dCodeGenerationPolicy()->getAutoGenerateAssocAccessors() value.
 * @return value of flag
 */
bool DCodeGenerator::getAutoGenerateAssocAccessors ( )
{
    return getDPolicy()->getAutoGenerateAssocAccessors ();
}

/**
 * Get the list variable class name to use. For D, we have set this to "Vector".
 * @return name of list field class
 */
QString DCodeGenerator::getListFieldClassName ()
{
    return QString("Vector");
}

/**
 * General purpose function we may reuse for all types of D code documents.
 * @param item   the item to change
 * @return the changed item
 */
QString DCodeGenerator::capitalizeFirstLetter(const QString &item)
{
    // we could lowercase everything tostart and then capitalize? Nah, it would
    // screw up formatting like getMyRadicalVariable() to getMyradicalvariable(). Bah.
    QChar firstChar = item.at(0);
    return firstChar.toUpper() + item.mid(1);
}

/**
 * IF the type is "string" we need to declare it as
 * the D Object "String" (there is no string primative in D).
 * Same thing again for "bool" to "boolean".
 * @param item   the item to change
 * @return the changed item
 */
QString DCodeGenerator::fixTypeName(const QString &item)
{
    if (item.isEmpty() || item.contains(QRegExp("^\\s+$")))
        return "void";
    if (item == "string")
        return "char[]";
    return cleanName(item);
}

/**
 * Create a new classifier code document.
 * @param classifier   the UML classifier
 * @return the created classifier code document object
 */
CodeDocument * DCodeGenerator::newClassifierCodeDocument ( UMLClassifier * classifier)
{
    DClassifierCodeDocument * doc = new DClassifierCodeDocument(classifier);
    doc->initCodeClassFields();
    return doc;
}

/**
 * Initialization of class.
 */
void DCodeGenerator::init()
{
    // load Classifier documents from parent document
    //initFromParentDocument();
}

/**
 * Adds D's primitives as datatypes.
 * @return the list of primitive datatypes
 */
QStringList DCodeGenerator::defaultDatatypes()
{
    QStringList l;
    l << "void"
    << "bool"
    << "byte"
    << "ubyte"
    << "short"
    << "ushort"
    << "int"
    << "uint"
    << "long"
    << "ulong"
    << "cent"
    << "ucent"
    << "float"
    << "double"
    << "real"
    << "ifloat"
    << "idouble"
    << "ireal"
    << "cfloat"
    << "cdouble"
    << "creal"
    << "char"
    << "wchar"
    << "dchar";
    return l;
}

/**
 * Get list of reserved keywords.
 * @return the list of reserved keywords
 */
QStringList DCodeGenerator::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords << "abstract"
        << "alias"
        << "align"
        << "asm"
        << "assert"
        << "auto"
        << "body"
        << "bool"
        << "break"
        << "byte"
        << "case"
        << "cast"
        << "catch"
        << "cdouble"
        << "cent"
        << "cfloat"
        << "char"
        << "class"
        << "const"
        << "continue"
        << "creal"
        << "dchar"
        << "debug"
        << "default"
        << "delegate"
        << "delete"
        << "deprecated"
        << "do"
        << "double"
        << "else"
        << "enum"
        << "export"
        << "extern"
        << "false"
        << "final"
        << "finally"
        << "float"
        << "for"
        << "foreach"
        << "foreach_reverse"
        << "function"
        << "goto"
        << "idouble"
        << "if"
        << "ifloat"
        << "import"
        << "in"
        << "inout"
        << "int"
        << "interface"
        << "invariant"
        << "ireal"
        << "is"
        << "lazy"
        << "long"
        << "macro"
        << "mixin"
        << "module"
        << "msg"
        << "new"
        << "null"
        << "out"
        << "override"
        << "package"
        << "pragma"
        << "private"
        << "protected"
        << "public"
        << "real"
        << "ref"
        << "return"
        << "scope"
        << "short"
        << "static"
        << "struct"
        << "super"
        << "switch"
        << "synchronized"
        << "template"
        << "this"
        << "throw"
        << "true"
        << "try"
        << "typedef"
        << "typeid"
        << "typeof"
        << "ubyte"
        << "ucent"
        << "uint"
        << "ulong"
        << "union"
        << "unittest"
        << "ushort"
        << "version"
        << "void"
        << "volatile"
        << "wchar"
        << "while"
        << "with";
    }

    return keywords;
}

#include "dcodegenerator.moc"
