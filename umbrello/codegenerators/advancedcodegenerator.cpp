/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2015                                              *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "advancedcodegenerator.h"

#include "classifier.h"
#include "classifiercodedocument.h"
#include "codeviewerdialog.h"
#include "uml.h"
#include "umldoc.h"

/**
 * Constructor
 */
AdvancedCodeGenerator::AdvancedCodeGenerator()
{
}

/**
 * Destructor
 */
AdvancedCodeGenerator::~AdvancedCodeGenerator()
{
}

/**
 * Get the editing dialog for this code document.
 */
CodeViewerDialog * AdvancedCodeGenerator::getCodeViewerDialog(QWidget* parent, CodeDocument *doc,
        Settings::CodeViewerState & state)
{
    return new CodeViewerDialog(parent, doc, state);
}

/**
 * This function checks for adding objects to the UMLDocument.
 */
void AdvancedCodeGenerator::checkAddUMLObject(UMLObject * obj)
{
    if (!obj) {
        return;
    }

    UMLClassifier * c = dynamic_cast<UMLClassifier*>(obj);
    if (c) {
        CodeDocument * cDoc = newClassifierCodeDocument(c);
        addCodeDocument(cDoc);
    }
}

/**
 * This function checks for removing objects from the UMLDocument.
 */
void AdvancedCodeGenerator::checkRemoveUMLObject(UMLObject * obj)
{
    if (!obj) {
        return;
    }

    UMLClassifier * c = dynamic_cast<UMLClassifier*>(obj);
    if (c) {
        ClassifierCodeDocument * cDoc = (ClassifierCodeDocument*) findCodeDocumentByClassifier(c);
        if (cDoc) {
            removeCodeDocument(cDoc);
        }
    }
}

/**
 * Initialize this code generator from its parent UMLDoc. When this is called,
 * it will (re-)generate the list of code documents for this project (generator)
 * by checking for new objects/attributes which have been added or changed in the
 * document. One or more CodeDocuments will be created/overwritten/amended as is
 * appropriate for the given language.
 * <p>ClassifierCodeDocument
 * In this 'generic' version a ClassifierCodeDocument will exist for each and
 * every classifier that exists in our UMLDoc. IF when this is called, a code document
 * doesn't exist for the given classifier, then we will created and add a new code
 * document to our generator.
 * <p>
 * IF you want to add non-classifier related code documents at this step,
 * you will need to overload this method in the appropriate
 * code generatator (see JavaCodeGenerator for an example of this).
 */
void AdvancedCodeGenerator::initFromParentDocument()
{
    // Walk through the document converting classifiers into
    // classifier code documents as needed (e.g only if doesn't exist)
    UMLClassifierList concepts = m_document->classesAndInterfaces();
    foreach (UMLClassifier *c, concepts) {
        // Doesn't exist? Then build one.
        CodeDocument * codeDoc = findCodeDocumentByClassifier(c);
        if (!codeDoc) {
            codeDoc = newClassifierCodeDocument(c);
            addCodeDocument(codeDoc); // this will also add a unique tag to the code document
        }
    }
}

/**
 * Connect additional slots.
 * To be called after constructing the code generator (see CodeGenFactory).
 */
void AdvancedCodeGenerator::connect_newcodegen_slots()
{
    connect(m_document, SIGNAL(sigObjectCreated(UMLObject*)),
            this, SLOT(checkAddUMLObject(UMLObject*)));
    connect(m_document, SIGNAL(sigObjectRemoved(UMLObject*)),
            this, SLOT(checkRemoveUMLObject(UMLObject*)));
    CodeGenerationPolicy *commonPolicy = UMLApp::app()->commonPolicy();
    connect(commonPolicy, SIGNAL(modifiedCodeContent()),
            this, SLOT(syncCodeToDocument()));
}
