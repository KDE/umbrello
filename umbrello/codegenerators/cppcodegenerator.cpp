
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

#include <kdebug.h>
#include <kconfig.h>

#include "cppcodegenerator.h"
#include "cppcodedocumentation.h"
#include "cppcodegenerationpolicy.h"
#include "cppsourcecodedocument.h"
#include "cppheadercodedocument.h"

// #include "cppheadercodedocuent.h"

const bool CPPCodeGenerator::DEFAULT_BUILD_MAKEFILE = false;

// Constructors/Destructors
//

CPPCodeGenerator::CPPCodeGenerator ( UMLDoc * parentDoc , const char * name)
    : CodeGenerator( parentDoc, name ) {

	//m_parentDoc = parentDoc; // this should be done by the call to the parent constructor?
	initAttributes();
}

CPPCodeGenerator::~CPPCodeGenerator ( ) {
	// destroy all separately owned codedocuments (e.g. header docs)
        for (CodeDocument *doc = m_headercodedocumentVector.first(); doc; doc=m_headercodedocumentVector.next())
                delete doc;
}

//
// Methods
//

// Accessor methods
//

QString CPPCodeGenerator::getLanguage() {
        return "Cpp";
}

bool CPPCodeGenerator::isType (QString & type)
{
   if(type == "CPPCodeGenerator")
	return true;
   return false;
}

/**
 * Set the value of m_createMakefile
 * @param new_var the new value of m_createMakefile
 */
void CPPCodeGenerator::setCreateProjectMakefile ( bool buildIt) {
        m_createMakefile = buildIt;
        CodeDocument * antDoc = findCodeDocumentByID("MAKE_DOC");
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
        	tag = "cppheader"+QString::number(doc->getParentClassifier()->getID());
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
                                                        CodeViewerDialog::CodeViewerState state)
{

	ClassifierCodeDocument * cdoc = dynamic_cast<ClassifierCodeDocument*>(doc);
	if(!cdoc)
		// bah..not a classcode document?? then just use vanilla version
		return CodeGenerator::getCodeViewerDialog(parent,doc,state);
	else {
		// build with passed (source) code document
        	CodeViewerDialog *dialog = new CodeViewerDialog(parent, doc, state);

		// use classifier to find appropriate header document
		UMLClassifier * c = cdoc->getParentClassifier();
		CPPHeaderCodeDocument * hdoc = findHeaderCodeDocumentByClassifier(c);
		if(hdoc)
			dialog->addCodeDocument(hdoc);

		// add in makefile if available and desired
        	if(getCreateProjectMakefile())
                	dialog->addCodeDocument(findCodeDocumentByID("MAKE_DOC"));

        	return dialog;
	}
}

QString CPPCodeGenerator::getCPPClassName (QString name) {
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
        QPtrList<CodeDocument> docs;
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
        CodeDocument * doc = findCodeDocumentByID("cppheader"+QString::number(classifier->getID()));
	return dynamic_cast<CPPHeaderCodeDocument*>(doc);
}

/**
 * @return	ClassifierCodeDocument
 * @param	classifier
 * @param	this This package generator object.
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
 * @return	CPPMakefileCodeDocument
 * @param	this
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

	setPolicy ( new CPPCodeGenerationPolicy(this, getPolicy()) );

       // load Classifier documents from parent document
        initFromParentDocument();

        // add in an Make build document
        CPPMakefileCodeDocument * buildDoc = newMakefileCodeDocument( );
        addCodeDocument(buildDoc);

        // set our 'writeout' policy for that code document
        setCreateProjectMakefile(DEFAULT_BUILD_MAKEFILE);

}

// should be 'static'
QString CPPCodeGenerator::scopeToCPPDecl(Uml::Scope scope)
{
        QString scopeString;
        switch(scope)
        {
                case Uml::Public:
                        scopeString = "public";
                        break;
                case Uml::Protected:
                        scopeString = "protected";
                        break;
                case Uml::Private:
                default:
                        scopeString = "private";
                        break;
        }
        return scopeString;
}

void CPPCodeGenerator::createDefaultDatatypes() {
	m_document->createDatatype("int");
	m_document->createDatatype("char");
	m_document->createDatatype("bool");
	m_document->createDatatype("float");
	m_document->createDatatype("double");
	m_document->createDatatype("long");
	m_document->createDatatype("short");
}

#include "cppcodegenerator.moc"
