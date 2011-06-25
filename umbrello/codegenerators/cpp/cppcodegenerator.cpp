/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "cppcodegenerator.h"

// app includes
#include "cppcodedocumentation.h"
#include "cppcodegenerationpolicy.h"
#include "cppsourcecodedocument.h"
#include "cppheadercodedocument.h"
#include "codegen_utils.h"
#include "codeviewerdialog.h"
#include "codedocumentlist.h"
#include "uml.h"

// kde includes
#include <kconfig.h>

// qt includes
#include <QtCore/QRegExp>

const bool CPPCodeGenerator::DEFAULT_BUILD_MAKEFILE = false;

/**
 * Basic Constructor.
 */
CPPCodeGenerator::CPPCodeGenerator()
{
    initAttributes();
}

/**
 * Destructor.
 */
CPPCodeGenerator::~CPPCodeGenerator()
{
    // destroy all separately owned codedocuments (e.g. header docs)
    qDeleteAll(m_headercodedocumentVector);
    m_headercodedocumentVector.clear();
}

/**
 * Returns language identifier. In this case "Cpp".
 * @return language identifier
 */
Uml::ProgrammingLanguage CPPCodeGenerator::language() const
{
    return Uml::ProgrammingLanguage::Cpp;
}

/**
 * Set the value of m_createMakefile
 * @param buildIt the new value to set for creating makefile
 */
void CPPCodeGenerator::setCreateProjectMakefile(bool buildIt)
{
    m_createMakefile = buildIt;
    CodeDocument * antDoc = findCodeDocumentByID(CPPMakefileCodeDocument::DOCUMENT_ID_VALUE);
    if (antDoc) {
        antDoc->setWriteOutCode(buildIt);
    }
}

/**
 * Get the value of m_createMakefile
 * @return the value of m_createMakefile
 */
bool CPPCodeGenerator::getCreateProjectMakefile()
{
    return m_createMakefile;
}

/**
 * Add a header CodeDocument object from m_headercodedocumentVector List
 * @param doc   the header code document
 * @return      success status
 */
bool CPPCodeGenerator::addHeaderCodeDocument(CPPHeaderCodeDocument * doc)
{
    QString tag = doc->getID();

    // assign a tag if one doesn't already exist
    if (tag.isEmpty()) {
        tag = "cppheader"+ID2STR(doc->getParentClassifier()->id());
        doc->setID(tag);
    }

    if (m_codeDocumentDictionary.contains(tag))
        return false; // return false, we already have some object with this tag in the list
    else
        m_codeDocumentDictionary.insert(tag, doc);

    m_headercodedocumentVector.append(doc);
    return true;
}

/**
 * Remove a header CodeDocument object from m_headercodedocumentVector List
 */
bool CPPCodeGenerator::removeHeaderCodeDocument(CPPHeaderCodeDocument * remove_object)
{
    QString tag = remove_object->getID();
    if (!(tag.isEmpty()))
        m_codeDocumentDictionary.remove(tag);
    else
        return false;

    m_headercodedocumentVector.removeAll(remove_object);
    return true;
}

/**
 * Get the editing dialog for this code document.
 * In the C++ version, we need to make both source and header files as well
 * as the makefile available.
 * @param parent   the parent widget
 * @param doc      the code document
 * @param state    the code viewer state
 * @return         the code viewer dialog object
 */
CodeViewerDialog * CPPCodeGenerator::getCodeViewerDialog(QWidget* parent, CodeDocument *doc,
        Settings::CodeViewerState state)
{
    ClassifierCodeDocument * cdoc = dynamic_cast<ClassifierCodeDocument*>(doc);
    if (!cdoc)
        // bah..not a classcode document?? then just use vanilla version
        return CodeGenerator::getCodeViewerDialog(parent,doc,state);
    else {
        // build with passed (source) code document
        CodeViewerDialog *dialog;

        // use classifier to find appropriate header document
        UMLClassifier * c = cdoc->getParentClassifier();
        CPPHeaderCodeDocument * hdoc = findHeaderCodeDocumentByClassifier(c);
        if (hdoc) {
            // if we have a header document..build with that
            dialog = new CodeViewerDialog(parent, hdoc, state);
            dialog->addCodeDocument(doc);
        }
        else {
            // shouldn't happen, but lets try to gracefully deliver something.
            dialog = new CodeViewerDialog(parent, doc, state);
        }
        // add in makefile if available and desired
        if (getCreateProjectMakefile()) {
            dialog->addCodeDocument(findCodeDocumentByID(CPPMakefileCodeDocument::DOCUMENT_ID_VALUE));
        }
        return dialog;
    }
}

