/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include "basictypes.h"
#include "codegenpolicyext.h"
#include "codegenerationpolicy.h"
#include "umlpackagelist.h"
#include "umlclassifierlist.h"
#include "codedocumentlist.h"
#include "codeviewerstate.h"

#include <QDir>
#include <QHash>
#include <QList>

class UMLDoc;
class UMLObject;
class ClassifierCodeDocument;
class CodeDocument;
class CodeViewerDialog;
class QDomDocument;
class QDomElement;
class QXmlStreamWriter;

/**
 * This class collects together all of the code documents which form this project,
 * and generates code for them in a given language.
 *
 * CodeGenerator is the abstract base class for all types of code generators.
 * It provides the interface through which all generators are invoked and
 * provides most of the basic functionality.
 *
 * If you want to implement a CodeGenerator for some language follow
 * these steps:
 *
 * Create a class which inherits SimpleCodeGenerator or AdvancedCodeGenerator.
 * This class can have any name, I use names like CppCodeGenerator
 * for the Cpp Generator, JavaCodeGenerator for the Java Generator and so on,
 * but you can use what you want.
 *
 * The code you generate should be output to getOutputDirectory() of the
 * CodeGenerationPolicy instance and you  should respect the OverwritePolicy
 * specified. You should call findFileName(..) to get an appropriate file name,
 * and then you can call openFile() and write the generated content to this file.
 */
class CodeGenerator : public QObject
{
    Q_OBJECT
public:
    typedef enum {Generated, Failed, Skipped } GenerationState;
    CodeGenerator();
    virtual ~CodeGenerator();

    bool addCodeDocument(CodeDocument * add_object);
    bool removeCodeDocument(CodeDocument * remove_object);

    CodeDocumentList * getCodeDocumentList();

    QString getUniqueID(CodeDocument * codeDoc);

    virtual void saveToXMI(QXmlStreamWriter& writer);

    CodeDocument * findCodeDocumentByID(const QString &id);

    virtual void writeCodeToFile();
    virtual void writeCodeToFile(UMLClassifierList &list);

    void setIncludeHeadings(bool i);
    bool includeHeadings() const;

    void setHeadingFileDir(const QString &);
    QString headingFileDir() const;

    void setForceDoc(bool f);
    bool forceDoc() const;

    void setSectionCommentPolicy(CodeGenerationPolicy::WriteSectionCommentsPolicy f);
    bool forceSections() const; // TODO change to WriteSectionCommentsPolicy

    virtual QString getHeadingFile(const QString &file);

    QString findFileName(CodeDocument * codeDocument);

    static QString cleanName(const QString &name);

    static QString formatDoc(const QString& text, const QString& linePrefix = QStringLiteral(" *"), int lineWidth = 80);
    static QString formatFullDocBlock(const QString &text,
                                      const QString &blockHeader = QStringLiteral("/* "),
                                      const QString &blockFooter = QStringLiteral(" */"),
                                      const QString &linePrefix = QStringLiteral(" *"), int lineWidth = 80);

    static QString formatSourceCode(const QString& code, const QString& indentation);

    static void findObjectsRelated(UMLClassifier *c, UMLPackageList &cList);

    /**
     * A series of accessor method constructors that we need to define
     * for any particular language.
     */
    virtual void loadFromXMI(QDomElement & element);

    /**
     * Return the unique language enum that identifies this type of code generator.
     */
    virtual Uml::ProgrammingLanguage::Enum language() const = 0;

    CodeDocument * findCodeDocumentByClassifier(UMLClassifier * classifier);

    virtual QStringList defaultDatatypes() const;

    virtual bool isReservedKeyword(const QString & keyword);

    virtual QStringList reservedKeywords() const;

    virtual void createDefaultStereotypes();

    /**
     * Initialize this code generator from its parent UMLDoc.
     */
    virtual void initFromParentDocument() = 0;

protected:
    QString overwritableName(const QString& name, const QString &extension);

    bool openFile(QFile& file, const QString &name);

    void writeListedCodeDocsToFile(CodeDocumentList * docs);

    virtual void finalizeRun();

    // map of what code documents we currently have in this generator.
    QHash<QString, CodeDocument*> m_codeDocumentDictionary;

    /**
     * Used by OverwriteDialog to know if the apply to all
     * remaining files checkbox should be checked (is by default).
     */
    bool m_applyToAllRemaining;

    /**
     * The document object.
     */
    UMLDoc* m_document;

private:
    CodeDocumentList m_codedocumentVector;
    int m_lastIDIndex;

    void loadCodeForOperation(const QString& id, const QDomElement& codeDocElement);

public slots:
    virtual void syncCodeToDocument();

signals:
    void codeGenerated(UMLClassifier* concept, bool generated);
    void codeGenerated(UMLClassifier* concept, CodeGenerator::GenerationState result);
    void showGeneratedFile(const QString& filename);
};

#endif // CODEGENERATOR_H
