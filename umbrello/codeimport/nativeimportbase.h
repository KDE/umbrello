/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2005-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef NATIVEIMPORTBASE_H
#define NATIVEIMPORTBASE_H

#include "basictypes.h"
#include "classimport.h"

#include <QString>
#include <QStringList>

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
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class NativeImportBase : public ClassImport
{
public:
    explicit NativeImportBase(const QString &singleLineCommentIntro, CodeImpThread* thread = 0);
    virtual ~NativeImportBase();

protected:
    void initialize();

    void setMultiLineComment(const QString &intro, const QString &end);
    void setMultiLineAltComment(const QString &intro, const QString &end);

    virtual bool parseFile(const QString& filename);

    virtual void initVars();

    void scan(const QString& line);

    virtual bool preprocess(QString& line);

    virtual QStringList split(const QString& line);

    /**
     * Analyze the given word and fill `m_source`.
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

    void skipStmt(const QString& until = QStringLiteral(";"));
    bool skipToClosing(QChar opener);

    QString current();
    QString lookAhead();
    virtual QString advance();

    void pushScope(UMLPackage *p);
    UMLPackage *popScope();
    UMLPackage *currentScope();
    int scopeIndex();

    QString                m_singleLineCommentIntro;  ///< start token of a single line comment
    QStringList            m_source;         ///< the scanned lexemes
    int                    m_srcIndex;       ///< used for indexing m_source
    QList<UMLPackage *>    m_scope;          ///< stack of scopes for use by the specific importer
    UMLClassifier         *m_klass;          ///< class currently being processed
    Uml::Visibility::Enum  m_currentAccess;  ///< current access (public/protected/private)
    QString                m_comment;        ///< intermediate accumulator for comment text

    /**
     * True if we are currently in a multi-line comment.
     * Only applies to languages with multi-line comments.
     */
    bool m_inComment;
    bool m_isAbstract;     ///< accumulator for abstractness

    /**
     * List of parsed files. Contains file names without paths.
     * Before actually parsing a given file, NativeImportBase checks
     * whether the name is already present in this list in order to
     * avoid parsing the same file multiple times.
     */
    static QStringList m_parsedFiles;

    QString m_multiLineCommentIntro;  ///< multi line comment delimiter intro
    QString m_multiLineCommentEnd;    ///< multi line comment delimiter end
    /**
     * Some languages support an alternative set of multi line
     * comment delimiters.
     */
    QString m_multiLineAltCommentIntro;
    QString m_multiLineAltCommentEnd;
};

#endif

