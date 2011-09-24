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
#include "codegenerator.h"

// app includes
#include "debug_utils.h"
#include "overwritedialogue.h"
#include "codeviewerdialog.h"
#include "simplecodegenerator.h"
#include "attribute.h"
#include "association.h"
#include "classifier.h"
#include "classifiercodedocument.h"
#include "codedocument.h"
#include "codegenerationpolicy.h"
#include "operation.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlattributelist.h"
#include "umloperationlist.h"

// kde includes
#include <klocale.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kapplication.h>

// qt includes
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QPointer>
#include <QtCore/QRegExp>
#include <QtCore/QTextStream>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

// system includes
#include <cstdlib>  // to get the user name

/**
 * Constructor for a code generator.
 */
CodeGenerator::CodeGenerator()
  : QObject(UMLApp::app()->document())
{
    initFields();
}

/**
 * Constructor for a code generator and then initialize it from an XMI element.
 * FIX: hmm. this should be pure virtual so that implemented in sub-class.
 * @param element   an element from an XMI document
 */
CodeGenerator::CodeGenerator(QDomElement & element)
  : QObject(UMLApp::app()->document())
{
    initFields();
    loadFromXMI(element);  // hmm. cannot call this here.. it is 'pure' virtual
}

/**
 * Destructor.
 */
CodeGenerator::~CodeGenerator()
{
    // destroy all owned codedocuments
    qDeleteAll(m_codedocumentVector);
    m_codedocumentVector.clear();
}

/**
 * Get a unique id for this codedocument.
 * @return   id for the codedocument
 */
QString CodeGenerator::getUniqueID(CodeDocument * codeDoc)
{
    QString id = codeDoc->getID();

    // does this document already exist? then just return its present id
    if (!id.isEmpty() && findCodeDocumentByID(id)) {
        return id;
    }

    // approach now differs by whether or not it is a classifier code document
    ClassifierCodeDocument * classDoc = dynamic_cast<ClassifierCodeDocument*>(codeDoc);
    if (classDoc) {
        UMLClassifier *c = classDoc->getParentClassifier();
        id = ID2STR(c->id()); // this is supposed to be unique already..
    }
    else {
        QString prefix = "doc";
        QString id = prefix + "_0";
        int number = lastIDIndex;
        for ( ; findCodeDocumentByID(id); ++number) {
            id = prefix + '_' + QString::number(number);
        }
        lastIDIndex = number;
    }

    return id;
}

/**
 * Find a code document by the given id string.
 * @return  CodeDocument
 */
CodeDocument * CodeGenerator::findCodeDocumentByID(const QString &tag)
{
    CodeDocument* doc = m_codeDocumentDictionary.value(tag);
    if (doc) {
        return doc;
    }
    else {
        return NULL;
    }
}

/**
 * Add a CodeDocument object to the m_codedocumentVector List.
 * @return boolean - will return false if it couldnt add a document
 */
bool CodeGenerator::addCodeDocument(CodeDocument * doc)
{
    QString tag = doc->getID();

    // assign a tag if one doesn't already exist
    if (tag.isEmpty()) {
        tag = getUniqueID(doc);
        doc->setID(tag);
    }

    if (m_codeDocumentDictionary.contains(tag)) {
        return false;  // return false, we already have some object with this tag in the list
    }
    else {
        m_codeDocumentDictionary.insert(tag, doc);
    }

    m_codedocumentVector.append(doc);
    return true;
}

/**
 * Remove a CodeDocument object from m_codedocumentVector List.
 * @return boolean - will return false if it couldnt remove a document
 */
bool CodeGenerator::removeCodeDocument(CodeDocument * remove_object)
{
    QString tag = remove_object->getID();
    if (!(tag.isEmpty())) {
        m_codeDocumentDictionary.remove(tag);
    }
    else {
        return false;
    }

    m_codedocumentVector.removeAll(remove_object);
    return true;
}

/**
 * Get the list of CodeDocument objects held by m_codedocumentVector.
 * @return CodeDocumentList list of CodeDocument objects held by
 * m_codedocumentVector
 */
CodeDocumentList * CodeGenerator::getCodeDocumentList()
{
    return &m_codedocumentVector;
}

// the vanilla version

/**
 * Get the editing dialog for this code document.
 */
CodeViewerDialog * CodeGenerator::getCodeViewerDialog(QWidget* parent, CodeDocument *doc,
        Settings::CodeViewerState state)
{
    return new CodeViewerDialog(parent, doc, state);
}

