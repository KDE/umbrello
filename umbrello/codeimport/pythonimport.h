/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef PYTHONIMPORT_H
#define PYTHONIMPORT_H

#include "nativeimportbase.h"

class UMLOperation;

/**
 * Python code import
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class PythonImport : public NativeImportBase
{
public:
    explicit PythonImport(CodeImpThread* thread = 0);
    virtual ~PythonImport();

protected:
    void initVars();

    bool parseInitializer(const QString &keyword, QString &type, QString &value);
    bool parseAssignmentStmt(const QString &keyword);

    bool parseMethodParameters(UMLOperation *op);
    bool parseStmt();

    void fillSource(const QString& line);

    bool preprocess(QString& line);

    QString indentation(int level);

    QString skipBody(bool *foundReturn = 0);

    /**
     * Buffer for number of indentation characters (whitespace,
     * i.e. tabs or spaces) at beginning of input line.
     */
    int m_srcIndent[100];

    /**
     * Index for m_srcIndent[]. Index 0 is reserved and contains 0.
     */
    int m_srcIndentIndex;

    bool m_braceWasOpened;  ///< Flag denoting the opening of a block.
    bool m_isStatic;        ///< Flag denoting staticness of defined method
};

#endif

