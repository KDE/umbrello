#ifndef NATIVEIMPORTBASE_H
#define NATIVEIMPORTBASE_H

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2005                                                     *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

#include <qstring.h>
#include <qstringlist.h>
#include "classimport.h"
#include "umlnamespace.h"

class UMLPackage;
class UMLClassifier;

/**
 * Intermediate base class for native Umbrello implementations of
 * programming language import
 * @author Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class NativeImportBase : public ClassImport {
public:
    /**
     * Constructor
     * @param singleLineCommentIntro  "//" for IDL and Java, "--" for Ada
     */
    NativeImportBase(QString singleLineCommentIntro);
    virtual ~NativeImportBase();

    /**
     * Implement abstract operation from ClassImport.
     */
    void importFiles(QStringList files);

protected:
    /**
     * Import a single file.
     * To be provided by the programming language specific code import
     * implementation class.
     *
     * @param file  The file to import.
     */
    virtual void parseFile(QString file) = 0;

    /**
     * Scan a single line.
     * The specific importer class is expected to call this for each line
     * read from the input file.
     * This in turn calls other methods such as preprocess() and fillSource().
     *
     * @param line  The line to scan.
     */
    void scan(QString line);

    /**
     * Preprocess a line.
     * May modify the given line to remove items consumed by the
     * preprocessing such as comments or preprocessor directives.
     * The default implementation is a no-op.
     *
     * @param line  The line to preprocess.
     * @return      True if the line was completely consumed,
     *              false if there are still items left in the line
     *              for further analysis.
     */
    virtual bool preprocess(QString& line);

    /**
     * Analyze the given word and fill `m_source'.
     * A "word" is a whitespace delimited item from the input line.
     * To be provided by the specific importer class.
     */
    virtual void fillSource(QString word) = 0;

    /**
     * Advance m_srcIndex until m_source[m_srcIndex] contains the lexeme
     * given by `until'.
     */
    void skipStmt(QString until = ";");

    /**
     * Advance m_srcIndex until m_source[m_srcIndex] contains a non-comment.
     * Comments encountered during advancement are accumulated in `m_comment'.
     * if m_srcIndex hits the end of m_source then QString::null is returned.
     */
    QString advance();

    /**
     * How to start a single line comment in this programming language.
     */
    QString m_singleLineCommentIntro;

    /**
     * The scanned lexemes.
     */
    QStringList m_source;
    /**
     * Used for indexing m_source.
     */
    uint m_srcIndex;

    /**
     * Stack of scopes.
     */
    UMLPackage *m_scope[32];
    /**
     * Indexes m_scope. Index 0 is reserved for global scope.
     */
    uint m_scopeIndex;

    /**
     * The class currently being processed.
     */
    UMLClassifier *m_klass;
    /**
     * The current access (public/protected/private)
     */
    Uml::Scope m_currentAccess;
    /**
     * Intermediate accumulator for comment text.
     */
    QString m_comment;

};

#endif

