/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "codegenerationpolicy.h"

// app includes
#include "codegenerationpolicypage.h"
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umbrellosettings.h"

// kde includes
#include <kconfig.h>
#if QT_VERSION < 0x050000
#include <kstandarddirs.h>
#endif

// qt includes
#include <QDateTime>
#include <QRegExp>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#endif
#include <QStringList>
#include <QTextStream>

using namespace std;

#define MAXLINES 256

/**
 * Constructor.
 * @param clone   generation policy to clone
 */
CodeGenerationPolicy::CodeGenerationPolicy(CodeGenerationPolicy * clone)
{
    // first call the function which can give us values from disk, so that we have something to fall back on
    setDefaults(false);
    // then set the values from the object passed.
    setDefaults(clone, false);
}

/**
 * Constructor.
 */
CodeGenerationPolicy::CodeGenerationPolicy()
{
    setDefaults(false);
}

/**
 * Destructor
 */
CodeGenerationPolicy::~CodeGenerationPolicy()
{
}

/**
 * Set the value of m_overwritePolicy
 * Policy of how to deal with overwriting existing files. Allowed values are "ask",
 * "yes" and "no".
 * @param new_var the new value of m_overwritePolicy
 */
void CodeGenerationPolicy::setOverwritePolicy (OverwritePolicy new_var)
{
    Settings::optionState().codeGenerationState.overwritePolicy = new_var;
}

/**
 * Get the value of m_overwritePolicy
 * Policy of how to deal with overwriting existing files. Allowed values are "ask",
 * "yes" and "no".
 * @return the overwrite policy
 */
CodeGenerationPolicy::OverwritePolicy CodeGenerationPolicy::getOverwritePolicy () const
{
    return Settings::optionState().codeGenerationState.overwritePolicy;
}

/**
 * Set the value of m_commentStyle
 * @param new_var the new value of m_commentStyle
 */
void CodeGenerationPolicy::setCommentStyle (CommentStyle new_var)
{
    Settings::optionState().codeGenerationState.commentStyle = new_var;
    emit modifiedCodeContent();
}

/**
 * Get the value of m_commentStyle
 * @return the comment style
 */
CodeGenerationPolicy::CommentStyle CodeGenerationPolicy::getCommentStyle()
{
    return Settings::optionState().codeGenerationState.commentStyle;
}

/**
 * Set the value of Settings::optionState().codeGenerationState.writeSectionComments
 * Whether or not verbose code commenting for sections is desired.
 * If the value is Always, comments for sections will be written even if the section
 * is empty.
 * @param new_var the new WriteSectionCommentsPolicy to set
 */
void CodeGenerationPolicy::setSectionCommentsPolicy (WriteSectionCommentsPolicy new_var)
{
    Settings::optionState().codeGenerationState.writeSectionComments = new_var;
    emit modifiedCodeContent();
}

/**
 * Get the value of Settings::optionState().codeGenerationState.writeSectionComments
 * Whether or not verbose code commenting for sections is desired.
 * If the value is Always, comments for sections will be written even if the section
 * is empty.
 * @return the flag whether verbose code commenting for sections is desired
 */
CodeGenerationPolicy::WriteSectionCommentsPolicy
CodeGenerationPolicy::getSectionCommentsPolicy () const
{
    return Settings::optionState().codeGenerationState.writeSectionComments;
}

/**
 * Set the value of m_codeVerboseDocumentComments
 * Whether or not verbose code commenting for documentation is desired. If true,
 * documentation for various code will be written even if no code would normally be
 * created at that point in the file.
 * @param new_var the new value to set verbose code commenting
 */
void CodeGenerationPolicy::setCodeVerboseDocumentComments (bool new_var)
{
    Settings::optionState().codeGenerationState.forceDoc = new_var;
    emit modifiedCodeContent();
}

/**
 * Get the value of m_codeVerboseDocumentComments
 * Whether or not verbose code commenting for documentation is desired. If true,
 * documentation for various code will be written even if no code would normally be
 * created at that point in the file.
 * @return the value of m_codeVerboseDocumentComments
 */
bool CodeGenerationPolicy::getCodeVerboseDocumentComments () const
{
    return Settings::optionState().codeGenerationState.forceDoc;
}

/**
 * Set the value of m_headingFileDir
 * location of the header file template.
 * @param path   the new value of m_headingFileDir
 */
void CodeGenerationPolicy::setHeadingFileDir (const QString & path)
{
    Settings::optionState().codeGenerationState.headingsDirectory.setPath(path);
}

/**
 * Get the value of m_headingFileDir
 * location of the header file template.
 * @return the value of m_headingFileDir
 */
