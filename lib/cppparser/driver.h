/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2002, 2003 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DRIVER_H
#define DRIVER_H

#include "ast.h"

#include "macro.h"
#include <qpair.h>
#include <QStringList>
#include <qdatastream.h>
#include <qmap.h>
#include <qdatetime.h>
#include <QList>
#include <map>
#include <set>
#include <hashedstring.h>
#include "lexercache.h"


class Lexer;
class Parser;

enum {
    Dep_Global,
    Dep_Local
};

typedef QPair<QString, int> Dependence;

class ParsedFile;
typedef KSharedPtr<ParsedFile> ParsedFilePointer;

class ParsedFile: public AbstractParseResult
{
public:
    struct IncludeDesc {
        bool local; //Whether it is a local include(#include "local.h", not #include <global.h>)
        QString includePath;
        ParsedFilePointer parsed; //May be zero!
    };
//     ParsedFile() {
//     }
    explicit ParsedFile(QDataStream& s)
    {
        read(s);
    }

    ParsedFile(const QString& fileName, const QDateTime& timeStamp);

    ///Deserializes the ParsedFile from a previous call to serialize(). AST will always be zero after a call to this.
    explicit ParsedFile(const QByteArray& array);

    virtual ~ParsedFile() {}

    /**
     * @return All Macros that were used while processing this translation-unit. May be modified.
     */
    MacroSet& usedMacros();
    const MacroSet& usedMacros() const;

    /**
     * @return the count of lines that were skipped while preprocessing the file
     * */
    int skippedLines() const;

    void setSkippedLines(int lines);
    /**
     * @return Absolutely all files included by this one(no matter through how many other files they were included)
     */
//     HashedStringSet& includeFiles();

    const HashedStringSet& includeFiles() const;

    void addIncludeFiles(const HashedStringSet& includeFiles);

    void addIncludeFile(const QString& includePath, const ParsedFilePointer& parsed, bool localInclude);

    ///If this file was parsed while resolving the dependencies of another file, this returns the file this one was included from. Else returns an empty string.
    QString includedFrom() const;

    void setIncludedFrom(const QString& str);
    /**
     * @return Reference to the internal list of all directly included files(without those included indirectly)
     */
    const QList<IncludeDesc>& directIncludeFiles() const;

    operator TranslationUnitAST* () const   // May be zero
    {
        if (!this) return 0;
        return (TranslationUnitAST*)m_translationUnit.data();
    }

    TranslationUnitAST* operator -> () const
    {
        return (TranslationUnitAST*)m_translationUnit.data();
    }

    void setTranslationUnit(const TranslationUnitAST::Node& trans);

    QString fileName() const;

    QDateTime timeStamp() const;

    ///Serializes the content of this class into a byte-array. Note that this does not serialize the AST.
    QByteArray serialize() const;

    /*void read(QDataStream& stream);
    void write(QDataStream& stream) const;*/

    virtual void read(QDataStream& stream)
    {
        int directIncludeFilesCount;
        stream >> directIncludeFilesCount;
        m_directIncludeFiles.clear();
        for (int a = 0; a < directIncludeFilesCount; a++) {
            IncludeDesc i;
            Q_INT8 in;
            stream >> in;
            i.local = in;
            stream >> i.includePath;
            //"parsed" will not be reconstructed
            m_directIncludeFiles.push_back(i);
        }
        stream >> m_skippedLines;
        stream >> m_fileName;
        stream >> m_timeStamp;
        stream >> m_includedFrom;
        m_usedMacros.read(stream);
        m_translationUnit = 0;
        m_includeFiles.read(stream);
    }

    virtual void write(QDataStream& stream) const
    {
        int i = m_directIncludeFiles.size();
        stream << i;
        for (QList<IncludeDesc>::const_iterator it = m_directIncludeFiles.begin(); it != m_directIncludeFiles.end(); ++it) {
            Q_INT8 i = (*it).local;
            stream << i;
            stream << (*it).includePath;
        }
        stream << m_skippedLines;
        stream << m_fileName;
        stream << m_timeStamp;
        stream << m_includedFrom;
        m_usedMacros.write(stream);
        m_includeFiles.write(stream);
    }

