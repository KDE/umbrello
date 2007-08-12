/***************************************************************************
                          rubycodegenerator.cpp
                          Derived from the Java code generator by thomas

    begin                : Thur Jul 21 2005
    author               : Richard Dale
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "rubycodegenerator.h"

// qt/kde includes
#include <qregexp.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

// local includes
#include "rubycodecomment.h"
#include "codeviewerdialog.h"
#include "../uml.h"

// Constructors/Destructors
//

RubyCodeGenerator::RubyCodeGenerator (QDomElement & elem )
  : CodeGenerator(elem)
{
}

RubyCodeGenerator::RubyCodeGenerator ()
{
}

RubyCodeGenerator::~RubyCodeGenerator ( ) { }

//
// Methods
//

// Accessor methods
//

// return our language
Uml::Programming_Language RubyCodeGenerator::getLanguage() {
    return Uml::pl_Ruby;
}

// In the Java version, we make the ANT build file also available.
CodeViewerDialog * RubyCodeGenerator::getCodeViewerDialog ( QWidget* parent, CodeDocument *doc,
        Settings::CodeViewerState state)
{
    CodeViewerDialog *dialog = new CodeViewerDialog(parent, doc, state);
    return dialog;
}


RubyCodeGenerationPolicy * RubyCodeGenerator::getRubyPolicy() {
    return dynamic_cast<RubyCodeGenerationPolicy*>(UMLApp::app()->getPolicyExt());
}

bool RubyCodeGenerator::getAutoGenerateAttribAccessors ( )
{
    return getRubyPolicy()->getAutoGenerateAttribAccessors ();
}

bool RubyCodeGenerator::getAutoGenerateAssocAccessors ( )
{
    return getRubyPolicy()->getAutoGenerateAssocAccessors ();
}

QString RubyCodeGenerator::getListFieldClassName () {
    return QString("Array");
}

// Other methods
//

QString RubyCodeGenerator::cppToRubyType(const QString &typeStr) {
    QString type = cleanName(typeStr);
    type.replace("const ", "");
    type.replace(QRegExp("[*&\\s]"), "");
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

QString RubyCodeGenerator::cppToRubyName(const QString &nameStr) {
    QString name = cleanName(nameStr);
    name.replace(QRegExp("^m_"), "");
    name.replace(QRegExp("^[pbn](?=[A-Z])"), "");
    name = name.mid(0, 1).lower() + name.mid(1);
    return name;
}

/**
 * @return      ClassifierCodeDocument
 * @param       classifier
 */
CodeDocument * RubyCodeGenerator::newClassifierCodeDocument ( UMLClassifier * c)
{
    RubyClassifierCodeDocument * doc = new RubyClassifierCodeDocument(c);
    doc->initCodeClassFields();
    return doc;
}

/* These initializations are done in CodeGenFactory::createObject()
void RubyCodeGenerator::initFields() {
    UMLApp::app()->setPolicyExt ( new RubyCodeGenerationPolicy(UMLApp::app()->getConfig()) );
    // load Classifier documents from parent document
    initFromParentDocument();
}
 */

const QStringList RubyCodeGenerator::reservedKeywords() const {

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

