/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "dclassdeclarationblock.h"

#include "dcodegenerator.h"
#include "dcodegenerationpolicy.h"
#include "dcodedocumentation.h"
#include "model_utils.h"
#include "umlapp.h"

DClassDeclarationBlock::DClassDeclarationBlock
  (DClassifierCodeDocument * parentDoc, const QString &startText, const QString &endText, const QString &comment)
        : OwnedHierarchicalCodeBlock(parentDoc->getParentClassifier(), parentDoc, startText, endText, comment)
{
    init(parentDoc, comment);
}

DClassDeclarationBlock::~DClassDeclarationBlock ()
{
}

void DClassDeclarationBlock::loadFromXMI (QDomElement & root)
{
    setAttributesFromNode(root);
}

void DClassDeclarationBlock::setAttributesFromObject (TextBlock * obj)
{
    HierarchicalCodeBlock::setAttributesFromObject(obj);
}

void DClassDeclarationBlock::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("dclassdeclarationblock"));

    setAttributesOnNode(writer);

    writer.writeEndElement();
}

void DClassDeclarationBlock::updateContent ()
{
    DClassifierCodeDocument *parentDoc = dynamic_cast<DClassifierCodeDocument*>(getParentDocument());
    UMLClassifier *c = parentDoc->getParentClassifier();
    CodeGenerationPolicy *commonPolicy = UMLApp::app()->commonPolicy();
    QString endLine = commonPolicy->getNewLineEndingChars();
    bool isInterface = parentDoc->parentIsInterface(); // a little shortcut
    QString DClassName = parentDoc->getDClassName(c->name());

    // COMMENT

    getComment()->setText(
        (isInterface ? QStringLiteral("Interface ") : QStringLiteral("Class ")) +
        DClassName + endLine + c->doc());

    bool forceDoc = commonPolicy->getCodeVerboseDocumentComments();

    getComment()->setWriteOutText(forceDoc || !c->doc().isEmpty());

    /*
     * Class declaration
     *
     * (private) class foo : class1, ..., classN, interface1, ..., interfaceN {
     *     a       b    c  d   e                      f                       g
     */

    QString startText;

    // (a) visibility modifier
    switch(c->visibility()) {
        case Uml::Visibility::Private: startText += QStringLiteral("private "); break;
        default: break;
    }

    // (b) keyword
    if (isInterface) {
        startText += QStringLiteral("interface ");
    } else {
        if (c->isAbstract()) {
            startText += QStringLiteral("abstract ");
        }

        startText += QStringLiteral("class ");
    }

    // (c) class name
    startText += DClassName;

    // (d) inheritances
    UMLClassifierList superclasses =
        c->findSuperClassConcepts(UMLClassifier::CLASS);
    UMLClassifierList superinterfaces =
        c->findSuperClassConcepts(UMLClassifier::INTERFACE);

    int count = superclasses.count() + superinterfaces.count();

    if (count > 0) startText += QStringLiteral(" : ");

    // (e) base classes
    for(UMLClassifier* classifier : superclasses) {
        startText += parentDoc->cleanName(classifier->name());

        count--;

        if (count>0) startText += QStringLiteral(", ");
    }

    // (f) interfaces
    for(UMLClassifier* classifier : superinterfaces) {
        startText += parentDoc->cleanName(classifier->name());

        count--;

        if (count>0) startText += QStringLiteral(", ");
    }

    // (g) block start
    startText += QStringLiteral(" {");

    setStartText(startText);
}

void DClassDeclarationBlock::init (DClassifierCodeDocument *parentDoc, const QString &comment)
{
    setComment(new DCodeDocumentation(parentDoc));
    getComment()->setText(comment);

    setEndText(QStringLiteral("}"));
}


