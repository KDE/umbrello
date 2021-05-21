/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "codegenerator.h"

// app includes
#include "debug_utils.h"
#include "overwritedialog.h"
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

#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QPointer>
#include <QRegExp>
#include <QTextStream>
#include <QXmlStreamWriter>

// system includes
#include <cstdlib>  // to get the user name

/**
 * Constructor for a code generator.
 */
CodeGenerator::CodeGenerator()
  : m_applyToAllRemaining(true),
    m_document(UMLApp::app()->document()),
    m_lastIDIndex(0)
{
    // initial population of our project generator
    // CANNOT Be done here because we would call pure virtual method
    // of newClassifierDocument (bad!).
    // We should only call from the child
    // initFromParentDocument();
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
    QString id = codeDoc->ID();

    // does this document already exist? then just return its present id
    if (!id.isEmpty() && findCodeDocumentByID(id)) {
        return id;
    }

    // approach now differs by whether or not it is a classifier code document
    ClassifierCodeDocument * classDoc = dynamic_cast<ClassifierCodeDocument*>(codeDoc);
    if (classDoc) {
        UMLClassifier *c = classDoc->getParentClassifier();
        id = Uml::ID::toString(c->id()); // this is supposed to be unique already..
    }
    else {
        QString prefix = QLatin1String("doc");
        QString id = prefix + QLatin1String("_0");
        int number = m_lastIDIndex;
        for (; findCodeDocumentByID(id); ++number) {
            id = prefix + QLatin1Char('_') + QString::number(number);
        }
        m_lastIDIndex = number;
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
        return 0;
    }
}

/**
 * Add a CodeDocument object to the m_codedocumentVector List.
 * @return boolean - will return false if it couldnt add a document
 */
