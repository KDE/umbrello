/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef RUBYCLASSIFIERCODEDOCUMENT_H
#define RUBYCLASSIFIERCODEDOCUMENT_H

#include "codeclassfieldlist.h"
#include "classifiercodedocument.h"
#include "umlclassifier.h"
#include "hierarchicalcodeblock.h"
#include "rubycodeclassfield.h"
#include "rubycodeoperation.h"

#include <QString>

class RubyClassDeclarationBlock;
class RubyCodeGenerationPolicy;

/**
 * class RubyClassifierCodeDocument
 * A Ruby UMLClassifier Code Document.
 * We carve the Ruby document up into sections as follows:
 * - header
 * - class declaration
 * -   guts of the class (e.g. accessor methods, operations, dependent classes)
 */
class RubyClassifierCodeDocument : public ClassifierCodeDocument
{
    Q_OBJECT
public:

    explicit RubyClassifierCodeDocument(UMLClassifier * classifier);
    virtual ~RubyClassifierCodeDocument();

    //CodeDocumentDialog getDialog();

    RubyCodeGenerationPolicy * getRubyPolicy() const;

    QString getRubyClassName(const QString &name) const;

    QString getPath() const;

    bool addCodeOperation(CodeOperation * op);

    void updateContent();

protected:

    void resetTextBlocks();

    virtual void loadChildTextBlocksFromNode(QDomElement & root);

    void addOrUpdateCodeClassFieldMethodsInCodeBlock(CodeClassFieldList &list, RubyClassDeclarationBlock * codeBlock);

    bool forceDoc();

private:

    RubyClassDeclarationBlock * classDeclCodeBlock;
    HierarchicalCodeBlock * constructorBlock;
    HierarchicalCodeBlock * operationsBlock;

    HierarchicalCodeBlock * publicBlock;
    HierarchicalCodeBlock * privateBlock;
    HierarchicalCodeBlock * protectedBlock;

//    HierarchicalCodeBlock * namespaceBlock;

    HierarchicalCodeBlock * pubConstructorBlock;
    HierarchicalCodeBlock * protConstructorBlock;
    HierarchicalCodeBlock * privConstructorBlock;

    HierarchicalCodeBlock * pubOperationsBlock;
    HierarchicalCodeBlock * privOperationsBlock;
    HierarchicalCodeBlock * protOperationsBlock;

    void init();
    RubyClassDeclarationBlock * getClassDecl();

};

#endif // RUBYCLASSIFIERCODEDOCUMENT_H
