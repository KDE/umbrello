/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cppheaderclassdeclarationblock.h"
#include "cppcodegenerator.h"
#include "cppcodegenerationpolicy.h"
#include "cppcodedocumentation.h"
#include "model_utils.h"
#include "uml.h"

CPPHeaderClassDeclarationBlock::CPPHeaderClassDeclarationBlock
  ( CPPHeaderCodeDocument * parentDoc, const QString &startText, const QString &endText, const QString &comment)
        : OwnedHierarchicalCodeBlock(parentDoc->getParentClassifier(), parentDoc, startText, endText, comment)
{
    init(parentDoc, comment);
}

CPPHeaderClassDeclarationBlock::~CPPHeaderClassDeclarationBlock ( )
{
}

/**
 * load params from the appropriate XMI element node.
 */
void CPPHeaderClassDeclarationBlock::loadFromXMI ( QDomElement & root )
{
    setAttributesFromNode(root);
}

/** set the class attributes from a passed object
 */
void CPPHeaderClassDeclarationBlock::setAttributesFromObject (TextBlock * obj)
{
    HierarchicalCodeBlock::setAttributesFromObject(obj);
}

/**
 * Save the XMI representation of this object
 */
void CPPHeaderClassDeclarationBlock::saveToXMI ( QDomDocument & doc, QDomElement & root )
{
    QDomElement blockElement = doc.createElement( "cppheaderclassdeclarationblock" );

    setAttributesOnNode(doc, blockElement);

    root.appendChild( blockElement );
}

/**
 * update the start and end text for this hierarchicalcodeblock.
 */
void CPPHeaderClassDeclarationBlock::updateContent ( )
{
    CPPHeaderCodeDocument *parentDoc = dynamic_cast<CPPHeaderCodeDocument*>(getParentDocument());
    UMLClassifier *c = parentDoc->getParentClassifier();
    QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();
    bool isInterface = parentDoc->parentIsInterface(); // a little shortcut
    QString CPPHeaderClassName = CodeGenerator::cleanName(c->name());
    bool forceDoc = UMLApp::app()->commonPolicy()->getCodeVerboseDocumentComments();

    // COMMENT

    //check if class is abstract.. it should have abstract methods
    if(!isInterface && c->isAbstract() && !c->hasAbstractOps())
    {
        getComment()->setText("******************************* Abstract Class ****************************"+endLine
                              +CPPHeaderClassName+" does not have any pure virtual methods, but its author"+endLine
                              +"  defined it as an abstract class, so you should not use it directly."+endLine
                              +"  Inherit from it instead and create only objects from the derived classes"+endLine
                              +"*****************************************************************************");
    } else {
        if(isInterface)
            getComment()->setText("Interface "+CPPHeaderClassName+endLine+c->doc());
        else
            getComment()->setText("Class "+CPPHeaderClassName+endLine+c->doc());
    }

    if(forceDoc || !c->doc().isEmpty())
        getComment()->setWriteOutText(true);
    else
        getComment()->setWriteOutText(false);


    // Now set START/ENDING Text
    QString startText = "";

    /*
    */

    /*
        if(parentDoc->parentIsInterface())
                startText.append("interface ");
        else
    */
    startText.append("class ");

    startText.append(CPPHeaderClassName);

    // write inheritances out
    UMLClassifierList superclasses = c->findSuperClassConcepts();
    int nrof_superclasses = superclasses.count();

    // write out inheritance
    int i = 0;
    if(nrof_superclasses >0)
        startText.append(" : ");
    foreach (UMLClassifier* concept, superclasses ) {
        startText.append(concept->visibility().toString() + ' ' +
        CodeGenerator::cleanName(concept->name()));
        if(i != (nrof_superclasses-1))
            startText.append(", ");
        i++;
    }

    // Set the header and end text for the hier.codeblock
    setStartText(startText+" {");

    // setEndText("}"); // not needed
}

void CPPHeaderClassDeclarationBlock::init (CPPHeaderCodeDocument *parentDoc, const QString &comment)
{
    setComment(new CPPCodeDocumentation(parentDoc));
    getComment()->setText(comment);

    setEndText("};");
}

#include "cppheaderclassdeclarationblock.moc"