/**
 * Change the following dataTypes to the ones the user really
 * wants in their code. Not yet complete.
 * @param name   type name
 * @return       clean name     
 */
QString CPPCodeGenerator::fixTypeName(const QString &name)
{
    return cleanName(name);
}

/**
 * Save the XMI representation of this object.
 * Special method needed so that we write out the header code documents.
 * @param doc    the document
 * @param root   the root element
 */
void CPPCodeGenerator::saveToXMI(QDomDocument & doc, QDomElement & root)
{
    QDomElement docElement = doc.createElement( "codegenerator" );
    docElement.setAttribute("language", "C++");

    const CodeDocumentList * docList = getCodeDocumentList();
    CodeDocumentList::ConstIterator it  = docList->begin();
    CodeDocumentList::ConstIterator end  = docList->end();
    for ( ; it != end; ++it )
        (*it)->saveToXMI(doc, docElement);

    CodeDocumentList::Iterator it2  = m_headercodedocumentVector.begin();
    CodeDocumentList::Iterator end2  = m_headercodedocumentVector.end();
    for ( ; it2 != end2; ++it2 )
        (*it2)->saveToXMI(doc, docElement);

    root.appendChild( docElement );
}

/**
 * Force a synchronize of this code generator, and its present contents, to that of the parent UMLDocument.
 * "UserGenerated" code will be preserved, but Autogenerated contents will be updated/replaced
 * or removed as is apppropriate.
 * Need to override parent method because we have header documents to consider too.
 */
void CPPCodeGenerator::syncCodeToDocument()
{
    const CodeDocumentList * docList = getCodeDocumentList();
    CodeDocumentList::ConstIterator it  = docList->begin();
    CodeDocumentList::ConstIterator end  = docList->end();
    for ( ; it != end; ++it ) {
        (*it)->synchronize();
    }
    CodeDocumentList::Iterator it2  = m_headercodedocumentVector.begin();
    CodeDocumentList::Iterator end2  = m_headercodedocumentVector.end();
    for ( ; it2 != end2; ++it2 ) {
        (*it2)->synchronize();
    }
}

/**
 * Write out all code documents to file as appropriate.
 */
void CPPCodeGenerator::writeCodeToFile()
{
    // write all source documents (incl. Makefile)
    writeListedCodeDocsToFile(getCodeDocumentList());

    // write all header documents
    writeListedCodeDocsToFile(&m_headercodedocumentVector);
}

/**
 * this method is here to provide class wizard the
 * ability to write out only those classes which
 * are selected by the user.
 * overridden because we need to be able to generate code for
 * both the header and source documents
 */
void CPPCodeGenerator::writeCodeToFile(UMLClassifierList & concepts)
{
    CodeDocumentList docs;

    foreach (UMLClassifier* concept, concepts ) {
        CodeDocument * doc = findCodeDocumentByClassifier(concept);
        if(doc)
            docs.append(doc);
        CodeDocument * hdoc = findHeaderCodeDocumentByClassifier(concept);
        if(hdoc)
            docs.append(hdoc);
    }

    writeListedCodeDocsToFile(&docs);
}

/**
 * Find a cppheadercodedocument by the given classifier.
 * @param classifier   UML classifier
 * @return             CPPHeaderCodeDocument object
 */
CPPHeaderCodeDocument * CPPCodeGenerator::findHeaderCodeDocumentByClassifier(UMLClassifier * classifier)
{
    CodeDocument * doc = findCodeDocumentByID("cppheader"+ID2STR(classifier->id()));
    return dynamic_cast<CPPHeaderCodeDocument*>(doc);
}

/**
 * Generate classifier code document (source document version).
 * @param classifier   the classifier for which the CodeDocument is to be created
 * @return             created ClassifierCodeDocument object
 */
CodeDocument * CPPCodeGenerator::newClassifierCodeDocument(UMLClassifier * classifier)
{
    ClassifierCodeDocument *doc = new CPPSourceCodeDocument(classifier);
    doc->initCodeClassFields();
    return doc;
}

/**
 * Generate header classifier code document.
 * @param classifier   the classifier for which the CodeDocument is to be created
 * @return             created CPPHeaderCodeDocument object
 */