/**
 * Load codegenerator data from xmi.
 * @param qElement   the element from which to load
 */
void CodeGenerator::loadFromXMI(QDomElement & qElement)
{
    // look for our particular child element
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    QString langType = Uml::ProgrammingLanguage::toString( language() );

    if (qElement.tagName() != "codegenerator"
            || qElement.attribute("language", "UNKNOWN") != langType) {
        return;
    }
    // got our code generator element, now load
    // codedocuments
    QDomNode codeDocNode = qElement.firstChild();
    QDomElement codeDocElement = codeDocNode.toElement();
    while (!codeDocElement.isNull()) {
        QString docTag = codeDocElement.tagName();
        QString id = codeDocElement.attribute( "id", "-1" );
        if (docTag == "sourcecode") {
            loadCodeForOperation(id, codeDocElement);
        }
        else if (docTag == "codedocument" || docTag == "classifiercodedocument") {
            CodeDocument * codeDoc = findCodeDocumentByID(id);
            if (codeDoc) {
                codeDoc->loadFromXMI(codeDocElement);
            }
            else {
                uWarning() << "missing code document for id:" << id;
            }
        }
        else {
            uWarning() << "got strange codegenerator child node:" << docTag << ", ignoring.";
        }
        codeDocNode = codeDocElement.nextSibling();
        codeDocElement = codeDocNode.toElement();
    }
}

/**
 * Extract and load code for operations from xmi section.
 * Probably we have code which was entered in classpropdlg for an operation.
 */
void CodeGenerator::loadCodeForOperation(const QString& idStr, const QDomElement& codeDocElement)
{
    Uml::IDType id = STR2ID(idStr);
    UMLObject *obj = m_document->findObjectById(id);
    if (obj) {
        uDebug() << "found UMLObject for id:" << idStr;
        QString value = codeDocElement.attribute("value", "");

        UMLObject::ObjectType t = obj->baseType();
        if (t == UMLObject::ot_Operation) {
            UMLOperation *op = static_cast<UMLOperation*>(obj);
            op->setSourceCode(value);
        }
        else {
            uError() << "sourcecode id " << idStr << " has unexpected type " << UMLObject::toString(t);
        }
    }
    else {
        uError() << "unknown sourcecode id " << idStr;
    }
}

/**
 * Save the XMI representation of this object
 */
void CodeGenerator::saveToXMI(QDomDocument & doc, QDomElement & root)
{
    QString langType = Uml::ProgrammingLanguage::toString( language() );
    QDomElement docElement = doc.createElement( "codegenerator" );
    docElement.setAttribute("language",langType);

    if (dynamic_cast<SimpleCodeGenerator*>(this)) {
        UMLClassifierList concepts = m_document->classesAndInterfaces();
        foreach (UMLClassifier *c, concepts) {
            UMLOperationList operations = c->getOpList();
            foreach (UMLOperation *op, operations) {
                // save the source code
                QString code = op->getSourceCode();
                if (code.isEmpty()) {
                    continue;
                }
                QDomElement codeElement = doc.createElement("sourcecode");
                codeElement.setAttribute("id", ID2STR(op->id()));
                codeElement.setAttribute("value", code);
                docElement.appendChild( codeElement );
            }
        }
    }
    else {
        const CodeDocumentList * docList = getCodeDocumentList();
        CodeDocumentList::const_iterator it = docList->begin();
        CodeDocumentList::const_iterator end = docList->end();
        for ( ; it != end; ++it ) {
            (*it)->saveToXMI(doc, docElement);
        }
    }
    root.appendChild( docElement );
}

/**
 * Initialize this code generator from its parent UMLDoc. When this is called,
 * it will (re-)generate the list of code documents for this project (generator)
 * by checking for new objects/attributes which have been added or changed in the
 * document. One or more CodeDocuments will be created/overwritten/amended as is
 * appropriate for the given language.
 * <p>
 * In this 'generic' version a ClassifierCodeDocument will exist for each and
 * every classifier that exists in our UMLDoc. IF when this is called, a code document
 * doesn't exist for the given classifier, then we will created and add a new code
 * document to our generator.
 * <p>
 * IF you want to add non-classifier related code documents at this step,
 * you will need to overload this method in the appropriate
 * code generatator (see JavaCodeGenerator for an example of this).
 */
