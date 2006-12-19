/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006                                                     *
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
class PythonImport : public NativeImportBase {
public:
    PythonImport();
    virtual ~PythonImport();

protected:
    /**
     * Reimplement operation from NativeImportBase.
     */
    void initVars();

    /**
     * Implement abstract operation from NativeImportBase.
     */
    bool parseStmt();

    /**
     * Implement abstract operation from NativeImportBase.
     */
    void fillSource(const QString& line);

    /**
     * Reimplement operation from NativeImportBase.
     * In addition to handling multiline comments, this method transforms
     * changes in leading indentation into braces (opening brace for increase
     * in indentation, closing brace for decrease in indentation) in m_source.
     * Removal of Python's indentation sensitivity simplifies subsequent
     * processing using Umbrello's native import framework.
     */
    bool preprocess(QString& line);

    /**
     * Skip ahead to outermost closing brace
     */
    void skipBody();

    /**
     * Buffer for number of indentation characters (whitespace,
     * i.e. tabs or spaces) at beginning of input line.
     */
    int m_srcIndent[100];

    /**
     * Index for m_srcIndent[]. Index 0 is reserved and contains 0.
     */
    int m_srcIndentIndex;

    /**
     * Auxiliary flag denoting the opening of a block
     */
    bool m_braceWasOpened;
};

#endif

