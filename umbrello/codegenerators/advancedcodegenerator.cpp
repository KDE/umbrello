/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "advancedcodegenerator.h"

#include "umlclassifier.h"
#include "classifiercodedocument.h"
#include "codeviewerdialog.h"
#include "umlapp.h"
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

    UMLClassifier * c = obj->asUMLClassifier();
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

    UMLClassifier * c = obj->asUMLClassifier();
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
    for(UMLClassifier *c : concepts) {
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
 * To be called after constructing the code generator.
 */
void AdvancedCodeGenerator::connectSlots()
{
    connect(m_document, SIGNAL(sigObjectCreated(UMLObject*)),
            this, SLOT(checkAddUMLObject(UMLObject*)));
    connect(m_document, SIGNAL(sigObjectRemoved(UMLObject*)),
            this, SLOT(checkRemoveUMLObject(UMLObject*)));
    CodeGenerationPolicy *commonPolicy = UMLApp::app()->commonPolicy();
    connect(commonPolicy, SIGNAL(modifiedCodeContent()),
            this, SLOT(syncCodeToDocument()));
}
