/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005                                                    *
 *   Richard Dale  <Richard_Dale@tipitina.demon.co.uk>                     *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "rubycodegenerator.h"

// local includes
#include "rubycodecomment.h"
#include "codeviewerdialog.h"
#include "uml.h"

// kde includes
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>

// qt includes
#include <QtCore/QRegExp>

/**
 * Constructor.
 * @param elem DOM element
 */
RubyCodeGenerator::RubyCodeGenerator(QDomElement & elem)
  : CodeGenerator(elem)
{
}

/**
 * Constructor.
 */
RubyCodeGenerator::RubyCodeGenerator()
{
}

/**
 * Destructor.
 */
RubyCodeGenerator::~RubyCodeGenerator()
{
}

/**
 * Return our language.
 * @return language identifier
 */
Uml::ProgrammingLanguage RubyCodeGenerator::language() const
{
    return Uml::ProgrammingLanguage::Ruby;
}

/**
 * Get the editing dialog for this code document.
 * @return code viewer dialog object
 */
CodeViewerDialog * RubyCodeGenerator::getCodeViewerDialog(QWidget* parent, CodeDocument *doc,
        Settings::CodeViewerState state)
{
    CodeViewerDialog *dialog = new CodeViewerDialog(parent, doc, state);
    return dialog;
}

/**
 * Utility function for getting the ruby code generation policy.
 * @return Ruby code generation policy object
 */
RubyCodeGenerationPolicy * RubyCodeGenerator::getRubyPolicy()
{
    return dynamic_cast<RubyCodeGenerationPolicy*>(UMLApp::app()->policyExt());
}

/**
 * A utility method to get the rubyCodeGenerationPolicy()->getAutoGenerateAttribAccessors() value.
 * @return flag
 */
bool RubyCodeGenerator::getAutoGenerateAttribAccessors()
{
    return getRubyPolicy()->getAutoGenerateAttribAccessors();
}

/**
 * A utility method to get the rubyCodeGenerationPolicy()->getAutoGenerateAssocAccessors() value.
 * @return flag
 */
bool RubyCodeGenerator::getAutoGenerateAssocAccessors()
{
    return getRubyPolicy()->getAutoGenerateAssocAccessors();
}

/**
 * Get the list variable class name to use. For Ruby, we have set this to "Array".
 * @return name of list field class
 */
QString RubyCodeGenerator::getListFieldClassName()
{
    return QString("Array");
}

/**
 * Convert a C++ type such as 'int' or 'QWidget' to
 * ruby types Integer and Qt::Widget.
 * @param cppType the C++ type to be converted
 * @return the ruby type as string
 */
QString RubyCodeGenerator::cppToRubyType(const QString &cppType)
{
    QString type = cleanName(cppType);
    type.remove("const ");
    type.remove(QRegExp("[*&\\s]"));
    type.replace(QRegExp("[<>]"), "_");
    type.replace("QStringList", "Array");
    type.replace(QRegExp("^string$"),"String");
    type.replace("QString", "String");
    type.replace("bool", "true|false");
    type.replace(QRegExp("^(uint|int|ushort|short|ulong|long)$"), "Integer");
    type.replace(QRegExp("^(float|double)$"), "Float");
    type.replace(QRegExp("^Q(?=[A-Z])"), "Qt::");
    type.replace(QRegExp("^K(?!(DE|Parts|IO)"), "KDE::");

    return type;
}

/**
 * Convert C++ names such as 'm_foobar' or pFoobar to
 * just 'foobar' for ruby.
 * @param cppName the C++ name to be converted
 * @return the ruby name as string
 */
QString RubyCodeGenerator::cppToRubyName(const QString &cppName)
{
    QString name = cleanName(cppName);
    name.remove(QRegExp("^m_"));
    name.remove(QRegExp("^[pbn](?=[A-Z])"));
    name = name.mid(0, 1).toLower() + name.mid(1);
    return name;
}

/**
 * Create a new classifier code document.
 * @param classifier the UML classifier
 * @return a new classifier code document
 */
CodeDocument * RubyCodeGenerator::newClassifierCodeDocument(UMLClassifier * classifier)
{
    RubyClassifierCodeDocument * doc = new RubyClassifierCodeDocument(classifier);
    doc->initCodeClassFields();
    return doc;
}

/* These initializations are done in CodeGenFactory::createObject()
void RubyCodeGenerator::initFields()
{
    UMLApp::app()->setPolicyExt ( new RubyCodeGenerationPolicy(UMLApp::app()->getConfig()) );
    // load Classifier documents from parent document
    initFromParentDocument();
}
*/

/**
 * Get list of reserved keywords.
 * @return the list of reserved keywords
 */
QStringList RubyCodeGenerator::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords << "__FILE__"
        << "__LINE__"
        << "BEGIN"
        << "END"
        << "alias"
        << "and"
        << "begin"
        << "break"
        << "case"
        << "class"
        << "def"
        << "defined?"
        << "do"
        << "else"
        << "elsif"
        << "end"
        << "ensure"
        << "false"
        << "for"
        << "if"
        << "in"
        << "module"
        << "next"
        << "nil"
        << "not"
        << "or"
        << "redo"
        << "rescue"
        << "retry"
        << "return"
        << "self"
        << "super"
        << "then"
        << "true"
        << "undef"
        << "unless"
        << "until"
        << "when"
        << "while"
        << "yield";
    }

    return keywords;
}

#include "rubycodegenerator.moc"
