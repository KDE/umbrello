
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*  This code generated by:
 *      Author : thomas
 *      Date   : Thu Jun 19 2003
 */

// own header
#include "cppcodegenerator.h"

// qt/kde includes
#include <qregexp.h>
#include <kdebug.h>
#include <kconfig.h>

// app includes
#include "cppcodedocumentation.h"
#include "cppcodegenerationpolicy.h"
#include "cppsourcecodedocument.h"
#include "cppheadercodedocument.h"
#include "codegen_utils.h"
#include "codeviewerdialog.h"
#include "../codedocumentlist.h"

const bool CPPCodeGenerator::DEFAULT_BUILD_MAKEFILE = false;

// Constructors/Destructors
//

CPPCodeGenerator::CPPCodeGenerator () {
    initAttributes();
}

CPPCodeGenerator::~CPPCodeGenerator ( ) {
    // destroy all separately owned codedocuments (e.g. header docs)
    CodeDocumentList * list = &m_headercodedocumentVector;
    for (CodeDocument *doc = list->first(); doc; doc=list->next())
        delete doc;
}

//
// Methods
//

// Accessor methods
//

/**
 * returns "C++"
 */
Uml::Programming_Language CPPCodeGenerator::getLanguage() {
    return Uml::pl_Cpp;
}

/**
 * Set the value of m_createMakefile
 * @param new_var the new value of m_createMakefile
 */
void CPPCodeGenerator::setCreateProjectMakefile ( bool buildIt) {
    m_createMakefile = buildIt;
    CodeDocument * antDoc = findCodeDocumentByID(CPPMakefileCodeDocument::DOCUMENT_ID_VALUE);
    if (antDoc)
        antDoc->setWriteOutCode(buildIt);
}

/**
 * Get the value of m_createMakefile
 * @return the value of m_createMakefile
 */
bool CPPCodeGenerator::getCreateProjectMakefile ( ) {
    return m_createMakefile;
}

bool CPPCodeGenerator::addHeaderCodeDocument ( CPPHeaderCodeDocument * doc )
{

    QString tag = doc->getID();

    // assign a tag if one doesnt already exist
    if(tag.isEmpty())
    {
        tag = "cppheader"+ID2STR(doc->getParentClassifier()->getID());
        doc->setID(tag);
    }

    if(m_codeDocumentDictionary.find(tag))
        return false; // return false, we already have some object with this tag in the list
    else
        m_codeDocumentDictionary.insert(tag, doc);

    m_headercodedocumentVector.append(doc);
    return true;
}

/**
 * Remove a header CodeDocument object from m_headercodedocumentVector List
 */
bool CPPCodeGenerator::removeHeaderCodeDocument ( CPPHeaderCodeDocument * remove_object ) {
    QString tag = remove_object->getID();
    if(!(tag.isEmpty()))
        m_codeDocumentDictionary.remove(tag);
    else
        return false;

    m_headercodedocumentVector.remove(remove_object);
    return true;
}

// In the C++ version, we need to make both source and header files as well
// as the makefile available.
CodeViewerDialog * CPPCodeGenerator::getCodeViewerDialog ( QWidget* parent, CodeDocument *doc,
        Settings::CodeViewerState state)
{

    ClassifierCodeDocument * cdoc = dynamic_cast<ClassifierCodeDocument*>(doc);
    if(!cdoc)
        // bah..not a classcode document?? then just use vanilla version
        return CodeGenerator::getCodeViewerDialog(parent,doc,state);
    else {
        // build with passed (source) code document
        CodeViewerDialog *dialog;

        // use classifier to find appropriate header document
        UMLClassifier * c = cdoc->getParentClassifier();
        CPPHeaderCodeDocument * hdoc = findHeaderCodeDocumentByClassifier(c);
        if(hdoc)
        {
            // if we have a header document..build with that
            dialog = new CodeViewerDialog(parent, hdoc, state);
            dialog->addCodeDocument(doc);
        } else
            // shouldnt happen, but lets try to gracefully deliver something.
            dialog = new CodeViewerDialog(parent, doc, state);

        // add in makefile if available and desired
        if(getCreateProjectMakefile())
            dialog->addCodeDocument(findCodeDocumentByID(CPPMakefileCodeDocument::DOCUMENT_ID_VALUE));

        return dialog;
    }
}

QString CPPCodeGenerator::getCPPClassName (const QString &name) {
    return cleanName(name);
}

CPPCodeGenerationPolicy::CPPCommentStyle CPPCodeGenerator::getCommentStyle ( )
{
    return ((CPPCodeGenerationPolicy*)getPolicy())->getCommentStyle();
}

bool CPPCodeGenerator::getAutoGenerateConstructors ( )
{
    return ((CPPCodeGenerationPolicy*)getPolicy())->getAutoGenerateConstructors();
}

bool CPPCodeGenerator::getAutoGenerateAccessors ( )
{
    return ((CPPCodeGenerationPolicy*)getPolicy())->getAutoGenerateAccessors ();
}

// Other methods
//

// CHange the following dataTypes to the ones the user really
// wants in their code. Not yet complete.
QString CPPCodeGenerator::fixTypeName(QString string)
{
    // FIX!!
    string.replace(QRegExp("^[Ll]ist$"),"QPtrList");
    string.replace(QRegExp("^string$"),"QString");
    return cleanName(string);
}

