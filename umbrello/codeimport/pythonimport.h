/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006-2009                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#ifndef PYTHONIMPORT_H
#define PYTHONIMPORT_H

#include "nativeimportbase.h"

/**
 * Python code import
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class PythonImport : public NativeImportBase
{
public:
    PythonImport(CodeImpThread* thread = 0);
    virtual ~PythonImport();

protected:
    void initVars();

    bool parseStmt();

    void fillSource(const QString& line);

    bool preprocess(QString& line);

    QString indentation(int level);

    QString skipBody();

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
};

#endif

