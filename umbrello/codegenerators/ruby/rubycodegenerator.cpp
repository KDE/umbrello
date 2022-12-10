/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
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
#include <QRegExp>

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
    return QString(QLatin1String("Array"));
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
    type.remove(QLatin1String("const "));
    type.remove(QRegExp(QLatin1String("[*&\\s]")));
    type.replace(QRegExp(QLatin1String("[<>]")), QLatin1String("_"));
    type.replace(QLatin1String("QStringList"), QLatin1String("Array"));
    type.replace(QRegExp(QLatin1String("^string$")),QLatin1String("String"));
    type.replace(QLatin1String("QString"), QLatin1String("String"));
    type.replace(QLatin1String("bool"), QLatin1String("true|false"));
    type.replace(QRegExp(QLatin1String("^(uint|int|ushort|short|ulong|long)$")), QLatin1String("Integer"));
    type.replace(QRegExp(QLatin1String("^(float|double)$")), QLatin1String("Float"));
    type.replace(QRegExp(QLatin1String("^Q(?=[A-Z])")), QLatin1String("Qt::"));
    type.replace(QRegExp(QLatin1String("^K(?!(DE|Parts|IO)")), QLatin1String("KDE::"));

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
    name.remove(QRegExp(QLatin1String("^m_")));
    name.remove(QRegExp(QLatin1String("^[pbn](?=[A-Z])")));
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
          << QLatin1String("__FILE__")
          << QLatin1String("__LINE__")
          << QLatin1String("BEGIN")
          << QLatin1String("END")
          << QLatin1String("alias")
          << QLatin1String("and")
          << QLatin1String("begin")
          << QLatin1String("break")
          << QLatin1String("case")
          << QLatin1String("class")
          << QLatin1String("def")
          << QLatin1String("defined?")
          << QLatin1String("do")
          << QLatin1String("else")
          << QLatin1String("elsif")
          << QLatin1String("end")
          << QLatin1String("ensure")
          << QLatin1String("false")
          << QLatin1String("for")
          << QLatin1String("if")
          << QLatin1String("in")
          << QLatin1String("module")
          << QLatin1String("next")
          << QLatin1String("nil")
          << QLatin1String("not")
          << QLatin1String("or")
          << QLatin1String("redo")
          << QLatin1String("rescue")
          << QLatin1String("retry")
          << QLatin1String("return")
          << QLatin1String("self")
          << QLatin1String("super")
          << QLatin1String("then")
          << QLatin1String("true")
          << QLatin1String("undef")
          << QLatin1String("unless")
          << QLatin1String("until")
          << QLatin1String("when")
          << QLatin1String("while")
          << QLatin1String("yield");
    }

    return keywords;
}

