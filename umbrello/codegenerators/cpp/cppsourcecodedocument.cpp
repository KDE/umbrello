/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

/**
 * We carve the CPP document up into 2 documents, "source" and "header".
 * The sections of each are as follows:
 * - header
 * - includes
 * - constructor methods
 * - all other methods
*/

// own header
#include "cppsourcecodedocument.h"

// app includes
#include "cppcodegenerator.h"
#include "cppcodegenerationpolicy.h"
#include "cppcodedocumentation.h"
#include "cppcodeclassfield.h"
#include "cppsourcecodeclassfielddeclarationblock.h"
#include "debug_utils.h"
#include "uml.h"

// qt includes
#include <QRegExp>

CPPSourceCodeDocument::CPPSourceCodeDocument (UMLClassifier * concept)
        : ClassifierCodeDocument (concept)
{
    setFileExtension(QLatin1String(".cpp"));

    m_methodsBlock = 0;
    m_constructorBlock = 0;

    /* We cannot call any virtual initialization functions here because
       the object is still under construction and the C++ dispatch table
       is not yet set up.
       Full initialization is done in CodeGenFactory::newCodeOperation()
     */
}

CPPSourceCodeDocument::~CPPSourceCodeDocument()
{
}

bool CPPSourceCodeDocument::addCodeOperation (CodeOperation * op)
{
    bool retval = false;
    if (op->getParentOperation()->isLifeOperation()) {
        if (m_constructorBlock)
            retval = m_constructorBlock->addTextBlock(op);
        else
            uError() << "m_constructorBlock is NULL";
    } else {
        if (m_methodsBlock)
            retval = m_methodsBlock->addTextBlock(op);
        else
            uError() << "m_methodsBlock is NULL";
    }
    return retval;
}

void CPPSourceCodeDocument::resetTextBlocks()
{
    // all special pointers need to be zero'd out.
    m_methodsBlock = 0;
    m_constructorBlock = 0;

    // now do the traditional release of child text blocks
    ClassifierCodeDocument::resetTextBlocks();
}

void CPPSourceCodeDocument::updateContent()
{
    // Gather info on the various fields and parent objects of this class...
    //UMLClassifier * c = getParentClassifier();
    CodeGenPolicyExt *pe = UMLApp::app()->policyExt();
    CPPCodeGenerationPolicy * policy = dynamic_cast<CPPCodeGenerationPolicy*>(pe);
    QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();

    // first, set the global flag on whether or not to show classfield info
    const CodeClassFieldList * cfList = getCodeClassFieldList();
    CodeClassFieldList::const_iterator it = cfList->begin();
    CodeClassFieldList::const_iterator end = cfList->end();
    for(; it != end; ++it)
        (*it)->setWriteOutMethods(policy->getAutoGenerateAccessors());

    // attribute-based ClassFields
    // we do it this way to have the static fields sorted out from regular ones
    CodeClassFieldList staticAttribClassFields = getSpecificClassFields (CodeClassField::Attribute, true);
    CodeClassFieldList attribClassFields = getSpecificClassFields (CodeClassField::Attribute, false);
    // association-based ClassFields
    // don't care if they are static or not..all are lumped together
    CodeClassFieldList plainAssocClassFields = getSpecificClassFields (CodeClassField::PlainAssociation);
    CodeClassFieldList aggregationClassFields = getSpecificClassFields (CodeClassField::Aggregation);
    CodeClassFieldList compositionClassFields = getSpecificClassFields (CodeClassField::Composition);

    // START GENERATING CODE/TEXT BLOCKS and COMMENTS FOR THE DOCUMENT

    // INCLUDE CODEBLOCK
    QString includeStatement;
    // Include own header file
    QString myOwnName(getParentClassifier()->name());
    includeStatement.append(QLatin1String("#include \"") + CodeGenerator::cleanName(myOwnName.toLower()) + QLatin1String(".h\"") + endLine);
    CodeBlockWithComments * iblock = addOrUpdateTaggedCodeBlockWithComments(QLatin1String("includes"), includeStatement, QString(), 0, false);
    iblock->setWriteOutText(true);

    // After the includes we have just 2 big blocks basically, the "constructor" block and the
    // block for the rest of our methods (operations + accessors)

    m_constructorBlock = getHierarchicalCodeBlock(QLatin1String("constructionMethodsBlock"), QLatin1String("Constructors/Destructors"), 0);
    m_methodsBlock = getHierarchicalCodeBlock(QLatin1String("otherMethodsBlock"), QLatin1String("Methods"), 0);

    // add accessors to the methods block
    m_methodsBlock->addCodeClassFieldMethods(staticAttribClassFields);
    m_methodsBlock->addCodeClassFieldMethods(attribClassFields);
    m_methodsBlock->addCodeClassFieldMethods(plainAssocClassFields);
    m_methodsBlock->addCodeClassFieldMethods(aggregationClassFields);
    m_methodsBlock->addCodeClassFieldMethods(compositionClassFields);

    // constructors and other operations are handled by the "addCodeOperation" method above.
}


