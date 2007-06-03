
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "dcodegenerator.h"

// qt/kde includes
#include <qregexp.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

// local includes
#include "dcodecomment.h"
#include "codeviewerdialog.h"
#include "../uml.h"

// Constructors/Destructors
//

DCodeGenerator::DCodeGenerator (QDomElement & elem)
  : CodeGenerator(elem)
{
    init();
}

DCodeGenerator::DCodeGenerator ()
{
    init();
}

DCodeGenerator::~DCodeGenerator ( ) { }

//
// Methods
//

// Accessor methods
//

// return our language
Uml::Programming_Language DCodeGenerator::getLanguage() {
    return Uml::pl_D;
}

// In the D version, we make the ANT build file also available.
CodeViewerDialog * DCodeGenerator::getCodeViewerDialog ( QWidget* parent, CodeDocument *doc,
        Settings::CodeViewerState state)
{
    CodeViewerDialog *dialog = new CodeViewerDialog(parent, doc, state);
    return dialog;
}


DCodeGenerationPolicy * DCodeGenerator::getDPolicy() {
    return dynamic_cast<DCodeGenerationPolicy*>(UMLApp::app()->getPolicyExt());
}

bool DCodeGenerator::getAutoGenerateAttribAccessors ( )
{
    return getDPolicy()->getAutoGenerateAttribAccessors ();
}

bool DCodeGenerator::getAutoGenerateAssocAccessors ( )
{
    return getDPolicy()->getAutoGenerateAssocAccessors ();
}

QString DCodeGenerator::getListFieldClassName () {
    return QString("Vector");
}

// Other methods
//

QString DCodeGenerator::capitalizeFirstLetter(const QString &string)
{
    // we could lowercase everything tostart and then capitalize? Nah, it would
    // screw up formatting like getMyRadicalVariable() to getMyradicalvariable(). Bah.
    QChar firstChar = string.at(0);
    return firstChar.upper() + string.mid(1);
}

// IF the type is "string" we need to declare it as
// the D Object "String" (there is no string primative in D).
// Same thing again for "bool" to "boolean"
QString DCodeGenerator::fixTypeName(const QString &string)
{
    if (string.isEmpty() || string.contains(QRegExp("^\\s+$")))
        return "void";
    if (string == "string")
        return "char[]";
    return cleanName(string);
}

/**
 * @return      ClassifierCodeDocument
 * @param       classifier
 */
CodeDocument * DCodeGenerator::newClassifierCodeDocument ( UMLClassifier * c)
{
    DClassifierCodeDocument * doc = new DClassifierCodeDocument(c);
    doc->initCodeClassFields();
    return doc;
}

void DCodeGenerator::init() {
    // load Classifier documents from parent document
    //initFromParentDocument();
}

QStringList DCodeGenerator::defaultDatatypes() {
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

const QStringList DCodeGenerator::reservedKeywords() const {

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