    virtual ParsedFileType type() const
    {
        return CppParsedFile;
    }

private:
    QList<IncludeDesc> m_directIncludeFiles;
    MacroSet m_usedMacros;
    TranslationUnitAST::Node m_translationUnit;
    HashedStringSet m_includeFiles;
    int m_skippedLines;
    QString m_fileName;
    QDateTime m_timeStamp;
    QString m_includedFrom;
};

/**
 * An interface that provides source code to the Driver
 */
class SourceProvider
{
public:
    SourceProvider() {}
    virtual ~SourceProvider() {}

    /**
     * Get the contents of a file
     * \param fileName The name of the file to get the contents for. An absolute
     *                 path should be used.
     * \return A QString that contains the contents of the file
     */
    virtual QString contents(const QString& fileName) = 0;

    /**
     * Check to see if a file has been modified
     * \param fileName The name of hte file to get the modification state of. An
     *                 absolute path should be used.
     * \return true if the file has been modified
     * \return false if the file has not been modified
     */
    virtual bool isModified(const QString& fileName) = 0;

private:
    SourceProvider(const SourceProvider& source);
    void operator = (const SourceProvider& source);
};

/**
 * @brief The Driver class takes care of the management of the include files,
 * the macros, calls the parser and stores errors that occurred during parsing.
 */
class Driver
{
public:
    Driver();
    virtual ~Driver();

    typedef std::multimap< HashedString, Macro > MacroMap;

    /**
     * Get the source provider for this driver. This would be useful for
     * getting the text the driver is working with.
     */
    SourceProvider* sourceProvider();
    /**
     * Sets the source provider the driver will use
     * @param sourceProvider the SourceProvider the driver will use
     */
    void setSourceProvider(SourceProvider* sourceProvider);

    /**
     * @brief Resets the driver
     *
     * Clears the driver of all problems, dependencies, macros, and include paths and
     * removes any translation units that have been parsed
     */
    virtual void reset();

    /**
     * Tells the driver to start parsing a file
     * @param fileName The name of the file to parse
     * @param onlyPreProcesss Tells the driver to only run the file through the preprocessor. Defaults to false
     * @param force Force the parsing of the file. Defaults to false
     * @param macrosGlobal Should the macros be global? (Global macros are not deleted once a new translation-unit is parsed)
     * @return true on success
     */
    virtual bool parseFile(const QString& fileName, bool onlyPreProcesss = false, bool force = false, bool macrosGlobal = false);

    /**
     * Indicates that the file has been parsed
     * @param fileName The name of the file parsed. It is legal to create a ParsedFilePointer on the given item.
     */
    virtual void fileParsed(ParsedFile& fileName);

    /**
     * Removes the file specified by @p fileName from the driver
     * @param fileName The name of the file to remove
     */
    virtual void remove
    (const QString& fileName);

    /**
     * Add a dependency on another header file for @p fileName
     * @param fileName The file name to add the dependency for
     * @param dep The dependency to add
     */
    virtual void addDependence(const QString& fileName, const Dependence& dep);

    /**
     * Add a macro to the driver
     * @param macro The macro to add to the driver
     */
    virtual void addMacro(const Macro& macro);

    /**
     * Add a problem to the driver
     * @param fileName The file name to add the problem for
     * @param problem The problem to add
     */
    virtual void addProblem(const QString& fileName, const Problem& problem);

    /**
     * The current file name the driver is working with
     */
    QString currentFileName() const
    {
        return m_currentFileName;
    }
    ParsedFilePointer takeTranslationUnit(const QString& fileName);

    void takeTranslationUnit(const ParsedFile& file);
    /**
     * Get the translation unit contained in the driver for @p fileName.
     * @param fileName The name of the file to get the translation unit for
     * @return The TranslationUnitAST pointer that represents the translation unit
     * @return 0 if no translation unit exists for the file
     */
    ParsedFilePointer translationUnit(const QString& fileName) const;
    /**
     * Get the dependencies for a file
     * @param fileName The file name to get dependencies for
     * @return The dependencies for the file
     */
    QMap<QString, Dependence> dependences(const QString& fileName) const;
    /**
     * Get all the macros the driver contains
     * @return The macros
     */
    const MacroMap& macros() const;

    /**
     * Take all macros from the given map(forgetting own macros) */
    void insertMacros(const MacroSet& macros);
    /**
     * Get the list of problem areas the driver contains
     * @param fileName The filename to get problems for
     * @return The list of problems for @p fileName
     */
    QList<Problem> problems(const QString& fileName) const;