CPPHeaderCodeDocument * CPPCodeGenerator::newHeaderClassifierCodeDocument(UMLClassifier * classifier)
{
    CPPHeaderCodeDocument *doc = new CPPHeaderCodeDocument(classifier);
    doc->initCodeClassFields();
    return doc;
}

/**
 * Create a new CPPMakefileCodeDocument.
 * @return  CPPMakefileCodeDocument object
 */
CPPMakefileCodeDocument * CPPCodeGenerator::newMakefileCodeDocument()
{
    return new CPPMakefileCodeDocument();
}

/**
 * Overloaded so that we may have both source and header documents for each
 * classifier.
 */
void CPPCodeGenerator::initFromParentDocument()
{
    // Walk through the document converting classifiers into
    // classifier code documents as needed (e.g only if doesn't exist)
    UMLClassifierList concepts = UMLApp::app()->document()->classesAndInterfaces();
    foreach (UMLClassifier* c, concepts ) {
        // Doesn't exist? Then build one.
        CodeDocument * codeDoc = findCodeDocumentByClassifier(c);
        if (!codeDoc)
        {
            codeDoc = newClassifierCodeDocument(c);
            codeDoc->synchronize();
            addCodeDocument(codeDoc); // this will also add a unique tag to the code document
        }

        CPPHeaderCodeDocument * hcodeDoc = findHeaderCodeDocumentByClassifier(c);
        if (!hcodeDoc)
        {
            hcodeDoc = newHeaderClassifierCodeDocument(c);
            hcodeDoc->synchronize();
            addHeaderCodeDocument(hcodeDoc); // this will also add a unique tag to the code document
        }
    }

}

/**
 * Check for adding objects to the UMLDocument.
 * They are need to be overridden here because unlike in the Java (or most other lang)
 * we add 2 types of classifiercodedocument per classifier,
 * e.g. a "source" and a "header" document.
 * Need to worry about adding both source, and header documents for each classifier.
 * @param obj   the UML object 
 */
void CPPCodeGenerator::checkAddUMLObject(UMLObject * obj)
{
    if (!obj)
        return;

    // if the obj being created is a native data type
    // there's no reason to create a .h/.cpp file
    if (isReservedKeyword(obj->name()))
        return;

    UMLClassifier * c = dynamic_cast<UMLClassifier*>(obj);
    if(c) {
        CodeDocument * cDoc = newClassifierCodeDocument(c);
        CPPHeaderCodeDocument * hcodeDoc = newHeaderClassifierCodeDocument(c);
        addCodeDocument(cDoc);
        addHeaderCodeDocument(hcodeDoc); // this will also add a unique tag to the code document
    }
}

/**
 * Check for removing objects to the UMLDocument.
 * they are need to be overridden here because unlike in the Java (or most other lang)
 * we add 2 types of classifiercodedocument per classifier,
 * e.g. a "source" and a "header" document.
 * Need to worry about removing both source, and header documents for each classifier.
 * @param obj   the UML object 
 */
void CPPCodeGenerator::checkRemoveUMLObject(UMLObject * obj)
{
    if (!obj)
        return;

    UMLClassifier * c = dynamic_cast<UMLClassifier*>(obj);
    if(c) {
        // source
        ClassifierCodeDocument * cDoc = (ClassifierCodeDocument*) findCodeDocumentByClassifier(c);
        if (cDoc)
            removeCodeDocument(cDoc);

        // header
        CPPHeaderCodeDocument * hcodeDoc = findHeaderCodeDocumentByClassifier(c);
        if (hcodeDoc)
            removeHeaderCodeDocument(hcodeDoc);
    }

}

/**
 * Init all attributes.
 */
void CPPCodeGenerator::initAttributes()
{
    m_createMakefile = false;

    UMLApp::app()->setPolicyExt ( new CPPCodeGenerationPolicy() );

    // load Classifier documents from parent document
    //initFromParentDocument();
}

/**
 * Add C++ primitives as datatypes.
 * @return a string list of C++ datatypes
 */
QStringList CPPCodeGenerator::defaultDatatypes()
{
    return Codegen_Utils::cppDatatypes();
}

/**
 * Get list of reserved keywords.
 * @return a string list with reserve keywords of this language
 */
QStringList CPPCodeGenerator::reservedKeywords() const
{
    return Codegen_Utils::reservedCppKeywords();
}

/**
 * Add the default stereotypes for c++ (constructor, int etc)
 */
void CPPCodeGenerator::createDefaultStereotypes()
{
    Codegen_Utils::createCppStereotypes();
}

#include "cppcodegenerator.moc"