void CodeGenerator::initFromParentDocument()
{
    // Walk through the document converting classifiers into
    // classifier code documents as needed (e.g only if doesn't exist)
    UMLClassifierList concepts = m_document->classesAndInterfaces();
    foreach (UMLClassifier *c , concepts) {
        // Doesn't exist? Then build one.
        CodeDocument * codeDoc = findCodeDocumentByClassifier(c);
        if (!codeDoc) {
            codeDoc = newClassifierCodeDocument(c);
            addCodeDocument(codeDoc); // this will also add a unique tag to the code document
        }
    }
}

/**
 * Force a synchronize of this code generator, and its present contents, to that of the parent UMLDocument.
 * "UserGenerated" code will be preserved, but Autogenerated contents will be updated/replaced
 * or removed as is apppropriate.
 */
void CodeGenerator::syncCodeToDocument()
{
    CodeDocumentList::iterator it = m_codedocumentVector.begin();
    CodeDocumentList::iterator end = m_codedocumentVector.end();
    for ( ; it != end; ++it ) {
        (*it)->synchronize();
    }
}

// in this 'vanilla' version, we only worry about adding classifier
// documents

/** These 2 functions check for adding or removing objects to the UMLDocument */
void CodeGenerator::checkAddUMLObject(UMLObject * obj)
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

void CodeGenerator::checkRemoveUMLObject(UMLObject * obj)
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
 * Find a code document by the given classifier.
 * NOTE: FIX, this should be 'protected' or we could have problems with CPP code generator
 * @return      CodeDocument
 * @param       classifier
 */
CodeDocument * CodeGenerator::findCodeDocumentByClassifier(UMLClassifier * classifier)
{
    return findCodeDocumentByID(ID2STR(classifier->id()));
}

/**
 * This method is here to provide class wizard the
 * ability to write out only those classes which
 * are selected by the user.
 */
void CodeGenerator::writeCodeToFile()
{
    writeListedCodeDocsToFile(&m_codedocumentVector);
}

/**
 * This method is here to provide class wizard the
 * ability to write out only those classes which
 * are selected by the user.
 */
void CodeGenerator::writeCodeToFile(UMLClassifierList & concepts)
{
    CodeDocumentList docs;

    foreach (UMLClassifier *concept, concepts ) {
        CodeDocument * doc = findCodeDocumentByClassifier(concept);
        if (doc) {
            docs.append(doc);
        }
    }

    writeListedCodeDocsToFile(&docs);
}

// Main method. Will write out passed code documents to file as appropriate.

/**
 * The actual internal routine which writes code documents.
 */
void CodeGenerator::writeListedCodeDocsToFile(CodeDocumentList * docs)
{
    // iterate thru all code documents
    CodeDocumentList::iterator it = docs->begin();
    CodeDocumentList::iterator end = docs->end();
    for ( ; it != end; ++it )
    {
        // we need this so we know when to emit a 'codeGenerated' signal
        ClassifierCodeDocument * cdoc = dynamic_cast<ClassifierCodeDocument *>(*it);
        bool codeGenSuccess = false;

        // we only write the document, if so requested
        if ((*it)->getWriteOutCode()) {
            QString filename = findFileName(*it);
            // check that we may open that file for writing
            QFile file;
            if ( openFile(file,filename) ) {
                QTextStream stream(&file);
                stream << (*it)->toString() << endl;
                file.close();
                codeGenSuccess = true; // we wrote the code OK
            }
            else {
                uWarning() << "Cannot open file :"<<filename<<" for writing ";
            }
        }

        if (cdoc) {
            emit codeGenerated(cdoc->getParentClassifier(), codeGenSuccess);
        }
    }
}

/**
 * Create a new Code document belonging to this package.
 * @return CodeDocument   pointer to new code document.
 */
CodeDocument * CodeGenerator::newCodeDocument()
{
    CodeDocument * newCodeDoc = new CodeDocument();
    return newCodeDoc;
}

/**
 *  Gets the heading file (as a string) to be inserted at the
 *  beginning of the generated file. you give the file type as
 *  parameter and get the string. if fileName starts with a
 *  period (.) then fileName is the extension (.cpp, .h,
 *  .java) if fileName starts with another character you are
 *  requesting a specific file (mylicensefile.txt).  The files
 *  can have parameters which are denoted by %parameter%.
 *
 *  current parameters are
 *  %author%
 *  %date%
 *  %time%
 *  %filepath%
 *
 * @return      QString
 * @param       file
 */
QString CodeGenerator::getHeadingFile(const QString &file)
{
    return UMLApp::app()->commonPolicy()->getHeadingFile(file);
}

