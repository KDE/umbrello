/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "rubycodegenerator.h"

// local includes
#include "rubycodecomment.h"
#include "codeviewerdialog.h"
#include "uml.h"

// kde includes
#include <kconfig.h>
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QRegularExpression>

/**
 * Constructor.
 */
RubyCodeGenerator::RubyCodeGenerator()
   : AdvancedCodeGenerator()
{
    // These initializations are done in CodeGenFactory::createObject()
        //UMLApp::app()->setPolicyExt (new RubyCodeGenerationPolicy(UMLApp::app()->getConfig()));
        // load Classifier documents from parent document
        //initFromParentDocument();
    connectSlots();
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
Uml::ProgrammingLanguage::Enum RubyCodeGenerator::language() const
{
    return Uml::ProgrammingLanguage::Ruby;
}

/**
 * Get the editing dialog for this code document.
 * @return code viewer dialog object
 */
CodeViewerDialog * RubyCodeGenerator::getCodeViewerDialog(QWidget* parent, CodeDocument *doc,
        Settings::CodeViewerState & state)
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
    return QString(QStringLiteral("Array"));
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
    type.remove(QStringLiteral("const "));
    type.remove(QRegularExpression(QStringLiteral("[*&\\s]")));
    type.replace(QRegularExpression(QStringLiteral("[<>]")), QStringLiteral("_"));
    type.replace(QStringLiteral("QStringList"), QStringLiteral("Array"));
    type.replace(QRegularExpression(QStringLiteral("^string$")),QStringLiteral("String"));
    type.replace(QStringLiteral("QString"), QStringLiteral("String"));
    type.replace(QStringLiteral("bool"), QStringLiteral("true|false"));
    type.replace(QRegularExpression(QStringLiteral("^(uint|int|ushort|short|ulong|long)$")), QStringLiteral("Integer"));
    type.replace(QRegularExpression(QStringLiteral("^(float|double)$")), QStringLiteral("Float"));
    type.replace(QRegularExpression(QStringLiteral("^Q(?=[A-Z])")), QStringLiteral("Qt::"));
    type.replace(QRegularExpression(QStringLiteral("^K(?!(DE|Parts|IO)")), QStringLiteral("KDE::"));

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
    name.remove(QRegularExpression(QStringLiteral("^m_")));
    name.remove(QRegularExpression(QStringLiteral("^[pbn](?=[A-Z])")));
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

/**
 * Get list of reserved keywords.
 * @return the list of reserved keywords
 */
QStringList RubyCodeGenerator::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords
          << QStringLiteral("__FILE__")
          << QStringLiteral("__LINE__")
          << QStringLiteral("BEGIN")
          << QStringLiteral("END")
          << QStringLiteral("alias")
          << QStringLiteral("and")
          << QStringLiteral("begin")
          << QStringLiteral("break")
          << QStringLiteral("case")
          << QStringLiteral("class")
          << QStringLiteral("def")
          << QStringLiteral("defined?")
          << QStringLiteral("do")
          << QStringLiteral("else")
          << QStringLiteral("elsif")
          << QStringLiteral("end")
          << QStringLiteral("ensure")
          << QStringLiteral("false")
          << QStringLiteral("for")
          << QStringLiteral("if")
          << QStringLiteral("in")
          << QStringLiteral("module")
          << QStringLiteral("next")
          << QStringLiteral("nil")
          << QStringLiteral("not")
          << QStringLiteral("or")
          << QStringLiteral("redo")
          << QStringLiteral("rescue")
          << QStringLiteral("retry")
          << QStringLiteral("return")
          << QStringLiteral("self")
          << QStringLiteral("super")
          << QStringLiteral("then")
          << QStringLiteral("true")
          << QStringLiteral("undef")
          << QStringLiteral("unless")
          << QStringLiteral("until")
          << QStringLiteral("when")
          << QStringLiteral("while")
          << QStringLiteral("yield");
    }

    return keywords;
}

