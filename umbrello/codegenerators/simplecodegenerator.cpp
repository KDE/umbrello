/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "simplecodegenerator.h"

// app includes
#include "overwritedialog.h"
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
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QPointer>
#include <QRegExp>

// system includes
#include <cstdlib> //to get the user name

/**
 * Constructor.
 */
SimpleCodeGenerator::SimpleCodeGenerator(bool createDirHierarchyForPackages)
  : CodeGenerator(),
    m_createDirHierarchyForPackages(createDirHierarchyForPackages),
    m_indentLevel(0)
{
    m_document->disconnect(this); // disconnect from UMLDoc.. we arent planning to be synced at all

    // load Classifier documents from parent document
    // initFromParentDocument();

    m_fileMap.clear();

    // this really is just being used to sync the internal params
    // to the codegenpolicy as there are no code documents to really sync.
    syncCodeToDocument();
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
 * @param classifier   the package
 * @param ext       the file extension
 * @return the valid file name
 */
QString SimpleCodeGenerator::findFileName(UMLPackage* classifier, const QString &ext)
{
    //if we already know to which file this class was written/should be written, just return it.
    if (m_fileMap.contains(classifier))
        return m_fileMap[classifier];

    //else, determine the "natural" file name
    QString name;
    // Get the package name
    QString package = classifier->package(QStringLiteral("."));

    // Replace all white spaces with blanks
    package = package.simplified();

    // Replace all blanks with underscore
    package.replace(QRegExp(QStringLiteral(" ")), QStringLiteral("_"));

    // Convert all "::" to "/" : Platform-specific path separator
    // package.replace(QRegExp(QStringLiteral("::")), QStringLiteral("/"));

    // if package is given add this as a directory to the file name
    if (!package.isEmpty() && m_createDirHierarchyForPackages) {
        name = package + QLatin1Char('.') + classifier->name();
        name.replace(QRegExp(QStringLiteral("\\.")), QStringLiteral("/"));
        package.replace(QRegExp(QStringLiteral("\\.")), QStringLiteral("/"));
        package = QLatin1Char('/') + package;
    } else {
        name = classifier->fullyQualifiedName(QStringLiteral("-"));
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
            const QStringList dirs = pathDir.absolutePath().split(QLatin1Char('/'));
            QString currentDir;

            QStringList::const_iterator end(dirs.end());
            for (QStringList::const_iterator dir(dirs.begin()); dir != end; ++dir)
            {
                currentDir += QLatin1Char('/') + *dir;
                if (! (pathDir.exists(currentDir)
                        || pathDir.mkdir(currentDir)))
                {
                    KMessageBox::error(0, i18n("Cannot create the folder:\n") +
                                       pathDir.absolutePath() + i18n("\nPlease check the access rights"),
                                       i18n("Cannot Create Folder"));
                    return QString();
                }
            }
        }
    }

    name = name.simplified();
    name.replace(QRegExp(QStringLiteral(" ")), QStringLiteral("_"));

    QString extension = ext.simplified();
    extension.replace(QLatin1Char(' '), QLatin1Char('_'));

    return overwritableName(classifier, name, extension);
}

/**
 * Check if a file named "name" with extension "ext" already exists.
 * @param classifier   the package
 * @param name      the name of the file
 * @param ext       the extension of the file
 * @return the valid filename or null
 */
QString SimpleCodeGenerator::overwritableName(UMLPackage* classifier, const QString &name, const QString &ext)
{
    CodeGenerationPolicy *commonPolicy = UMLApp::app()->commonPolicy();
    QDir outputDir = commonPolicy->getOutputDirectory();
    QString filename = name + ext;
    if (!outputDir.exists(filename)) {
        m_fileMap.insert(classifier, filename);
        return filename; //if not, "name" is OK and we have not much to to
    }

    int suffix;
    QPointer<OverwriteDialog> overwriteDialog =
        new OverwriteDialog(filename, outputDir.absolutePath(),
                              m_applyToAllRemaining, qApp->activeWindow());
    switch(commonPolicy->getOverwritePolicy()) {  //if it exists, check the OverwritePolicy we should use
    case CodeGenerationPolicy::Ok:                //ok to overwrite file
        break;
    case CodeGenerationPolicy::Ask:               //ask if we can overwrite
        switch(overwriteDialog->exec()) {
        case OverwriteDialog::Ok:  //overwrite file
            if (overwriteDialog->applyToAllRemaining()) {
                commonPolicy->setOverwritePolicy(CodeGenerationPolicy::Ok);
            } else {
                m_applyToAllRemaining = false;
            }
            break;
        case OverwriteDialog::No: //generate similar name
            suffix = 1;
            while (1) {
                filename = name + QStringLiteral("__") + QString::number(suffix) + ext;
                if (!outputDir.exists(filename))
                    break;
                suffix++;
            }
            if (overwriteDialog->applyToAllRemaining()) {
                commonPolicy->setOverwritePolicy(CodeGenerationPolicy::Never);
            } else {
                m_applyToAllRemaining = false;
            }
            break;
        case OverwriteDialog::Cancel: //don't output anything
            if (overwriteDialog->applyToAllRemaining()) {
                commonPolicy->setOverwritePolicy(CodeGenerationPolicy::Cancel);
            } else {
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
            filename = name + QStringLiteral("__") + QString::number(suffix) + ext;
            if (!outputDir.exists(filename))
                break;
            suffix++;
        }
        break;
    case CodeGenerationPolicy::Cancel: //don't output anything
        delete overwriteDialog;
        return QString();
        break;
    }

    m_fileMap.insert(classifier, filename);
    delete overwriteDialog;
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
    for(UMLAttribute *at : atl) {
        if (!at->getInitialValue().isEmpty())
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
    for(UMLOperation *op : opl) {
        if (op->isAbstract())
            return true;
    }
    return false;
}

/**
 * Write all concepts in project to file.
 */
void SimpleCodeGenerator::writeCodeToFile()
{
    UMLClassifierList concepts = m_document->classesAndInterfaces();
    writeCodeToFile(concepts);
}

/**
 * Write only selected concepts to file.
 * @param concepts   the selected concepts
 */
void SimpleCodeGenerator::writeCodeToFile(UMLClassifierList & concepts)
{
    m_fileMap.clear(); // ??
    for(UMLClassifier *c : concepts) {
        if (! Model_Utils::isCommonDataType(c->name()))
            this->writeClass(c); // call the writer for each class.
    }
    finalizeRun();
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

