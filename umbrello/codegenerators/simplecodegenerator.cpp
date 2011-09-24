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
#include "simplecodegenerator.h"

// app includes
#include "overwritedialogue.h"
#include "model_utils.h"
#include "attribute.h"
#include "umloperationlist.h"
#include "umlattributelist.h"
#include "classifier.h"
#include "codedocument.h"
#include "codegenerationpolicy.h"
#include "operation.h"
#include "umldoc.h"
#include "uml.h"

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

// system includes
#include <cstdlib> //to get the user name

/**
 * Constructor.
 */
SimpleCodeGenerator::SimpleCodeGenerator(bool createDirHierarchyForPackages)
{
    m_indentLevel = 0;
    UMLDoc * parentDoc = UMLApp::app()->document();
    parentDoc->disconnect(this); // disconnect from UMLDoc.. we arent planning to be synced at all
    m_createDirHierarchyForPackages = createDirHierarchyForPackages;
    initFields(parentDoc);
}

/**
 * Destructor.
 */
SimpleCodeGenerator::~SimpleCodeGenerator()
{
}

/**
 * Returns the current indent string based on m_indentLevel and m_indentation.
 * @return indentation string
 */
QString SimpleCodeGenerator::indent()
{
    QString myIndent;
    for (int i = 0 ; i < m_indentLevel; ++i) {
        myIndent.append(m_indentation);
    }
    return myIndent;
}

/**
 * Determine the file name.
 * @param concept   the package
 * @param ext       the file extension
 * @return the valid file name
 */
QString SimpleCodeGenerator::findFileName(UMLPackage* concept, const QString &ext)
{
    //if we already know to which file this class was written/should be written, just return it.
    if (m_fileMap.contains(concept))
        return m_fileMap[concept];

    //else, determine the "natural" file name
    QString name;
    // Get the package name
    QString package = concept->package(".");

    // Replace all white spaces with blanks
    package = package.simplified();

    // Replace all blanks with underscore
    package.replace(QRegExp(" "), "_");

    // Convert all "::" to "/" : Platform-specific path separator
    // package.replace(QRegExp("::"), "/");

    // if package is given add this as a directory to the file name
    if (!package.isEmpty() && m_createDirHierarchyForPackages) {
        name = package + '.' + concept->name();
        name.replace(QRegExp("\\."),"/");
        package.replace(QRegExp("\\."), "/");
        package = '/' + package;
    } else {
        name = concept->fullyQualifiedName("-");
    }

    if (! UMLApp::app()->activeLanguageIsCaseSensitive()) {
        package = package.toLower();
        name = name.toLower();
    }

    // if a package name exists check the existence of the package directory
    if (!package.isEmpty() && m_createDirHierarchyForPackages) {
        QDir pathDir(UMLApp::app()->commonPolicy()->getOutputDirectory().absolutePath() + package);
        // does our complete output directory exist yet? if not, try to create it
        if (!pathDir.exists())
        {
            const QStringList dirs = pathDir.absolutePath().split('/');
            QString currentDir = "";

            QStringList::const_iterator end(dirs.end());
            for (QStringList::const_iterator dir(dirs.begin()); dir != end; ++dir)
            {
                currentDir += '/' + *dir;
                if (! (pathDir.exists(currentDir)
                        || pathDir.mkdir(currentDir) ) )
                {
                    KMessageBox::error(0, i18n("Cannot create the folder:\n") +
                                       pathDir.absolutePath() + i18n("\nPlease check the access rights"),
                                       i18n("Cannot Create Folder"));
                    return NULL;
                }
            }
        }
    }

    name = name.simplified();
    name.replace(QRegExp(" "),"_");

    QString extension = ext.simplified();
    extension.replace(' ', '_');

    return overwritableName(concept, name, extension);
}

/**
 * Check if a file named "name" with extension "ext" already exists.
 * @param concept   the package
 * @param name      the name of the file
 * @param ext       the extension of the file
 * @return the valid filename or null
 */