QString CodeGenerationPolicy::getHeadingFileDir () const
{
    return Settings::optionState().codeGenerationState.headingsDirectory.absolutePath();
}

/**
 * Set the value of m_includeHeadings
 * @param new_var the new value of m_includeHeadings
 */
void CodeGenerationPolicy::setIncludeHeadings (bool new_var)
{
    Settings::optionState().codeGenerationState.includeHeadings = new_var;
    emit modifiedCodeContent();
}

/**
 * Get the value of m_includeHeadings
 * @return the value of m_includeHeadings
 */
bool CodeGenerationPolicy::getIncludeHeadings () const
{
    return Settings::optionState().codeGenerationState.includeHeadings;
}

/**
 * Set the value of m_outputDirectory
 * location of where output files will go.
 * @param new_var the new value of m_outputDirectory
 */
void CodeGenerationPolicy::setOutputDirectory (QDir new_var)
{
    Settings::optionState().codeGenerationState.outputDirectory = new_var;
}

/**
 * Get the value of m_outputDirectory
 * location of where output files will go.
 * @return the value of m_outputDirectory
 */
QDir CodeGenerationPolicy::getOutputDirectory ()
{
    return Settings::optionState().codeGenerationState.outputDirectory;
}

/**
 * Set the value of m_lineEndingType
 * What line ending characters to use.
 * @param type   the new value of m_lineEndingType
 */
void CodeGenerationPolicy::setLineEndingType (NewLineType type)
{
    Settings::optionState().codeGenerationState.lineEndingType = type;
    switch (Settings::optionState().codeGenerationState.lineEndingType) {
    case DOS:
        m_lineEndingChars = QString(QStringLiteral("\r\n"));
        break;
    case MAC:
        m_lineEndingChars = QString(QStringLiteral("\r"));
        break;
    case UNIX:
    default:
        m_lineEndingChars = QString(QStringLiteral("\n"));
        break;
    }
    emit modifiedCodeContent();
}

/**
 * Get the value of m_lineEndingType
 * What line ending characters to use.
 * @return the value of m_lineEndingType
 */
CodeGenerationPolicy::NewLineType CodeGenerationPolicy::getLineEndingType ()
{
    return Settings::optionState().codeGenerationState.lineEndingType;
}

/**
 * Utility function to get the actual characters.
 * @return   the line ending characters
 */
QString CodeGenerationPolicy::getNewLineEndingChars () const
{
    return m_lineEndingChars;
}

/**
 * Set the value of m_indentationType
 * The amount and type of whitespace to indent with.
 * @param new_var the new value of m_indentationType
 */
void CodeGenerationPolicy::setIndentationType (IndentationType new_var)
{
    Settings::optionState().codeGenerationState.indentationType = new_var;
    calculateIndentation();
    emit modifiedCodeContent();
}

/**
 * Get the value of m_indentationType
 */
CodeGenerationPolicy::IndentationType CodeGenerationPolicy::getIndentationType ()
{
    return Settings::optionState().codeGenerationState.indentationType;
}

/**
 * Set how many units to indent for each indentation level.
 * @param amount   the amount of indentation units
 */
void CodeGenerationPolicy::setIndentationAmount (int amount)
{
    if (amount > -1)
    {
        Settings::optionState().codeGenerationState.indentationAmount = amount;
        calculateIndentation();
        emit modifiedCodeContent();
    }
}

/**
 * Get indentation level units.
 */
int CodeGenerationPolicy::getIndentationAmount ()
{
    return Settings::optionState().codeGenerationState.indentationAmount;
}

/**
 * Utility method to get the amount (and type of whitespace) to indent with.
 * @return the value of the indentation
 */
QString CodeGenerationPolicy::getIndentation () const
{
    return m_indentation;
}

/**
 * Calculate the indentation.
 */
void CodeGenerationPolicy::calculateIndentation ()
{
    QString indent;
    m_indentation.clear();
    switch (Settings::optionState().codeGenerationState.indentationType) {
    case NONE:
        break;
    case TAB:
        indent = QString(QStringLiteral("\t"));
        break;
    default:
    case SPACE:
        indent = QString(QStringLiteral(" "));
        break;
    }

    for (int i = 0; i < Settings::optionState().codeGenerationState.indentationAmount; ++i) {
        m_indentation += indent;
    }
}

/**
 * Set the value of m_modifyPolicy
 * @param new_var the new value of m_modifyPolicy
 */
void CodeGenerationPolicy::setModifyPolicy (ModifyNamePolicy new_var)
{
    Settings::optionState().codeGenerationState.modnamePolicy = new_var;
}

/**
 * Get the value of m_modifyPolicy
 * @return the value of m_modifyPolicy
 */
