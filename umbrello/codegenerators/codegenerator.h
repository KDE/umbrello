/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include "basictypes.h"
#include "codegenpolicyext.h"
#include "codegenerationpolicy.h"
#include "umlpackagelist.h"
#include "umlclassifierlist.h"
#include "codedocumentlist.h"
#include "codeviewerstate.h"

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QDir>

class UMLDoc;
class UMLObject;
class ClassifierCodeDocument;
class CodeBlock;
class CodeDocument;
class CodeViewerDialog;
class QDomDocument;
class QDomElement;

/**
 * This class collects together all of the code documents which form this project,
 * and generates code for them in a given language.
 *
 * CodeGenerator is the base class for all CodeGenerators. It
 * provides the interface through which all Generators are invoked and
 * the all the basic functionality. The only thing it doesn't do is to
 * generate code =)
 *
 * If you want to implement a CodeGenerator for some language follow
 * these steps:
 *
 * Create a class which inherits CodeGenerator. This class can have
 * any name, I use names like CppCodeGenerator for the Cpp Generator,
 * JavaCodeGenerator  for the Java Generator and so on, but you can use what
 * you want.
 *
 * The code you generate should be output to "outputDirectory" and you
 * should respect the OverwritePolicy specified. You should call
 * findFileName(..) to get an appropriate file name, and then you can
 * call openFile if you want, but if you want to do it yourself you
 *
 * Finally put your generator in a library which can be dlopened
 * together with a factory class (see below) and you are ready to go.
 *
 * Note for "simple" code generators:
 * Code can be entered into a QTextEdit widget in the ClassPropDlg. This
 * code is then stored in the respective UMLOperation, written to the
 * xmi file, and also used when generating the source files.
 * The code fragments are stored into the xmi file in the section "codegeneration"
 * with the tag "sourcecode".
 */
class CodeGenerator : public QObject
{
    Q_OBJECT
public:
    CodeGenerator();
    CodeGenerator(QDomElement & element);

    virtual ~CodeGenerator();

    bool addCodeDocument(CodeDocument * add_object);

//    /**
//     * Replace (or possibly add a new) CodeDocument object to the m_codedocumentVector List.
//     * As names must be unique and each code document must have a name.
//     * @return  boolean value which will be true if the passed document was able to replace some
//     *    other document OR was added(no prior document existed..only when addIfPriorDocumentNotPresent is true).
//     *    The document which was replaced will be deleted IF deleteReplacedDocument is true.
//     */
//    bool replaceCodeDocument(CodeDocument * replace_doc = 0, bool addIfPriorDocumentNotPresent = true,
//                             bool deleteReplacedDocument = true);

    bool removeCodeDocument(CodeDocument * remove_object);

    CodeDocumentList * getCodeDocumentList();

    QString getUniqueID(CodeDocument * codeDoc);

    virtual void saveToXMI(QDomDocument & doc, QDomElement & root);

    CodeDocument * findCodeDocumentByID(const QString &id);

    virtual void writeCodeToFile();
    virtual void writeCodeToFile(UMLClassifierList &list);

    // these are utility methods for accessing the default
    // code gen policy object and *perhaps* should go away when we
    // finally implement the CodeGenDialog class -b.t.

    void setModifyNamePolicy(CodeGenerationPolicy::ModifyNamePolicy p);
    CodeGenerationPolicy::ModifyNamePolicy modifyNamePolicy()const;

    void setIncludeHeadings(bool i);
    bool includeHeadings() const;

    void setHeadingFileDir(const QString &);
    QString headingFileDir() const;

    void setForceDoc(bool f);
    bool forceDoc() const;

    void setForceSections(bool f);
    bool forceSections() const;

    virtual QString getHeadingFile(const QString &file);

    QString findFileName(CodeDocument * codeDocument);

    static QString cleanName(const QString &name );

    static QString formatDoc(const QString& text, const QString& linePrefix = " *", int lineWidth = 80 );

    static QString formatSourceCode(const QString& code, const QString& indentation);

    static void findObjectsRelated(UMLClassifier *c, UMLPackageList &cList);

    /**
     * A series of accessor method constructors that we need to define
     * for any particular language.
     */
    virtual CodeDocument * newClassifierCodeDocument(UMLClassifier * classifier) = 0;

    virtual void loadFromXMI(QDomElement & element);

    virtual CodeDocument * newCodeDocument();

    /**
     * Return the unique language enum that identifies this type of code generator.
     */
    virtual Uml::ProgrammingLanguage language() const = 0;

    CodeDocument * findCodeDocumentByClassifier(UMLClassifier * classifier);

    virtual QStringList defaultDatatypes();

    virtual CodeViewerDialog * getCodeViewerDialog(QWidget* parent, CodeDocument * doc,
            Settings::CodeViewerState state);

    virtual bool isReservedKeyword(const QString & keyword);

    virtual QStringList reservedKeywords() const;

    virtual void createDefaultStereotypes();

    virtual void initFromParentDocument();

    void connect_newcodegen_slots();

protected:

//    /**
//     * Remove (and possibly delete) all AutoGenerated content type CodeDocuments but leave
//     * the UserGenerated (and any other type) documents in this generator alone.
//     */
//    void removeAndDeleteAllAutoGeneratedCodeDocuments(bool deleteRemovedDocs = true);

    QString overwritableName(const QString& name, const QString &extension);

    bool openFile(QFile& file, const QString &name);

    void writeListedCodeDocsToFile(CodeDocumentList * docs);

    static const char * hierarchicalCodeBlockNodeName;

    // map of what code documents we currently have in this generator.
    QHash<QString, CodeDocument*> m_codeDocumentDictionary;

    /**
     * Used by overwriteDialogue to know if the apply to all
     * remaining files checkbox should be checked (is by default).
     */
    bool m_applyToAllRemaining;

    /**
     * The document object.
     */
    UMLDoc* m_document;

private:

//    /**
//     * Maps CodeDocuments to filenames. Used for finding out which file
//     * each class was written to.
//     */
    // this seems silly and overkill now. -b.t.
    // QMap<CodeDocument*,QString> *m_fileMap;

    CodeDocumentList m_codedocumentVector;
    int lastIDIndex;

    void initFields() ;

    void loadCodeForOperation(const QString& id, const QDomElement& codeDocElement);

public slots:

    virtual void checkAddUMLObject(UMLObject * obj);
    virtual void checkRemoveUMLObject(UMLObject * obj);

    virtual void syncCodeToDocument();

signals:

    void codeGenerated(UMLClassifier* concept, bool generated);

};

#endif // CODEGENERATOR_H

