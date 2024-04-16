/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cppheaderclassdeclarationblock.h"
#include "cppcodegenerator.h"
#include "cppcodegenerationpolicy.h"
#include "cppcodedocumentation.h"
#include "model_utils.h"
#include "uml.h"

CPPHeaderClassDeclarationBlock::CPPHeaderClassDeclarationBlock
  (CPPHeaderCodeDocument * parentDoc, const QString &startText, const QString &endText, const QString &comment)
        : OwnedHierarchicalCodeBlock(parentDoc->getParentClassifier(), parentDoc, startText, endText, comment)
{
    init(parentDoc, comment);
}

CPPHeaderClassDeclarationBlock::~CPPHeaderClassDeclarationBlock ()
{
}

/**
 * load params from the appropriate XMI element node.
 */
void CPPHeaderClassDeclarationBlock::loadFromXMI (QDomElement & root)
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
void CPPHeaderClassDeclarationBlock::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("cppheaderclassdeclarationblock"));

    setAttributesOnNode(writer);

    writer.writeEndElement();
}

/**
 * update the start and end text for this hierarchicalcodeblock.
 */
void CPPHeaderClassDeclarationBlock::updateContent ()
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
        getComment()->setText(QStringLiteral("******************************* Abstract Class ****************************") + endLine
                              + CPPHeaderClassName + QStringLiteral(" does not have any pure virtual methods, but its author") + endLine
                              + QStringLiteral("  defined it as an abstract class, so you should not use it directly.") + endLine
                              + QStringLiteral("  Inherit from it instead and create only objects from the derived classes") + endLine
                              + QStringLiteral("*****************************************************************************"));
    } else {
        if(isInterface)
            getComment()->setText(QStringLiteral("Interface ") + CPPHeaderClassName + endLine + c->doc());
        else
            getComment()->setText(QStringLiteral("Class ") + CPPHeaderClassName + endLine + c->doc());
    }

    if(forceDoc || !c->doc().isEmpty())
        getComment()->setWriteOutText(true);
    else
        getComment()->setWriteOutText(false);


    // Now set START/ENDING Text
    QString startText;

    /*
    */

    /*
        if(parentDoc->parentIsInterface())
                startText.append(QStringLiteral("interface "));
        else
    */
    startText.append(QStringLiteral("class "));

    startText.append(CPPHeaderClassName);

    // write inheritances out
    UMLClassifierList superclasses = c->findSuperClassConcepts();
    int nrof_superclasses = superclasses.count();

    // write out inheritance
    int i = 0;
    if(nrof_superclasses >0)
        startText.append(QStringLiteral(" : "));
    for(UMLClassifier *classifier : superclasses) {
        startText.append(Uml::Visibility::toString(classifier->visibility()) + QLatin1Char(' ') +
            CodeGenerator::cleanName(classifier->name()));
        if(i != (nrof_superclasses-1))
            startText.append(QStringLiteral(", "));
        i++;
    }

    // Set the header and end text for the hier.codeblock
    setStartText(startText + QStringLiteral(" {"));

    // setEndText(QStringLiteral("}")); // not needed
}

void CPPHeaderClassDeclarationBlock::init (CPPHeaderCodeDocument *parentDoc, const QString &comment)
{
    setComment(new CPPCodeDocumentation(parentDoc));
    getComment()->setText(comment);

    setEndText(QStringLiteral("};"));
}