CodeGenerationPolicy::ModifyNamePolicy CodeGenerationPolicy::getModifyPolicy () const
{
    return Settings::optionState().codeGenerationState.modnamePolicy;
}

/**
 * Set the value of m_autoGenerateConstructors
 * @param var   the new value
 */
void CodeGenerationPolicy::setAutoGenerateConstructors(bool var)
{
    Settings::optionState().codeGenerationState.autoGenEmptyConstructors = var;
    emit modifiedCodeContent();
}

/**
 * Get the value of m_autoGenerateConstructors
 * @return the value of m_autoGenerateConstructors
 */
bool CodeGenerationPolicy::getAutoGenerateConstructors()
{
    return Settings::optionState().codeGenerationState.autoGenEmptyConstructors;
}

/**
 * Set the value of m_attributeAccessorScope
 * @param var the new value
 */
void CodeGenerationPolicy::setAttributeAccessorScope(Uml::Visibility::Enum var)
{
    Settings::optionState().codeGenerationState.defaultAttributeAccessorScope = var;
    emit modifiedCodeContent();
}

/**
 * Get the value of m_attributeAccessorScope
 * @return the Visibility value of m_attributeAccessorScope
 */
Uml::Visibility::Enum CodeGenerationPolicy::getAttributeAccessorScope()
{
    return Settings::optionState().codeGenerationState.defaultAttributeAccessorScope;
}

/**
 * Set the value of m_associationFieldScope
 * @param var the new value
 */
void CodeGenerationPolicy::setAssociationFieldScope(Uml::Visibility::Enum var)
{
    Settings::optionState().codeGenerationState.defaultAssocFieldScope = var;
    emit modifiedCodeContent();
}

/**
 * Get the value of m_associationFieldScope
 * @return the Visibility value of m_associationFieldScope
 */
Uml::Visibility::Enum CodeGenerationPolicy::getAssociationFieldScope()
{
    return Settings::optionState().codeGenerationState.defaultAssocFieldScope;
}

/**
 * Create a new dialog interface for this object.
 * @return dialog object
 */
CodeGenerationPolicyPage * CodeGenerationPolicy::createPage (QWidget *pWidget, const char *name)
{
    return new CodeGenerationPolicyPage (pWidget, name, 0);
}

/**
 * Emits the signal 'ModifiedCodeContent'.
 */
void CodeGenerationPolicy::emitModifiedCodeContentSig()
{
    if (!UMLApp::app()->document()->loading())
        emit modifiedCodeContent();
}

/**
 * set the defaults from a config file
 */
void CodeGenerationPolicy::setDefaults (CodeGenerationPolicy * clone, bool emitUpdateSignal)
{
    if (!clone)
        return;

    blockSignals(true); // we need to do this because otherwise most of these
    // settors below will each send the modifiedCodeContent() signal
    // needlessly (we can just make one call at the end).

    setSectionCommentsPolicy (clone->getSectionCommentsPolicy());
    setCodeVerboseDocumentComments (clone->getCodeVerboseDocumentComments());
    setHeadingFileDir (clone->getHeadingFileDir());
    setIncludeHeadings (clone->getIncludeHeadings());
    setOutputDirectory (clone->getOutputDirectory());
    setLineEndingType (clone->getLineEndingType());
    setIndentationAmount (clone->getIndentationAmount());
    setIndentationType (clone->getIndentationType());
    setModifyPolicy (clone->getModifyPolicy());
    setOverwritePolicy (clone->getOverwritePolicy());

    calculateIndentation();
    blockSignals(false); // "as you were citizen"

    if (emitUpdateSignal)
        emit modifiedCodeContent();
}

/**
 * set the defaults from a config file
 */
void CodeGenerationPolicy::setDefaults(bool emitUpdateSignal)
{
    blockSignals(true); // we need to do this because otherwise most of these
    // settors below will each send the modifiedCodeContent() signal
    // needlessly (we can just make one call at the end).

    setSectionCommentsPolicy(UmbrelloSettings::writeSectionComments());
    setCodeVerboseDocumentComments(UmbrelloSettings::forceDoc());
    setLineEndingType(UmbrelloSettings::lineEndingType());
    setIndentationType(UmbrelloSettings::indentationType());
    setIndentationAmount(UmbrelloSettings::indentationAmount());
    setAutoGenerateConstructors(UmbrelloSettings::autoGenEmptyConstructors());
    setAttributeAccessorScope(UmbrelloSettings::defaultAttributeAccessorScope());
    setAssociationFieldScope(UmbrelloSettings::defaultAssocFieldScope());
    setCommentStyle(UmbrelloSettings::commentStyle());

    calculateIndentation();

    QString path = UmbrelloSettings::outputDirectory();
    if (path.isEmpty())
        path = QDir::homePath() + QStringLiteral("/uml-generated-code/");
    setOutputDirectory (QDir (path));

    path = UmbrelloSettings::headingsDirectory();
    if (path.isEmpty()) {
#if QT_VERSION >= 0x050000
        path =  QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                          QStringLiteral("umbrello5/headings"),
                                          QStandardPaths::LocateDirectory).first();
#else
        KStandardDirs stddirs;
        path =  stddirs.findDirs("data", QStringLiteral("umbrello/headings")).first();
#endif
    }
    setHeadingFileDir (path);

    setIncludeHeadings(UmbrelloSettings::includeHeadings());
    setOverwritePolicy(UmbrelloSettings::overwritePolicy());
    setModifyPolicy(UmbrelloSettings::modnamePolicy());

    blockSignals(false); // "as you were citizen"

    if (emitUpdateSignal)
        emit modifiedCodeContent();
}

