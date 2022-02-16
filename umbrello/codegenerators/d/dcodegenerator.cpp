/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
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
#include <KMessageBox>

// qt includes
#include <QRegExp>

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
    return QString(QLatin1String("Vector"));
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
    if (item.isEmpty() || item.contains(QRegExp(QLatin1String("^\\s+$"))))
        return QLatin1String("void");
    if (item == QLatin1String("string"))
        return QLatin1String("char[]");
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
    l << QLatin1String("void")
      << QLatin1String("bool")
      << QLatin1String("byte")
      << QLatin1String("ubyte")
      << QLatin1String("short")
      << QLatin1String("ushort")
      << QLatin1String("int")
      << QLatin1String("uint")
      << QLatin1String("long")
      << QLatin1String("ulong")
      << QLatin1String("cent")
      << QLatin1String("ucent")
      << QLatin1String("float")
      << QLatin1String("double")
      << QLatin1String("real")
      << QLatin1String("ifloat")
      << QLatin1String("idouble")
      << QLatin1String("ireal")
      << QLatin1String("cfloat")
      << QLatin1String("cdouble")
      << QLatin1String("creal")
      << QLatin1String("char")
      << QLatin1String("wchar")
      << QLatin1String("dchar")
      << QLatin1String("string");
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
          << QLatin1String("abstract")
          << QLatin1String("alias")
          << QLatin1String("align")
          << QLatin1String("asm")
          << QLatin1String("assert")
          << QLatin1String("auto")
          << QLatin1String("body")
          << QLatin1String("bool")
          << QLatin1String("break")
          << QLatin1String("byte")
          << QLatin1String("case")
          << QLatin1String("cast")
          << QLatin1String("catch")
          << QLatin1String("cdouble")
          << QLatin1String("cent")
          << QLatin1String("cfloat")
          << QLatin1String("char")
          << QLatin1String("class")
          << QLatin1String("const")
          << QLatin1String("continue")
          << QLatin1String("creal")
          << QLatin1String("dchar")
          << QLatin1String("debug")
          << QLatin1String("default")
          << QLatin1String("delegate")
          << QLatin1String("delete")
          << QLatin1String("deprecated")
          << QLatin1String("do")
          << QLatin1String("double")
          << QLatin1String("else")
          << QLatin1String("enum")
          << QLatin1String("export")
          << QLatin1String("extern")
          << QLatin1String("false")
          << QLatin1String("final")
          << QLatin1String("finally")
          << QLatin1String("float")
          << QLatin1String("for")
          << QLatin1String("foreach")
          << QLatin1String("foreach_reverse")
          << QLatin1String("function")
          << QLatin1String("goto")
          << QLatin1String("idouble")
          << QLatin1String("if")
          << QLatin1String("ifloat")
          << QLatin1String("import")
          << QLatin1String("in")
          << QLatin1String("inout")
          << QLatin1String("int")
          << QLatin1String("interface")
          << QLatin1String("invariant")
          << QLatin1String("ireal")
          << QLatin1String("is")
          << QLatin1String("lazy")
          << QLatin1String("long")
          << QLatin1String("macro")
          << QLatin1String("mixin")
          << QLatin1String("module")
          << QLatin1String("msg")
          << QLatin1String("new")
          << QLatin1String("null")
          << QLatin1String("out")
          << QLatin1String("override")
          << QLatin1String("package")
          << QLatin1String("pragma")
          << QLatin1String("private")
          << QLatin1String("protected")
          << QLatin1String("public")
          << QLatin1String("real")
          << QLatin1String("ref")
          << QLatin1String("return")
          << QLatin1String("scope")
          << QLatin1String("short")
          << QLatin1String("static")
          << QLatin1String("struct")
          << QLatin1String("super")
          << QLatin1String("switch")
          << QLatin1String("synchronized")
          << QLatin1String("template")
          << QLatin1String("this")
          << QLatin1String("throw")
          << QLatin1String("true")
          << QLatin1String("try")
          << QLatin1String("typedef")
          << QLatin1String("typeid")
          << QLatin1String("typeof")
          << QLatin1String("ubyte")
          << QLatin1String("ucent")
          << QLatin1String("uint")
          << QLatin1String("ulong")
          << QLatin1String("union")
          << QLatin1String("unittest")
          << QLatin1String("ushort")
          << QLatin1String("version")
          << QLatin1String("void")
          << QLatin1String("volatile")
          << QLatin1String("wchar")
          << QLatin1String("while")
          << QLatin1String("with");
    }

    return keywords;
}

