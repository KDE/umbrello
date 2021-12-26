/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "rubyclassdeclarationblock.h"
#include "rubycodedocumentation.h"
#include "rubycodegenerator.h"
#include "uml.h"

RubyClassDeclarationBlock::RubyClassDeclarationBlock
 (RubyClassifierCodeDocument * parentDoc, const QString &startText, const QString &endText, const QString &comment)
        : OwnedHierarchicalCodeBlock(parentDoc->getParentClassifier(), parentDoc, startText, endText, comment)
{
    init(parentDoc, comment);
}

RubyClassDeclarationBlock::~RubyClassDeclarationBlock ()
{
}

/**
 * Save the XMI representation of this object
 */
void RubyClassDeclarationBlock::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("rubyclassdeclarationblock"));

    setAttributesOnNode(writer);

    writer.writeEndElement();
}

/**
 * load params from the appropriate XMI element node.
 */
void RubyClassDeclarationBlock::loadFromXMI (QDomElement & root)
{
    setAttributesFromNode(root);
}

/**
 * update the start and end text for this ownedhierarchicalcodeblock.
 */
void RubyClassDeclarationBlock::updateContent ()
{
    RubyClassifierCodeDocument *parentDoc = dynamic_cast<RubyClassifierCodeDocument*>(getParentDocument());
    Q_ASSERT(parentDoc);
    UMLClassifier *c = parentDoc->getParentClassifier();
    CodeGenerationPolicy * p = UMLApp::app()->commonPolicy();
    QString endLine = p->getNewLineEndingChars();
    bool isInterface = parentDoc->parentIsInterface(); // a little shortcut
    QString RubyClassName = parentDoc->getRubyClassName(c->name());
    bool forceDoc = p->getCodeVerboseDocumentComments();

    // COMMENT
    QString comment = c->doc();
    comment.remove(QLatin1String("@ref "));
    comment.replace(QLatin1String("@see"), QLatin1String("_See_"));
    comment.replace(QLatin1String("@short"), QLatin1String("_Summary_"));
    comment.replace(QLatin1String("@author"), QLatin1String("_Author_"));

    if (isInterface)
        getComment()->setText(QLatin1String("Module ") + RubyClassName + endLine + comment);
    else
        getComment()->setText(QLatin1String("Class ") + RubyClassName + endLine + comment);

    if (forceDoc || !c->doc().isEmpty())
        getComment()->setWriteOutText(true);
    else
        getComment()->setWriteOutText(false);

    // Now set START/ENDING Text
    QString startText;

    if (parentDoc->parentIsInterface()) {
        startText.append(QLatin1String("module "));
    } else {
        startText.append(QLatin1String("class "));
    }

    UMLClassifierList superclasses = c->findSuperClassConcepts(UMLClassifier::CLASS);
    UMLClassifierList superinterfaces = c->findSuperClassConcepts(UMLClassifier::INTERFACE);

    // write out inheritance
    startText.append(RubyClassName);

    int i = 0;
    foreach (UMLClassifier* concept, superclasses) {
        if (i == 0) {
            startText.append(QString(QLatin1String(" < ")) + RubyCodeGenerator::cppToRubyType(concept->name()) + endLine);
        } else {
            // After the first superclass name in the list, assume the classes
            // are ruby modules that can be mixed in,
            startText.append(QLatin1String("include ") + RubyCodeGenerator::cppToRubyType(concept->name()) + endLine);
        }
        i++;
    }

    // Write out the interfaces we 'implement'. Are these modules to be mixed in, in Ruby?
    foreach (UMLClassifier* concept, superinterfaces) {
        startText.append(QString(QLatin1String("include ")) + RubyCodeGenerator::cppToRubyType(concept->name()) + endLine);
    }

    // Set the header and end text for the hier.codeblock
    setStartText(startText);
}

void RubyClassDeclarationBlock::init (RubyClassifierCodeDocument *parentDoc, const QString &comment)
{
    setComment(new RubyCodeDocumentation(parentDoc));
    getComment()->setText(comment);

    setEndText(QLatin1String("end"));
}