/**
 * Write Default params.
 */
void CodeGenerationPolicy::writeConfig ()
{
    UmbrelloSettings::setDefaultAttributeAccessorScope(getAttributeAccessorScope());
    UmbrelloSettings::setDefaultAssocFieldScope(getAssociationFieldScope());
    UmbrelloSettings::setCommentStyle(getCommentStyle());
    UmbrelloSettings::setAutoGenEmptyConstructors(getAutoGenerateConstructors());
    //UmbrelloSettings::setNewcodegen(getNewCodegen());
    UmbrelloSettings::setForceDoc(getCodeVerboseDocumentComments());
    UmbrelloSettings::setWriteSectionComments(getSectionCommentsPolicy());

    UmbrelloSettings::setLineEndingType(getLineEndingType());
    UmbrelloSettings::setIndentationType(getIndentationType());
    UmbrelloSettings::setIndentationAmount(getIndentationAmount());

    UmbrelloSettings::setOutputDirectory(getOutputDirectory().absolutePath());
    UmbrelloSettings::setHeadingsDirectory(getHeadingFileDir());
    UmbrelloSettings::setIncludeHeadings(getIncludeHeadings());
    UmbrelloSettings::setOverwritePolicy(getOverwritePolicy());
    UmbrelloSettings::setModnamePolicy(getModifyPolicy());

    // this will be written to the disk from the place it was called :)
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
 */
QString CodeGenerationPolicy::getHeadingFile(const QString& str)
{
    if (!getIncludeHeadings() || str.isEmpty())
        return QString();
    if (str.contains(QStringLiteral(" ")) || str.contains(QStringLiteral(";"))) {
        logWarn0("CodeGenerationPolicy::getHeadingFile: File folder must not have spaces or semicolons!");
        return QString();
    }
    //if we only get the extension, then we look for the default
    // heading.[extension]. If there is no such file, we try to
    // get any file with the same extension
    QString filename;
    QDir headingFiles = Settings::optionState().codeGenerationState.headingsDirectory;
    if (str.startsWith(QLatin1Char('.'))) {
        if (QFile::exists(headingFiles.absoluteFilePath(QStringLiteral("heading") + str)))
            filename = headingFiles.absoluteFilePath(QStringLiteral("heading") + str);
        else {
            QStringList filters;
            filters << QLatin1Char('*') + str;
            headingFiles.setNameFilters(filters);
            //if there is more than one match we just take the first one
            QStringList fileList = headingFiles.entryList();
            if (!fileList.isEmpty())
              filename = headingFiles.absoluteFilePath(fileList.first());
            // uWarning() << "header file name set to " << filename << " because it was *";
        }
    } else {   //we got a file name (not only extension)
        filename = headingFiles.absoluteFilePath(str);
    }

    if (filename.isEmpty())
        return QString();
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) {
        // uWarning() << "Error opening heading file: " << f.name();
        // uWarning() << "Headings directory was " << headingFiles.absolutePath();
        return QString();
    }

    QTextStream ts(&f);
    QString retstr;
    QString endLine = getNewLineEndingChars();
    for (int l = 0; l < MAXLINES && !ts.atEnd(); ++l) {
        retstr += ts.readLine()+endLine;
    }

    //do variable substitution
    retstr.replace(QRegExp(QStringLiteral("%author%")),
                   QString::fromLatin1(qgetenv("USER")));  //get the user name from some where else
    retstr.replace(QRegExp(QStringLiteral("%headingpath%")), filename);
    retstr.replace(QRegExp(QStringLiteral("%time%")), QTime::currentTime().toString());
    retstr.replace(QRegExp(QStringLiteral("%date%")), QDate::currentDate().toString());
    // the replace filepath, time parts are also in the code document updateHeader method
    // (which is not a virtual function)...

    return retstr;
}