bool CodeGenerator::addCodeDocument(CodeDocument * doc)
{
    QString tag = doc->ID();

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
    QString tag = remove_object->ID();
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

/**
 * Load codegenerator data from xmi.
 * @param qElement   the element from which to load
 */
void CodeGenerator::loadFromXMI1(QDomElement & qElement)
{
    // look for our particular child element
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    QString langType = Uml::ProgrammingLanguage::toString(language());

    if (qElement.tagName() != QLatin1String("codegenerator")
        || qElement.attribute(QLatin1String("language"), QLatin1String("UNKNOWN")) != langType) {
        return;
    }
    // got our code generator element, now load
    // codedocuments
    QDomNode codeDocNode = qElement.firstChild();
    QDomElement codeDocElement = codeDocNode.toElement();
    while (!codeDocElement.isNull()) {
        QString docTag = codeDocElement.tagName();
        QString id = codeDocElement.attribute(QLatin1String("id"), QLatin1String("-1"));
        if (docTag == QLatin1String("sourcecode")) {
            loadCodeForOperation(id, codeDocElement);
        }
        else if (docTag == QLatin1String("codedocument") || docTag == QLatin1String("classifiercodedocument")) {
            CodeDocument * codeDoc = findCodeDocumentByID(id);
            if (codeDoc) {
                codeDoc->loadFromXMI1(codeDocElement);
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
    Uml::ID::Type id = Uml::ID::fromString(idStr);
    UMLObject *obj = m_document->findObjectById(id);
    if (obj) {
        uDebug() << "found UMLObject for id:" << idStr;
        QString value = codeDocElement.attribute(QLatin1String("value"));

        UMLObject::ObjectType t = obj->baseType();
        if (t == UMLObject::ot_Operation) {
            UMLOperation *op = obj->asUMLOperation();
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
void CodeGenerator::saveToXMI1(QXmlStreamWriter& writer)
{
    QString langType = Uml::ProgrammingLanguage::toString(language());
    writer.writeStartElement(QLatin1String("codegenerator"));
    writer.writeAttribute(QLatin1String("language"), langType);

    if (dynamic_cast<SimpleCodeGenerator*>(this)) {
        UMLClassifierList concepts = m_document->classesAndInterfaces();
        foreach (UMLClassifier *c, concepts) {
            uIgnoreZeroPointer(c);
            UMLOperationList operations = c->getOpList();
            foreach (UMLOperation *op, operations) {
                // save the source code
                QString code = op->getSourceCode();
                if (code.isEmpty()) {
                    continue;
                }
                writer.writeStartElement(QLatin1String("sourcecode"));
                writer.writeAttribute(QLatin1String("id"), Uml::ID::toString(op->id()));
                writer.writeAttribute(QLatin1String("value"), code);
                writer.writeEndElement();
            }
        }
    }
    else {
        const CodeDocumentList * docList = getCodeDocumentList();
        CodeDocumentList::const_iterator it = docList->begin();
        CodeDocumentList::const_iterator end = docList->end();
        for (; it != end; ++it) {
            (*it)->saveToXMI1(writer);
        }
    }
    writer.writeEndElement();
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
    for (; it != end; ++it) {
        (*it)->synchronize();
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
    return findCodeDocumentByID(Uml::ID::toString(classifier->id()));
}

/**
 * This method is here to provide class wizard the
 * ability to write out only those classes which
 * are selected by the user.
 */
void CodeGenerator::writeCodeToFile()
{
    writeListedCodeDocsToFile(&m_codedocumentVector);
    finalizeRun();
}

/**
 * This method is here to provide class wizard the
 * ability to write out only those classes which
 * are selected by the user.
 */
void CodeGenerator::writeCodeToFile(UMLClassifierList & concepts)
{
    CodeDocumentList docs;

    foreach (UMLClassifier *concept, concepts) {
        CodeDocument * doc = findCodeDocumentByClassifier(concept);
        if (doc) {
            docs.append(doc);
        }
    }

    writeListedCodeDocsToFile(&docs);
    finalizeRun();
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
    for (; it != end; ++it)
    {
        // we need this so we know when to emit a 'codeGenerated' signal
        ClassifierCodeDocument * cdoc = dynamic_cast<ClassifierCodeDocument *>(*it);
        bool codeGenSuccess = false;

        // we only write the document, if so requested
        if ((*it)->getWriteOutCode()) {
            QString filename = findFileName(*it);
            // check that we may open that file for writing
            QFile file;
            if (openFile(file, filename)) {
                QTextStream stream(&file);
                stream << (*it)->toString() << endl;
                file.close();
                codeGenSuccess = true; // we wrote the code - OK
                emit showGeneratedFile(file.fileName());
            }
            else {
                uWarning() << "Cannot open file :" << file.fileName() << " for writing!";
                codeGenSuccess = false;
            }
        }

        if (cdoc) {
            emit codeGenerated(cdoc->getParentClassifier(), codeGenSuccess);
        }
    }
}

/**
 * A single call to writeCodeToFile() usually entails processing many
 * items (e.g. as classifiers) for which code is generated.
 * This method is called after all code of one call to writeCodeToFile()
 * has been generated.
 * It can be reimplemented by concrete code generators to perform additional
 * cleanups or other actions that can only be performed once all code has
 * been written.
 */
void CodeGenerator::finalizeRun()
{
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
    QPointer<OverwriteDialog> overwriteDialog =
        new OverwriteDialog(name, outputDirectory.absolutePath(),
                              m_applyToAllRemaining, qApp->activeWindow());
    switch (pol->getOverwritePolicy()) {  //if it exists, check the OverwritePolicy we should use
    case CodeGenerationPolicy::Ok:              //ok to overwrite file
        filename = name + extension;
        break;
    case CodeGenerationPolicy::Ask:            //ask if we can overwrite
        switch(overwriteDialog->exec()) {
#if QT_VERSION >= 0x050000
        case OverwriteDialog::Ok:  //overwrite file
#else
        case QDialog::Accepted:
#endif
            if (overwriteDialog->applyToAllRemaining()) {
                pol->setOverwritePolicy(CodeGenerationPolicy::Ok);
                filename = name + extension;
            }
            else {
                m_applyToAllRemaining = false;
            }
            break;
#if QT_VERSION >= 0x050000
        case OverwriteDialog::No: //generate similar name
#else
        case KDialog::No:
#endif
            suffix = 1;
            while (1) {
                filename = name + QLatin1String("__") + QString::number(suffix) + extension;
                if (!outputDirectory.exists(filename))
                    break;
                suffix++;
            }
            if (overwriteDialog->applyToAllRemaining()) {
                pol->setOverwritePolicy(CodeGenerationPolicy::Never);
            }
            else {
                m_applyToAllRemaining = false;
            }
            break;
#if QT_VERSION >= 0x050000
        case OverwriteDialog::Cancel: //don't output anything
#else
        case QDialog::Rejected:
#endif
            if (overwriteDialog->applyToAllRemaining()) {
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
            filename = name + QLatin1String("__") + QString::number(suffix) + extension;
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
        if (!outputDirectory.exists())
            outputDirectory.mkpath(outputDirectory.absolutePath());

        file.setFileName(outputDirectory.absoluteFilePath(fileName));
        if(!file.open(QIODevice::WriteOnly)) {
            KMessageBox::sorry(0, i18n("Cannot open file %1 for writing. Please make sure the folder exists and you have permissions to write to it.", file.fileName()), i18n("Cannot Open File"));
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
    retval.replace(QRegExp(QLatin1String("\\W+")), QLatin1String("_"));
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
        path.replace(QRegExp(QLatin1String("::")), QLatin1String("/")); // Simple hack!
        name = path + QLatin1Char('/') + codeDocument->getFileName();
        path = QLatin1Char('/') + path;
    }
    else {  // determine the "natural" file name
        name = codeDocument->getFileName();
    }

    // Convert all "::" to "/" : Platform-specific path separator
    name.replace(QRegExp(QLatin1String("::")), QLatin1String("/")); // Simple hack!

    // if a path name exists check the existence of the path directory
    if (!path.isEmpty()) {
        QDir outputDirectory = UMLApp::app()->commonPolicy()->getOutputDirectory();
        QDir pathDir(outputDirectory.absolutePath() + path);

        // does our complete output directory exist yet? if not, try to create it
        if (!pathDir.exists()) {
            // ugh. dir separator here is UNIX specific..
            const QStringList dirs = pathDir.absolutePath().split(QLatin1Char('/'));
            QString currentDir;

            QStringList::const_iterator end(dirs.end());
            for (QStringList::const_iterator dir(dirs.begin()); dir != end; ++dir) {
                currentDir += QLatin1Char('/') + *dir;
                if (! (pathDir.exists(currentDir) || pathDir.mkdir(currentDir))) {
                    KMessageBox::error(0, i18n("Cannot create the folder:\n") +
                                       pathDir.absolutePath() + i18n("\nPlease check the access rights"),
                                       i18n("Cannot Create Folder"));
                    return QString();
                }
            }
        }
    }

    name = name.simplified();
    name.replace(QRegExp(QLatin1String(" ")), QLatin1String("_"));

    return overwritableName(name, codeDocument->getFileExtension());
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

    foreach (UMLAssociation *a, associations) {
        temp = 0;
        switch (a->getAssocType()) {
        case Uml::AssociationType::Generalization:
        case Uml::AssociationType::Realization:
            // only the "b" end is seen by the "a" end, not other way around
            {
                UMLObject *objB = a->getObject(Uml::RoleType::B);
                if (objB != c) {
                    temp = (UMLPackage*)objB;
                }
            }
            break;
        case Uml::AssociationType::Dependency:
        case Uml::AssociationType::UniAssociation:
            {
                UMLObject *objA = a->getObject(Uml::RoleType::A);
                UMLObject *objB = a->getObject(Uml::RoleType::B);
                if (objA == c) {
                    temp = objB->asUMLPackage();
                }
            }
            break;
        case Uml::AssociationType::Aggregation:
        case Uml::AssociationType::Composition:
        case Uml::AssociationType::Association:
            {
                UMLObject *objA = a->getObject(Uml::RoleType::A);
                UMLObject *objB = a->getObject(Uml::RoleType::B);
                if (objA == c && !objB->isUMLDatatype()) {
                    temp = objB->asUMLPackage();
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
    foreach(UMLOperation *op, opl) {
        temp = 0;
        //check return value
        temp = (UMLClassifier*) op->getType();
        if (temp && !temp->isUMLDatatype() && !cList.count(temp)) {
            cList.append(temp);
        }
        //check parameters
        UMLAttributeList atl = op->getParmList();
        foreach(UMLAttribute *at, atl) {
            temp = (UMLClassifier*)at->getType();
            if (temp && !temp->isUMLDatatype() && !cList.count(temp)) {
                cList.append(temp);
            }
        }
    }

    //attributes
    if (!c->isInterface()) {
        UMLAttributeList atl = c->getAttributeList();
        foreach (UMLAttribute *at, atl) {
            temp=0;
            temp = (UMLClassifier*) at->getType();
            if (temp && !temp->isUMLDatatype() && !cList.count(temp)) {
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
        input.remove(QRegExp(QLatin1String("\\s+$")));
        if (input.length() < lineWidth) {
            output += linePrefix + input + endLine;
            continue;
        }
        int index;
        while ((index = input.lastIndexOf(QLatin1String(" "), lineWidth)) >= 0) {
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