    void usingString(const HashedString& str);
    /**
     * Check if we have a macro in the driver
    * If the last stacked macro of that name is an undef-macro, false is returned.
     * @param name The name of the macro to check for
     * @return true if we have the macro in the driver
     * @return false if we don't have the macro in the driver
     */
    bool hasMacro(const HashedString& name) ;
    /**
     * Get the macro identified by @p name
     * @param name The name of the macro to get
     * @return A const reference of the macro object represented by @p name
     */
    const Macro& macro(const HashedString& name) const;
    /**
     * Get the last inserted macro identified by @p name
     * @override
     * @param name The name of the macro to get
     * @return A non-const reference of the macro object represented by @p name
     *
     */
    Macro& macro(const HashedString& name);

    /**
     * Remove the last inserted Macro of that name
     * @param macroName The name of the macro to remove
     */
    virtual void removeMacro(const HashedString& macroName);

    /**
       * Remove all macros from the driver for a certain file
       * @param fileName The file name
       */
    virtual void removeAllMacrosInFile(const QString& fileName); ///Check when this is called. It may be wrong.

    QStringList includePaths() const
    {
        return m_includePaths;
    }

    virtual QStringList getCustomIncludePath(const QString&);


    virtual void addIncludePath(const QString &path);

    virtual void clearIncludePaths();

    /// @todo remove
    const QMap<QString, ParsedFilePointer> &parsedUnits() const
    {
        return m_parsedUnits;
    }

    /**
     * Set whether or not to enable dependency resolving for files added to the driver
     */
    virtual void setResolveDependencesEnabled(bool enabled);
    /**
     * Check if dependency resolving is enabled
     * \return true if dependency resolving is enabled
     * \return false if dependency resolving is disabled
     */
    bool isResolveDependencesEnabled() const
    {
        return depresolv;
    }

    void setMaxDependenceDepth(int depth);

    /**
     * Used by the Lexer to indicate that a Macro was used
     * @param macro The used macro
     * */
    void usingMacro(const Macro& macro);

    /**
     * Returns the local instance of the lexer-cache, can be used from outside to control the cache-behavior.
     * */
    LexerCache* lexerCache();

    ///This uses getCustomIncludePath(..) to resolve the include-path internally
    QString findIncludeFile(const Dependence& dep, const QString& fromFile);

protected:
    ///This uses the state of the parser to find the include-file
    QString findIncludeFile(const Dependence& dep) const;

    /**
     * Set up the lexer.
     */
    virtual void setupLexer(Lexer* lexer);
    /**
     * Setup the parser
     */
    virtual void setupParser(Parser* parser);
    /**
     * Set up the preprocessor
     */
    virtual void setupPreProcessor();

    /**
     * Is code-information for this file already available? If false is returned, the file will be parsed.
     * Code-model and static repository should be checked to find out whether the file is already available.
     * This function is only used when dependency-resolving is activated.
     * @arg file absolute path to the file
     */
    virtual bool shouldParseIncludedFile(const ParsedFilePointer& /*file*/);

    void clearMacros();

    void clearParsedMacros();

private:
    QMap<QString, Dependence>& findOrInsertDependenceList(const QString& fileName);
    QList<Problem>& findOrInsertProblemList(const QString& fileName);


private:
    QString m_currentFileName;
    QString m_currentMasterFileName;
    typedef QMap<QString, Dependence> DependenceMap;
    typedef QMap< QString, DependenceMap> DependencesMap;
    DependencesMap m_dependences;
    MacroMap m_macros;
    QMap< QString, QList<Problem> > m_problems;
    QMap<QString, ParsedFilePointer> m_parsedUnits;
    QStringList m_includePaths;
    unsigned int depresolv : 1;
    Lexer *lexer;
    SourceProvider* m_sourceProvider;

    ParsedFilePointer m_currentParsedFile;
    CachedLexedFilePointer m_currentLexerCache;
    LexerCache m_lexerCache;

    int m_dependenceDepth;
    int m_maxDependenceDepth;

    class ParseHelper;
    friend class ParseHelper;

private:
    Driver(const Driver& source);
    void operator = (const Driver& source);
};

#endif
