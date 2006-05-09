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
 ***************************************************************************/

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qregexp.h>

#include "rubycodegenerator.h"
#include "rubycodecomment.h"
#include "codeviewerdialog.h"
#include "../uml.h"

// Constructors/Destructors
//

RubyCodeGenerator::RubyCodeGenerator (QDomElement & elem )
{
    initFields();
    loadFromXMI(elem);
}

RubyCodeGenerator::RubyCodeGenerator ()
{
    initFields();
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
    return dynamic_cast<RubyCodeGenerationPolicy*>(m_codegeneratorpolicy);
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

QString RubyCodeGenerator::capitalizeFirstLetter(QString string)
{
    // we could lowercase everything tostart and then capitalize? Nah, it would
    // screw up formatting like getMyRadicalVariable() to getMyradicalvariable(). Bah.
    QChar firstChar = string.at(0);
    string.replace( 0, 1, firstChar.upper());
    return string;
}

QString RubyCodeGenerator::cppToRubyType(QString typeStr) {
    typeStr = cleanName(typeStr);
    typeStr.replace("const ", "");
    typeStr.replace(QRegExp("[*&\\s]"), "");
    typeStr.replace(QRegExp("[<>]"), "_");
    typeStr.replace("QStringList", "Array");
    typeStr.replace(QRegExp("^string$"),"String");
    typeStr.replace("QString", "String");
    typeStr.replace("bool", "true|false");
    typeStr.replace(QRegExp("^(uint|int|ushort|short|ulong|long)$"), "Integer");
    typeStr.replace(QRegExp("^(float|double)$"), "Float");
    typeStr.replace(QRegExp("^Q(?=[A-Z])"), "Qt::");
    typeStr.replace(QRegExp("^K(?!(DE|Parts|IO)"), "KDE::");

    return typeStr;
}

QString RubyCodeGenerator::cppToRubyName(QString nameStr) {
    nameStr = cleanName(nameStr);
    nameStr.replace(QRegExp("^m_"), "");
    nameStr.replace(QRegExp("^[pbn](?=[A-Z])"), "");
    nameStr = nameStr.mid(0, 1).lower() + nameStr.mid(1);
    return nameStr;
}

/**
 * @return      ClassifierCodeDocument
 * @param       classifier
 */
CodeDocument * RubyCodeGenerator::newClassifierCodeDocument ( UMLClassifier * c)
{
    RubyClassifierCodeDocument * doc = new RubyClassifierCodeDocument(c,this);
    return doc;
}

CodeComment * RubyCodeGenerator::newCodeComment ( CodeDocument * doc) {
    return new RubyCodeComment(doc);
}

void RubyCodeGenerator::initFields() {

    UMLApp::app()->setPolicyExt ( new RubyCodeGenerationPolicy(UMLApp::app()->getConfig()) );

    // load Classifier documents from parent document
    //initFromParentDocument();
}

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

