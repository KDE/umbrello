/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2005-2007                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#ifndef NATIVEIMPORTBASE_H
#define NATIVEIMPORTBASE_H

#include <qstring.h>
#include <qstringlist.h>
#include "classimport.h"
#include "../umlnamespace.h"

class UMLPackage;
class UMLClassifier;

/**
 * Intermediate base class for native Umbrello implementations of
 * programming language import
 *
 * The default call sequence is as follows (RealizedLanguageImport
 * is used as a placeholder name for the concrete language importer.)
 *   NativeImportBase                      RealizedLanguageImport
 * --> importFiles()
 *       parseFile()
 *         -----------------------------------> initVars()
 *         scan()
 *           preprocess() (may be reimplemented)
 *           ---------------------------------> fillSource()
 *         -----------------------------------> parseStmt()
 * This sequence may be changed by overriding default implementations
 * of virtual methods in NativeImportBase.
 *
 * @short Base class for native implementations of language import
 * @author Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class NativeImportBase : public ClassImport {
public:
    /**
     * Constructor
     * @param singleLineCommentIntro  "//" for IDL and Java, "--" for Ada
     */
    NativeImportBase(const QString &singleLineCommentIntro);
    virtual ~NativeImportBase();

protected:
    /**
     * Implement abstract operation from ClassImport.
     */
    void initialize();

    /**
     * Set the delimiter strings for a multi line comment.
     *
     * @param intro  In languages with a C style multiline comment
     *               this is slash-star.
     * @param end    In languages with a C style multiline comment
     *               this is star-slash.
     */
    void setMultiLineComment(const QString &intro, const QString &end);
    /**
     * Set the delimiter strings for an alternative form of
     * multi line comment. See setMultiLineComment().
     */
    void setMultiLineAltComment(const QString &intro, const QString &end);

    /**
     * Import a single file.
     * The default implementation should be feasible for languages that
     * don't depend on an external preprocessor.
     *
     * @param filename  The file to import.
     */
    virtual void parseFile(const QString& filename);

    /**
     * Initialize auxiliary variables.
     * This is called by the default implementation of parseFile()
     * after scanning (before parsing the QStringList m_source.)
     * The default implementation is empty.
     */
    virtual void initVars();

    /**
     * Scan a single line.
     * parseFile() calls this for each line read from the input file.
     * This in turn calls other methods such as preprocess() and fillSource().
     *
     * @param line  The line to scan.
     */
    void scan(QString line);

    /**
     * Preprocess a line.
     * May modify the given line to remove items consumed by the
     * preprocessing such as comments or preprocessor directives.
     * The default implementation handles multi-line comments.
     *
     * @param line  The line to preprocess.
     * @return      True if the line was completely consumed,
     *              false if there are still items left in the line
     *              for further analysis.
     */
    virtual bool preprocess(QString& line);

    /**
     * Split the line so that a string is returned as a single element of the list.
     * When not in a string then split at white space.
     * The default implementation is suitable for C style strings and char constants.
     */
    virtual QStringList split(const QString& line);

    /**
     * Analyze the given word and fill `m_source'.
     * A "word" is a whitespace delimited item from the input line.
     * To be provided by the specific importer class.
     */
    virtual void fillSource(const QString& word) = 0;

    /**
     * Parse the statement which starts at m_source[m_srcIndex]
     * leaving m_srcIndex pointing to the end of the recognized
     * statement.
     * To be provided by the concrete importer.
     *
     * @return   True if the statement was recognized.
     */
    virtual bool parseStmt() = 0;

    /**
     * Advance m_srcIndex until m_source[m_srcIndex] contains the lexeme
     * given by `until'.
     */
    void skipStmt(QString until = ";");

    /**
     * Advance m_srcIndex to the index of the corresponding closing character
     * of the given opening.  Nested opening/closing pairs are respected.
     * Valid openers are:    '{'  '['  '('  '<'
     *
     * @return  True for success, false for misuse (invalid opener) or
     *          if no matching closing character is found in m_source.
     */
    bool skipToClosing(QChar opener);

    /**
     * Advance m_srcIndex until m_source[m_srcIndex] contains a non-comment.
     * Comments encountered during advancement are accumulated in `m_comment'.
     * If m_srcIndex hits the end of m_source then QString::null is returned.
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
     * Stack of scopes for use by the specific importer.
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
    Uml::Visibility m_currentAccess;
    /**
     * Intermediate accumulator for comment text.
     */
    QString m_comment;
    /**
     * True if we are currently in a multi-line comment.
     * Only applies to languages with multi-line comments.
     */
    bool m_inComment;
    /**
     * Accumulator for abstractness
     */
    bool m_isAbstract;

    /**
     * List of parsed files. Contains file names without paths.
     * Before actually parsing a given file, NativeImportBase checks
     * whether the name is already present in this list in order to
     * avoid parsing the same file multiple times.
     */
    QStringList m_parsedFiles;

    /**
     * Multi line comment delimiters
     */
    QString m_multiLineCommentIntro;
    QString m_multiLineCommentEnd;
    /**
     * Some languages support an alternative set of multi line
     * comment delimiters.
     */
    QString m_multiLineAltCommentIntro;
    QString m_multiLineAltCommentEnd;
};

#endif

