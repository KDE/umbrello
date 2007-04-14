/***************************************************************************
                          rubyclassdeclarationblock.cpp
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
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "rubyclassdeclarationblock.h"
#include "rubycodedocumentation.h"
#include "rubycodegenerator.h"
#include "../uml.h"

// Constructors/Destructors
//

RubyClassDeclarationBlock::RubyClassDeclarationBlock
 ( RubyClassifierCodeDocument * parentDoc, const QString &startText, const QString &endText, const QString &comment)
        : OwnedHierarchicalCodeBlock(parentDoc->getParentClassifier(), parentDoc, startText, endText, comment)
{
    init(parentDoc, comment);
}

RubyClassDeclarationBlock::~RubyClassDeclarationBlock ( ) { }

//
// Methods
//

/**
 * Save the XMI representation of this object
 */
void RubyClassDeclarationBlock::saveToXMI ( QDomDocument & doc, QDomElement & root ) {
    QDomElement blockElement = doc.createElement( "rubyclassdeclarationblock" );

    setAttributesOnNode(doc, blockElement);

    root.appendChild( blockElement );
}

/**
 * load params from the appropriate XMI element node.
 */
void RubyClassDeclarationBlock::loadFromXMI ( QDomElement & root )
{
    setAttributesFromNode(root);
}

// Accessor methods
//

// Other methods
//

/**
 * update the start and end text for this ownedhierarchicalcodeblock.
 */
void RubyClassDeclarationBlock::updateContent ( )
{

    RubyClassifierCodeDocument *parentDoc = dynamic_cast<RubyClassifierCodeDocument*>(getParentDocument());
    UMLClassifier *c = parentDoc->getParentClassifier();
    CodeGenerationPolicy * p = UMLApp::app()->getCommonPolicy();
    QString endLine = p->getNewLineEndingChars();
    bool isInterface = parentDoc->parentIsInterface(); // a little shortcut
    QString RubyClassName = parentDoc->getRubyClassName(c->getName());
    bool forceDoc = p->getCodeVerboseDocumentComments();

    // COMMENT
    QString comment = c->getDoc();
    comment.replace("@ref ", "");
    comment.replace("@see", "_See_");
    comment.replace("@short", "_Summary_");
    comment.replace("@author", "_Author_");

    if (isInterface)
        getComment()->setText("Module " + RubyClassName + endLine + comment);
    else
        getComment()->setText("Class " + RubyClassName + endLine + comment);

    if(forceDoc || !c->getDoc().isEmpty())
        getComment()->setWriteOutText(true);
    else
        getComment()->setWriteOutText(false);


    // Now set START/ENDING Text
    QString startText = "";

    if (parentDoc->parentIsInterface()) {
        startText.append("module ");
    } else {
        startText.append("class ");
    }

    UMLClassifierList superclasses = c->findSuperClassConcepts(UMLClassifier::CLASS);
    UMLClassifierList superinterfaces = c->findSuperClassConcepts(UMLClassifier::INTERFACE);

    // write out inheritance
    startText.append(RubyClassName);

    int i = 0;
    for (UMLClassifier * concept= superclasses.first(); concept; concept = superclasses.next()) {
        if (i == 0) {
            startText.append(QString(" < ") + RubyCodeGenerator::cppToRubyType(concept->getName()) + endLine);
        } else {
            // After the first superclass name in the list, assume the classes
            // are ruby modules that can be mixed in,
            startText.append("include " + RubyCodeGenerator::cppToRubyType(concept->getName()) + endLine);
        }
        i++;
    }

    // Write out the interfaces we 'implement'. Are these modules to be mixed in, in Ruby?
    for (UMLClassifier * concept= superinterfaces.first(); concept; concept = superinterfaces.next()) {
        startText.append(QString("include ") + RubyCodeGenerator::cppToRubyType(concept->getName()) + endLine);
    }

    // Set the header and end text for the hier.codeblock
    setStartText(startText);
}

void RubyClassDeclarationBlock::init (RubyClassifierCodeDocument *parentDoc, const QString &comment)
{

    setComment(new RubyCodeDocumentation(parentDoc));
    getComment()->setText(comment);

    setEndText("end");

    updateContent();

}


#include "rubyclassdeclarationblock.moc"
