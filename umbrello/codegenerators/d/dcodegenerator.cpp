/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "dcodegenerator.h"

// local includes
#include "codeviewerdialog.h"
#include "dcodecomment.h"
#include "uml.h"

// kde includes
#include <kconfig.h>
#include <KLocalizedString>

// qt includes
#include <QRegularExpression>

/**
 * Constructor.
 */
DCodeGenerator::DCodeGenerator()
  : AdvancedCodeGenerator()
{
    // load Classifier documents from parent document
    //initFromParentDocument();
    connectSlots();
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
Uml::ProgrammingLanguage::Enum DCodeGenerator::language() const
{
    return Uml::ProgrammingLanguage::D;
}

/**
 * Get the editing dialog for this code document.
 * In the D version, we make the ANT build file also available.
 */
CodeViewerDialog * DCodeGenerator::getCodeViewerDialog (QWidget* parent, CodeDocument *doc,
        Settings::CodeViewerState & state)
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
bool DCodeGenerator::getAutoGenerateAttribAccessors ()
{
    return getDPolicy()->getAutoGenerateAttribAccessors ();
}

/**
 * A utility method to get the dCodeGenerationPolicy()->getAutoGenerateAssocAccessors() value.
 * @return value of flag
 */
bool DCodeGenerator::getAutoGenerateAssocAccessors ()
{
    return getDPolicy()->getAutoGenerateAssocAccessors ();
}

/**
 * Get the list variable class name to use. For D, we have set this to "Vector".
 * @return name of list field class
 */
QString DCodeGenerator::getListFieldClassName ()
{
    return QString(QStringLiteral("Vector"));
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
 * the D Object "String" (there is no string primitive in D).
 * Same thing again for "bool" to "boolean".
 * @param item   the item to change
 * @return the changed item
 */
QString DCodeGenerator::fixTypeName(const QString &item)
{
    if (item.isEmpty() || item.contains(QRegularExpression(QStringLiteral("^\\s+$"))))
        return QStringLiteral("void");
    if (item == QStringLiteral("string"))
        return QStringLiteral("char[]");
    return cleanName(item);
}

/**
 * Create a new classifier code document.
 * @param classifier   the UML classifier
 * @return the created classifier code document object
 */
CodeDocument * DCodeGenerator::newClassifierCodeDocument (UMLClassifier * classifier)
{
    DClassifierCodeDocument * doc = new DClassifierCodeDocument(classifier);
    doc->initCodeClassFields();
    return doc;
}

/**
 * Adds D's primitives as datatypes.
 * @return the list of primitive datatypes
 */
QStringList DCodeGenerator::defaultDatatypes() const
{
    QStringList l;
    l << QStringLiteral("void")
      << QStringLiteral("bool")
      << QStringLiteral("byte")
      << QStringLiteral("ubyte")
      << QStringLiteral("short")
      << QStringLiteral("ushort")
      << QStringLiteral("int")
      << QStringLiteral("uint")
      << QStringLiteral("long")
      << QStringLiteral("ulong")
      << QStringLiteral("cent")
      << QStringLiteral("ucent")
      << QStringLiteral("float")
      << QStringLiteral("double")
      << QStringLiteral("real")
      << QStringLiteral("ifloat")
      << QStringLiteral("idouble")
      << QStringLiteral("ireal")
      << QStringLiteral("cfloat")
      << QStringLiteral("cdouble")
      << QStringLiteral("creal")
      << QStringLiteral("char")
      << QStringLiteral("wchar")
      << QStringLiteral("dchar")
      << QStringLiteral("string");
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
        keywords
          << QStringLiteral("abstract")
          << QStringLiteral("alias")
          << QStringLiteral("align")
          << QStringLiteral("asm")
          << QStringLiteral("assert")
          << QStringLiteral("auto")
          << QStringLiteral("body")
          << QStringLiteral("bool")
          << QStringLiteral("break")
          << QStringLiteral("byte")
          << QStringLiteral("case")
          << QStringLiteral("cast")
          << QStringLiteral("catch")
          << QStringLiteral("cdouble")
          << QStringLiteral("cent")
          << QStringLiteral("cfloat")
          << QStringLiteral("char")
          << QStringLiteral("class")
          << QStringLiteral("const")
          << QStringLiteral("continue")
          << QStringLiteral("creal")
          << QStringLiteral("dchar")
          << QStringLiteral("debug")
          << QStringLiteral("default")
          << QStringLiteral("delegate")
          << QStringLiteral("delete")
          << QStringLiteral("deprecated")
          << QStringLiteral("do")
          << QStringLiteral("double")
          << QStringLiteral("else")
          << QStringLiteral("enum")
          << QStringLiteral("export")
          << QStringLiteral("extern")
          << QStringLiteral("false")
          << QStringLiteral("final")
          << QStringLiteral("finally")
          << QStringLiteral("float")
          << QStringLiteral("for")
          << QStringLiteral("foreach")
          << QStringLiteral("foreach_reverse")
          << QStringLiteral("function")
          << QStringLiteral("goto")
          << QStringLiteral("idouble")
          << QStringLiteral("if")
          << QStringLiteral("ifloat")
          << QStringLiteral("import")
          << QStringLiteral("in")
          << QStringLiteral("inout")
          << QStringLiteral("int")
          << QStringLiteral("interface")
          << QStringLiteral("invariant")
          << QStringLiteral("ireal")
          << QStringLiteral("is")
          << QStringLiteral("lazy")
          << QStringLiteral("long")
          << QStringLiteral("macro")
          << QStringLiteral("mixin")
          << QStringLiteral("module")
          << QStringLiteral("msg")
          << QStringLiteral("new")
          << QStringLiteral("null")
          << QStringLiteral("out")
          << QStringLiteral("override")
          << QStringLiteral("package")
          << QStringLiteral("pragma")
          << QStringLiteral("private")
          << QStringLiteral("protected")
          << QStringLiteral("public")
          << QStringLiteral("real")
          << QStringLiteral("ref")
          << QStringLiteral("return")
          << QStringLiteral("scope")
          << QStringLiteral("short")
          << QStringLiteral("static")
          << QStringLiteral("struct")
          << QStringLiteral("super")
          << QStringLiteral("switch")
          << QStringLiteral("synchronized")
          << QStringLiteral("template")
          << QStringLiteral("this")
          << QStringLiteral("throw")
          << QStringLiteral("true")
          << QStringLiteral("try")
          << QStringLiteral("typedef")
          << QStringLiteral("typeid")
          << QStringLiteral("typeof")
          << QStringLiteral("ubyte")
          << QStringLiteral("ucent")
          << QStringLiteral("uint")
          << QStringLiteral("ulong")
          << QStringLiteral("union")
          << QStringLiteral("unittest")
          << QStringLiteral("ushort")
          << QStringLiteral("version")
          << QStringLiteral("void")
          << QStringLiteral("volatile")
          << QStringLiteral("wchar")
          << QStringLiteral("while")
          << QStringLiteral("with");
    }

    return keywords;
}