/**
 * Returns a name that can be written to in the output directory,
 * respecting the overwrite policy.
 * If a file of the given name and extension does not exist,
 * then just returns the name.
 * If a file of the given name and extension does exist,
 * then opens an overwrite dialog. In this case the name returned
 * may be a modification of the input name.
 * This method is invoked by findFileName().
 *
 * @param name the proposed output file name
 * @param extension the extension to use
 * @return the real file name that should be used (including extension) or
 *      QString() if none to be used
 */
QString CodeGenerator::overwritableName(const QString& name, const QString &extension)
{
    CodeGenerationPolicy *pol = UMLApp::app()->commonPolicy();
    QDir outputDirectory = pol->getOutputDirectory();
    QString filename = name + extension;

    if (!outputDirectory.exists(filename)) {
        return filename;
    }

    int suffix;
    QPointer<OverwriteDialogue> overwriteDialog =
        new OverwriteDialogue(name, outputDirectory.absolutePath(),
                              m_applyToAllRemaining, kapp->activeWindow());
    switch (pol->getOverwritePolicy()) {  //if it exists, check the OverwritePolicy we should use
    case CodeGenerationPolicy::Ok:              //ok to overwrite file
        filename = name + extension;
        break;
    case CodeGenerationPolicy::Ask:            //ask if we can overwrite
        switch(overwriteDialog->exec()) {
        case KDialog::Yes:  //overwrite file
            if ( overwriteDialog->applyToAllRemaining() ) {
                pol->setOverwritePolicy(CodeGenerationPolicy::Ok);
                filename = name + extension;
            }
            else {
                m_applyToAllRemaining = false;
            }
            break;
        case KDialog::No: //generate similar name
            suffix = 1;
            while (1) {
                filename = name + "__" + QString::number(suffix) + extension;
                if (!outputDirectory.exists(filename))
                    break;
                suffix++;
            }
            if ( overwriteDialog->applyToAllRemaining() ) {
                pol->setOverwritePolicy(CodeGenerationPolicy::Never);
            }
            else {
                m_applyToAllRemaining = false;
            }
            break;
        case KDialog::Cancel: //don't output anything
            if ( overwriteDialog->applyToAllRemaining() ) {
                pol->setOverwritePolicy(CodeGenerationPolicy::Cancel);
            }
            else {
                m_applyToAllRemaining = false;
            }
            delete overwriteDialog;
            return QString();
            break;
        }

        break;
    case CodeGenerationPolicy::Never: //generate similar name
        suffix = 1;
        while (1) {
            filename = name + "__" + QString::number(suffix) + extension;
            if (!outputDirectory.exists(filename)) {
                break;
            }
            suffix++;
        }
        break;
    case CodeGenerationPolicy::Cancel: //don't output anything
        delete overwriteDialog;
        return QString();
        break;
    }

    delete overwriteDialog;
    return filename;
}

/**
 * Opens a file named "name" for writing in the outputDirectory.
 * If something goes wrong, it informs the user
 * if this function returns true, you know you can write to the file.
 * @param file       file descriptor
 * @param fileName   the name of the file
 * @return           success state
 */
bool CodeGenerator::openFile(QFile & file, const QString &fileName)
{
    //open files for writing.
    if (fileName.isEmpty()) {
        uWarning() << "cannot find a file name";
        return false;
    }
    else {
        QDir outputDirectory = UMLApp::app()->commonPolicy()->getOutputDirectory();
        file.setFileName(outputDirectory.absoluteFilePath(fileName));
        if(!file.open(QIODevice::WriteOnly)) {
            KMessageBox::sorry(0,i18n("Cannot open file %1 for writing. Please make sure the folder exists and you have permissions to write to it.", file.fileName()),i18n("Cannot Open File"));
            return false;
        }
        return true;
    }
}

/**
 * Replaces spaces with underscores and capitalises as defined in m_modname
 * @return      QString
 * @param       name
 */
QString CodeGenerator::cleanName(const QString &name)
{
    QString retval = name;
    retval.replace(QRegExp("\\W+"), "_");
    return retval;
}

/**
 * Finds an appropriate file name for the given CodeDocument, taking into
 * account the Overwrite Policy and asking the user what to do if need be
 * (if policy == Ask).
 *
 * @param codeDocument  the CodeDocument for which an output file name is desired.
 * @return the file name that should be used. (with extension) or
 *      NULL if none to be used
 */
