/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CPPCODEGENERATOR_H
#define CPPCODEGENERATOR_H

#include "codedocumentlist.h"
#include "codeviewerstate.h"
#include "advancedcodegenerator.h"
#include "cppmakecodedocument.h"
#include "umldoc.h"

#include <QString>

class CodeViewerDialog;
class CPPHeaderCodeDocument;

class CPPCodeGenerator : public AdvancedCodeGenerator
{
    Q_OBJECT
public:

    static const bool DEFAULT_BUILD_MAKEFILE;

    CPPCodeGenerator();
    virtual ~CPPCodeGenerator();

    void setCreateProjectMakefile(bool buildIt);
    bool getCreateProjectMakefile();

    QString fixTypeName(const QString &name);

    bool addHeaderCodeDocument (CPPHeaderCodeDocument * doc);
    bool removeHeaderCodeDocument (CPPHeaderCodeDocument * remove_object);

    Uml::ProgrammingLanguage::Enum language() const;

    CodeDocument * newClassifierCodeDocument (UMLClassifier * classifier);
    CPPHeaderCodeDocument * newHeaderClassifierCodeDocument (UMLClassifier * classifier);

    virtual CodeViewerDialog * getCodeViewerDialog(QWidget* parent, CodeDocument * doc,
            Settings::CodeViewerState & state);

    virtual void writeCodeToFile();
    virtual void writeCodeToFile(UMLClassifierList &list);

    QStringList defaultDatatypes() const;

    virtual void saveToXMI(QXmlStreamWriter& writer);

    virtual QStringList reservedKeywords() const;

    virtual void createDefaultStereotypes ();

    void initFromParentDocument();

protected:

    CPPMakefileCodeDocument * newMakefileCodeDocument ();

    CPPHeaderCodeDocument * findHeaderCodeDocumentByClassifier (UMLClassifier * classifier);

private:

    bool             m_createMakefile;
    CodeDocumentList m_headercodedocumentVector;  //< a separate list for recording the header documents

public slots:

    virtual void checkAddUMLObject (UMLObject * obj);
    virtual void checkRemoveUMLObject (UMLObject * obj);

    virtual void syncCodeToDocument ();
};

#endif // CPPCODEGENERATOR_H
