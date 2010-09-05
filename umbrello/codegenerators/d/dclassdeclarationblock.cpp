/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   copyright (C) 2008-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "dclassdeclarationblock.h"

#include "dcodegenerator.h"
#include "dcodegenerationpolicy.h"
#include "dcodedocumentation.h"
#include "model_utils.h"
#include "uml.h"

DClassDeclarationBlock::DClassDeclarationBlock
  ( DClassifierCodeDocument * parentDoc, const QString &startText, const QString &endText, const QString &comment)
        : OwnedHierarchicalCodeBlock(parentDoc->getParentClassifier(), parentDoc, startText, endText, comment)
{
    init(parentDoc, comment);
}

DClassDeclarationBlock::~DClassDeclarationBlock ( )
{
}

void DClassDeclarationBlock::loadFromXMI ( QDomElement & root )
{
    setAttributesFromNode(root);
}

void DClassDeclarationBlock::setAttributesFromObject (TextBlock * obj)
{
    HierarchicalCodeBlock::setAttributesFromObject(obj);
}

void DClassDeclarationBlock::saveToXMI ( QDomDocument & doc, QDomElement & root )
{
    QDomElement blockElement = doc.createElement( "dclassdeclarationblock" );

    setAttributesOnNode(doc, blockElement);

    root.appendChild( blockElement );
}

void DClassDeclarationBlock::updateContent ( )
{
    DClassifierCodeDocument *parentDoc = dynamic_cast<DClassifierCodeDocument*>(getParentDocument());
    UMLClassifier *c = parentDoc->getParentClassifier();
    CodeGenerationPolicy *commonPolicy = UMLApp::app()->commonPolicy();
    QString endLine = commonPolicy->getNewLineEndingChars();
    bool isInterface = parentDoc->parentIsInterface(); // a little shortcut
    QString DClassName = parentDoc->getDClassName(c->name());

    // COMMENT

    getComment()->setText(
        (isInterface ? "Interface " : "Class ") +
        DClassName + endLine + c->doc());

    bool forceDoc = commonPolicy->getCodeVerboseDocumentComments();

    getComment()->setWriteOutText( forceDoc || !c->doc().isEmpty() );

    /*
     * Class declaration
     *
     * (private) class foo : class1, ..., classN, interface1, ..., interfaceN {
     *     a       b    c  d   e                      f                       g
     */

    QString startText = "";

    // (a) visibility modifier
    switch(c->visibility()) {
        case Uml::Visibility::Private: startText += "private "; break;
        default: break;
    }

    // (b) keyword
    if (isInterface) {
        startText += "interface ";
    } else {
        if (c->isAbstract()) {
            startText += "abstract ";
        }

        startText += "class ";
    }

    // (c) class name
    startText += DClassName;

    // (d) inheritances
    UMLClassifierList superclasses =
        c->findSuperClassConcepts(UMLClassifier::CLASS);
    UMLClassifierList superinterfaces =
        c->findSuperClassConcepts(UMLClassifier::INTERFACE);

    int count = superclasses.count() + superinterfaces.count();

    if (count > 0) startText += " : ";

    // (e) base classes
    foreach (UMLClassifier* concept, superclasses ) {
        startText += parentDoc->cleanName(concept->name());

        count--;

        if (count>0) startText += ", ";
    }

    // (f) interfaces
    foreach (UMLClassifier* concept, superinterfaces ) {
        startText += parentDoc->cleanName(concept->name());

        count--;

        if (count>0) startText += ", ";
    }

    // (g) block start
    startText += " {";

    setStartText(startText);
}

void DClassDeclarationBlock::init (DClassifierCodeDocument *parentDoc, const QString &comment)
{
    setComment(new DCodeDocumentation(parentDoc));
    getComment()->setText(comment);

    setEndText("}");
}


#include "dclassdeclarationblock.moc"