QString CodeGenerator::findFileName(CodeDocument * codeDocument)
{
    // Get the path name
    QString path = codeDocument->getPath();

    // if path is given add this as a directory to the file name
    QString name;
    if (!path.isEmpty()) {
        path.replace(QRegExp("::"), "/"); // Simple hack!
        name = path + '/' + codeDocument->getFileName();
        path = '/' + path;
    }
    else {  // determine the "natural" file name
        name = codeDocument->getFileName();
    }

    // Convert all "::" to "/" : Platform-specific path separator
    name.replace(QRegExp("::"), "/"); // Simple hack!

    // if a path name exists check the existence of the path directory
    if (!path.isEmpty()) {
        QDir outputDirectory = UMLApp::app()->commonPolicy()->getOutputDirectory();
        QDir pathDir(outputDirectory.absolutePath() + path);

        // does our complete output directory exist yet? if not, try to create it
        if (!pathDir.exists()) {
            // ugh. dir separator here is UNIX specific..
            const QStringList dirs = pathDir.absolutePath().split('/');
            QString currentDir;

            QStringList::const_iterator end(dirs.end());
            for (QStringList::const_iterator dir(dirs.begin()); dir != end; ++dir) {
                currentDir += '/' + *dir;
                if (! (pathDir.exists(currentDir) || pathDir.mkdir(currentDir))) {
                    KMessageBox::error(0, i18n("Cannot create the folder:\n") +
                                       pathDir.absolutePath() + i18n("\nPlease check the access rights"),
                                       i18n("Cannot Create Folder"));
                    return NULL;
                }
            }
        }
    }

    name.simplified();
    name.replace(QRegExp(" "),"_");

    return overwritableName( name, codeDocument->getFileExtension() );
}

/**
 * Finds all classes in the current document to which objects of class c
 * are in some way related. Possible relations are Associations (generalization,
 * composition, etc) as well as parameters to methods and return values
 * this is useful in deciding which classes/files to import/include in code generation
 * @param c the class for which relations are to be found
 * @param cList a reference to the list into which return the result
 */
void CodeGenerator::findObjectsRelated(UMLClassifier *c, UMLPackageList &cList)
{
    UMLPackage *temp;
    UMLAssociationList associations = c->getAssociations();

    foreach (UMLAssociation *a , associations) {
        temp = 0;
        switch (a->getAssocType()) {
        case Uml::AssociationType::Generalization:
        case Uml::AssociationType::Realization:
            // only the "b" end is seen by the "a" end, not other way around
            {
                UMLObject *objB = a->getObject(Uml::B);
                if (objB != c) {
                    temp = (UMLPackage*)objB;
                }
            }
            break;
        case Uml::AssociationType::Dependency:
        case Uml::AssociationType::UniAssociation:
            {
                UMLObject *objA = a->getObject(Uml::A);
                UMLObject *objB = a->getObject(Uml::B);
                if (objA == c) {
                    temp = static_cast<UMLPackage*>(objB);
                }
            }
            break;
        case Uml::AssociationType::Aggregation:
        case Uml::AssociationType::Composition:
        case Uml::AssociationType::Association:
            {
                UMLObject *objA = a->getObject(Uml::A);
                UMLObject *objB = a->getObject(Uml::B);
                if (objA == c && objB->baseType() != UMLObject::ot_Datatype) {
                    temp = static_cast<UMLPackage*>(objB);
                }
            }
            break;
        default: // all others.. like for state diagrams..we currently don't use
            break;
        }

        // now add in list ONLY if it is not already there
        if (temp  && !cList.count(temp)) {
            cList.append(temp);
        }
    }

    //operations
    UMLOperationList opl(c->getOpList());
    foreach(UMLOperation *op , opl ) {
        temp = 0;
        //check return value
        temp = (UMLClassifier*) op->getType();
        if (temp && temp->baseType() != UMLObject::ot_Datatype && !cList.count(temp)) {
            cList.append(temp);
        }
        //check parameters
        UMLAttributeList atl = op->getParmList();
        foreach(UMLAttribute *at , atl) {
            temp = (UMLClassifier*)at->getType();
            if (temp && temp->baseType() != UMLObject::ot_Datatype && !cList.count(temp)) {
                cList.append(temp);
            }
        }
    }

    //attributes
    if (!c->isInterface()) {
        UMLAttributeList atl = c->getAttributeList();
        foreach (UMLAttribute *at , atl ) {
            temp=0;
            temp = (UMLClassifier*) at->getType();
            if (temp && temp->baseType() != UMLObject::ot_Datatype && !cList.count(temp)) {
                cList.append(temp);
            }
        }
    }
}