// special method needed so that we write out the header code documents
void CPPCodeGenerator::saveToXMI ( QDomDocument & doc, QDomElement & root ) {
    QDomElement docElement = doc.createElement( "codegenerator" );
    docElement.setAttribute("language", "C++");

    CodeDocumentList * docList = getCodeDocumentList();
    for (CodeDocument * codeDoc = docList->first(); codeDoc; codeDoc= docList->next())
        codeDoc->saveToXMI(doc, docElement);

    for (CodeDocument * hcodeDoc = m_headercodedocumentVector.first(); hcodeDoc; hcodeDoc=m_headercodedocumentVector.next())
        hcodeDoc->saveToXMI(doc, docElement);

    root.appendChild( docElement );
}

/**
 * Force a synchronize of this code generator, and its present contents, to that of the parent UMLDocument.
 * Need to override parent method because we have header documents to consider too.
 */
void CPPCodeGenerator::syncCodeToDocument ( ) {

    CodeDocumentList * docList = getCodeDocumentList();

    for (CodeDocument * doc = docList->first(); doc; doc=docList->next())
        doc->synchronize();

    for (CodeDocument * hcodeDoc = m_headercodedocumentVector.first(); hcodeDoc; hcodeDoc=m_headercodedocumentVector.next())
        hcodeDoc->synchronize();

}

/**
 * Write out all code documents to file as appropriate.
 */
void CPPCodeGenerator::writeCodeToFile ( )
{
    // write all source documents (incl. Makefile)
    writeListedCodeDocsToFile(getCodeDocumentList());

    // write all header documents
    writeListedCodeDocsToFile(&m_headercodedocumentVector);

}

// overridden because we need to be able to generate code for
// both the header and source documents
void CPPCodeGenerator::writeCodeToFile ( UMLClassifierList & concepts) {
    CodeDocumentList docs;
    docs.setAutoDelete(false);

    for (UMLClassifier *concept= concepts.first(); concept; concept= concepts.next())
    {
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
 * @return      CPPHeaderCodeDocument
* @param       classifier
*/
CPPHeaderCodeDocument * CPPCodeGenerator::findHeaderCodeDocumentByClassifier (UMLClassifier * classifier )
{
    CodeDocument * doc = findCodeDocumentByID("cppheader"+ID2STR(classifier->getID()));
    return dynamic_cast<CPPHeaderCodeDocument*>(doc);
}

/**
 * @return      ClassifierCodeDocument
 * @param       classifier
 * @param       this This package generator object.
 */
// source document version.
CodeDocument * CPPCodeGenerator::newClassifierCodeDocument (UMLClassifier * classifier)
{
    return new CPPSourceCodeDocument(classifier, this);
}

CodeComment * CPPCodeGenerator::newCodeComment ( CodeDocument * doc) {
    return new CPPCodeDocumentation(doc);
}

CPPHeaderCodeDocument * CPPCodeGenerator::newHeaderClassifierCodeDocument (UMLClassifier * classifier)
{
    return new CPPHeaderCodeDocument(classifier, this);
}

/**
 * @return      CPPMakefileCodeDocument
 * @param       this
 */
CPPMakefileCodeDocument * CPPCodeGenerator::newMakefileCodeDocument ( ) {
    return new CPPMakefileCodeDocument (this);
}


/**
 * Overloaded so that we may have both source and header documents for each
 * classifier.
 */
void CPPCodeGenerator::initFromParentDocument( ) {

    // Walk through the document converting classifiers into
    // classifier code documents as needed (e.g only if doesnt exist)
    UMLClassifierList concepts = getDocument()->getClassesAndInterfaces();
    for (UMLClassifier *c = concepts.first(); c; c = concepts.next())
    {

        // Doesnt exist? Then build one.
        CodeDocument * codeDoc = findCodeDocumentByClassifier(c);
        if (!codeDoc)
        {
            codeDoc = newClassifierCodeDocument(c);
            addCodeDocument(codeDoc); // this will also add a unique tag to the code document
        }

        CPPHeaderCodeDocument * hcodeDoc = findHeaderCodeDocumentByClassifier(c);
        if (!hcodeDoc)
        {
            hcodeDoc = new CPPHeaderCodeDocument(c,this);
            addHeaderCodeDocument(hcodeDoc); // this will also add a unique tag to the code document
        }
    }

}

// need to worry about adding both source, and header documents for each
// classifier
void CPPCodeGenerator::checkAddUMLObject (UMLObject * obj) {
    if (!obj)
        return;

    // if the obj being created is a native data type
    // there's no reason to create a .h/.cpp file
    if (isReservedKeyword(obj->getName()))
        return;

    UMLClassifier * c = dynamic_cast<UMLClassifier*>(obj);
    if(c) {
        CodeDocument * cDoc = newClassifierCodeDocument(c);
        CPPHeaderCodeDocument * hcodeDoc = new CPPHeaderCodeDocument(c, this);
        addCodeDocument(cDoc);
        addHeaderCodeDocument(hcodeDoc); // this will also add a unique tag to the code document
    }
}

// need to worry about removing both source, and header documents for each
// classifier
void CPPCodeGenerator::checkRemoveUMLObject (UMLObject * obj)
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

void CPPCodeGenerator::initAttributes ( )
{
    m_createMakefile = false;

    setPolicy ( new CPPCodeGenerationPolicy(getPolicy()) );

    // load Classifier documents from parent document
    initFromParentDocument();
}

void CPPCodeGenerator::createDefaultDatatypes() {
    Codegen_Utils::createCppDatatypes();
}

const QStringList CPPCodeGenerator::reservedKeywords() const {
    return Codegen_Utils::reservedCppKeywords();
}

void CPPCodeGenerator::createDefaultStereotypes (){
    Codegen_Utils::createCppStereotypes();
}

#include "cppcodegenerator.moc"