QString SimpleCodeGenerator::overwritableName(UMLPackage* concept, const QString &name, const QString &ext)
{
    CodeGenerationPolicy *commonPolicy = UMLApp::app()->commonPolicy();
    QDir outputDir = commonPolicy->getOutputDirectory();
    QString filename = name + ext;
    if(!outputDir.exists(filename)) {
        m_fileMap.insert(concept,filename);
        return filename; //if not, "name" is OK and we have not much to to
    }

    int suffix;
    QPointer<OverwriteDialogue> overwriteDialogue =
        new OverwriteDialogue(filename, outputDir.absolutePath(),
                              m_applyToAllRemaining, kapp->activeWindow());
    switch(commonPolicy->getOverwritePolicy()) {  //if it exists, check the OverwritePolicy we should use
    case CodeGenerationPolicy::Ok:                //ok to overwrite file
        break;
    case CodeGenerationPolicy::Ask:               //ask if we can overwrite
        switch(overwriteDialogue->exec()) {
        case KDialog::Yes:  //overwrite file
            if ( overwriteDialogue->applyToAllRemaining() ) {
                commonPolicy->setOverwritePolicy(CodeGenerationPolicy::Ok);
            } else {
                m_applyToAllRemaining = false;
            }
            break;
        case KDialog::No: //generate similar name
            suffix = 1;
            while (1) {
                filename = name + "__" + QString::number(suffix) + ext;
                if (!outputDir.exists(filename))
                    break;
                suffix++;
            }
            if ( overwriteDialogue->applyToAllRemaining() ) {
                commonPolicy->setOverwritePolicy(CodeGenerationPolicy::Never);
            } else {
                m_applyToAllRemaining = false;
            }
            break;
        case KDialog::Cancel: //don't output anything
            if ( overwriteDialogue->applyToAllRemaining() ) {
                commonPolicy->setOverwritePolicy(CodeGenerationPolicy::Cancel);
            } else {
                m_applyToAllRemaining = false;
            }
            delete overwriteDialogue;
            return QString();
            break;
        }

        break;
    case CodeGenerationPolicy::Never: //generate similar name
        suffix = 1;
        while (1) {
            filename = name + "__" + QString::number(suffix) + ext;
            if (!outputDir.exists(filename))
                break;
            suffix++;
        }
        break;
    case CodeGenerationPolicy::Cancel: //don't output anything
        delete overwriteDialogue;
        return QString();
        break;
    }

    m_fileMap.insert(concept, filename);
    delete overwriteDialogue;
    return filename;
}

/**
 * Check whether classifier has default values for attributes.
 * @param c   the classifier to check
 * @return true when classifier attributes has default values
 */
bool SimpleCodeGenerator::hasDefaultValueAttr(UMLClassifier *c)
{
    UMLAttributeList atl = c->getAttributeList();
    foreach (UMLAttribute* at, atl ) {
        if(!at->getInitialValue().isEmpty())
            return true;
    }
    return false;
}

/**
 * Check whether classifier has abstract operations.
 * @param c   the classifier to check
 * @return true when classifier has abstract operations
 */
bool SimpleCodeGenerator::hasAbstractOps(UMLClassifier *c)
{
    UMLOperationList opl(c->getOpList());
    foreach (UMLOperation* op, opl ) {
        if(op->isAbstract())
            return true;
    }
    return false;
}

/**
 * Create a new classifier code document.
 * TODO: Not yet implemented.
 * @param classifier   UML classifier
 * @return classifier code document object
 */
CodeDocument * SimpleCodeGenerator::newClassifierCodeDocument(UMLClassifier* classifier)
{
    Q_UNUSED(classifier);
    return 0;
}

/**
 * Write all concepts in project to file.
 */
void SimpleCodeGenerator::writeCodeToFile()
{
    m_fileMap.clear(); // need to do this, else just keep getting same directory to write to.
    UMLClassifierList concepts = m_doc->classesAndInterfaces();
    foreach (UMLClassifier* c, concepts ) {
        if (! Model_Utils::isCommonDataType(c->name()))
            this->writeClass(c); // call the writer for each class.
    }
}

/**
 * Write only selected concepts to file.
 * @param concepts   the selected concepts
 */
void SimpleCodeGenerator::writeCodeToFile(UMLClassifierList & concepts)
{
    m_fileMap.clear(); // ??
    foreach (UMLClassifier* c, concepts ) {
        this->writeClass(c); // call the writer for each class.
    }
}

/**
 * Initialization of fields.
 * @param parentDoc   the parent document
 */
void SimpleCodeGenerator::initFields(UMLDoc * parentDoc)
{
    // load Classifier documents from parent document
    // initFromParentDocument();

    m_fileMap.clear();
    m_applyToAllRemaining = true;
    m_doc = parentDoc;

    // this really is just being used to sync the internal params
    // to the codegenpolicy as there are no code documents to really sync.
    syncCodeToDocument();
}

/**
 * A little method to provide some compatibility between
 * the newer codegenpolicy object and the older class fields.
 */
void SimpleCodeGenerator::syncCodeToDocument()
{
    CodeGenerationPolicy *policy = UMLApp::app()->commonPolicy();

    m_indentation = policy->getIndentation();
    m_endl = policy->getNewLineEndingChars();
}

/**
 * Override parent method.
 */
void SimpleCodeGenerator::initFromParentDocument()
{
    // do nothing
}

#include "simplecodegenerator.moc"