/**
 * Format documentation for output in source files
 *
 * @param text         the documentation which has to be formatted
 * @param linePrefix   the prefix which has to be added in the beginnig of each line
 * @param lineWidth    the line width used for word-wrapping the documentation
 *
 * @return the formatted documentation text
 */
QString CodeGenerator::formatDoc(const QString &text, const QString &linePrefix, int lineWidth)
{
    const QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();
    QString output;
    QStringList lines = text.split(endLine);
    for (QStringList::ConstIterator lit = lines.constBegin(); lit != lines.constEnd(); ++lit) {
        QString input = *lit;
        input.remove( QRegExp("\\s+$") );
        if (input.length() < lineWidth) {
            output += linePrefix + input + endLine;
            continue;
        }
        int index;
        while ((index = input.lastIndexOf(" ", lineWidth)) >= 0) {
            output += linePrefix + input.left(index) + endLine; // add line
            input.remove(0, index + 1); // remove processed string, including white space
        }
        if (!input.isEmpty()) {
            output += linePrefix + input + endLine;
        }
    }
    return output;
}

/**
 * Format source code for output in source files by
 * adding the correct indentation to every line of code.
 *
 * @param code          the source code block which has to be formatted
 * @param indentation   the blanks to indent
 */
QString CodeGenerator::formatSourceCode(const QString& code, const QString& indentation)
{
    const QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();
    QString output;
    if (! code.isEmpty()) {
        QStringList lines = code.split(endLine);
        for (int i = 0; i < lines.size(); ++i) {
            output += indentation + lines.at(i) + endLine;
        }
    }
    return output;
}

void CodeGenerator::initFields()
{
    m_document = UMLApp::app()->document();
    m_applyToAllRemaining = true;
    lastIDIndex = 0;

    // initial population of our project generator
    // CANT Be done here because we would call pure virtual method
    // of newClassifierDocument (bad!).
    // We should only call from the child
    // initFromParentDocument();
}

/**
 * Connect additional slots.
 * Only required for Advanced Code Generators.
 * To be called after constructing the code generator (see CodeGenFactory).
 */
void CodeGenerator::connect_newcodegen_slots()
{
    connect(m_document, SIGNAL(sigObjectCreated(UMLObject*)),
            this, SLOT(checkAddUMLObject(UMLObject*)));
    connect(m_document, SIGNAL(sigObjectRemoved(UMLObject*)),
            this, SLOT(checkRemoveUMLObject(UMLObject*)));
    CodeGenerationPolicy *commonPolicy = UMLApp::app()->commonPolicy();
    connect(commonPolicy, SIGNAL(modifiedCodeContent()),
            this, SLOT(syncCodeToDocument()));
}

// these are utility methods for accessing the default
// code gen policy object and should go away when we
// finally implement the CodeGenDialog class -b.t.

void CodeGenerator::setForceDoc(bool f)
{
    UMLApp::app()->commonPolicy()->setCodeVerboseDocumentComments(f);
}

bool CodeGenerator::forceDoc() const
{
    return UMLApp::app()->commonPolicy()->getCodeVerboseDocumentComments();
}

void CodeGenerator::setForceSections(bool f)
{
    UMLApp::app()->commonPolicy()->setCodeVerboseSectionComments(f);
}

bool CodeGenerator::forceSections() const
{
    return UMLApp::app()->commonPolicy()->getCodeVerboseSectionComments();
}

/**
 * Return the default datatypes for your language (bool, int etc).
 * Default implementation returns empty list.
 */
QStringList CodeGenerator::defaultDatatypes()
{
    return QStringList();
    //empty by default, override in your code generator
}

/**
 * Check whether the given string is a reserved word for the
 * language of this code generator.
 *
 * @param keyword   string to check
 *
 */
bool CodeGenerator::isReservedKeyword(const QString & keyword)
{
    const QStringList keywords = reservedKeywords();
    return keywords.contains(keyword);
}

/**
 * Get list of reserved keywords.
 */
QStringList CodeGenerator::reservedKeywords() const
{
    static QStringList emptyList;
    return emptyList;
}

/**
 * Create the default stereotypes for your language (constructor, int etc).
 */
void CodeGenerator::createDefaultStereotypes()
{
    //empty by default, override in your code generator
    //e.g.  m_document->createDefaultStereotypes("constructor");
}

#include "codegenerator.moc"
