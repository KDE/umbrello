/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2002, 2003 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#define CACHELEXER
#define DBG_SRC QLatin1String("Driver")

#include "driver.h"
#include "lexer.h"
#include "parser.h"
#include "debug_utils.h"

#include <KLocalizedString>
#include <stdlib.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qdatastream.h>
#include <qbuffer.h>
#include <assert.h>

#include <iostream>

DEBUG_REGISTER_DISABLED(Driver)

//     void Macro::read(QDataStream& stream) {
//         stream >> m_idHashValid;
//         stream >> m_valueHashValid;
//         stream >> m_idHash;
//         stream >> m_valueHash;
//         stream >> m_name;
//         stream >> m_body;
//         stream >> m_fileName;
//         stream >> m_hasArguments;
//         stream >> m_argumentList;
//     }
//
//     void Macro::write(QDataStream& stream) const {
//         stream << m_idHashValid;
//         stream << m_valueHashValid;
//         stream << m_idHash;
//         stream << m_valueHash;
//         stream << m_name;
//         stream << m_body;
//         stream << m_fileName;
//         stream << m_hasArguments;
//         stream << m_argumentList;
//     }


class IntIncreaser
{
public:
    IntIncreaser(int& i) : m_i(i)
    {
        ++m_i;
    }
    ~IntIncreaser()
    {
        --m_i;
    }
private:
    int& m_i;
};

class DefaultSourceProvider: public SourceProvider
{
public:
    DefaultSourceProvider() {}

    virtual QString contents(const QString& fileName)
    {
        QString source;

        QFile f(fileName);
        if (f.open(QIODevice::ReadOnly)) {
            QTextStream s(&f);
            source = s.readAll();
            f.close();
        }
        return source;
    }

    virtual bool isModified(const QString& fileName)
    {
        Q_UNUSED(fileName);
        return true;
    }

private:
    DefaultSourceProvider(const DefaultSourceProvider& source);
    void operator = (const DefaultSourceProvider& source);
};


Driver::Driver()
    : depresolv(0), lexer(nullptr), m_lexerCache(this), m_dependenceDepth(0), m_maxDependenceDepth(20)
{
    m_sourceProvider = new DefaultSourceProvider();
}

Driver::~Driver()
{
    reset();
    delete m_sourceProvider;
}

void Driver::setMaxDependenceDepth(int depth)
{
    m_maxDependenceDepth = depth;
}

SourceProvider* Driver::sourceProvider()
{
    return m_sourceProvider;
}

void Driver::setSourceProvider(SourceProvider* sourceProvider)
{
    delete m_sourceProvider;
    m_sourceProvider = sourceProvider;
}

void Driver::reset()
{
    m_lexerCache.clear();
    m_dependences.clear();
    m_macros.clear();
    m_problems.clear();
    m_includePaths.clear();

    while (m_parsedUnits.size()) {
        //TranslationUnitAST* unit = **m_parsedUnits.begin();
        m_parsedUnits.erase(m_parsedUnits.begin());
        //delete(unit);
    }
}

QStringList Driver::getCustomIncludePath(const QString&)
{
    return includePaths();
}

void Driver::remove
(const QString & fileName)
{
    m_dependences.remove(fileName);
    m_problems.remove(fileName);
    if (!isResolveDependencesEnabled())
        removeAllMacrosInFile(fileName);

    QMap<QString, ParsedFilePointer>::Iterator it = m_parsedUnits.find(fileName);
    if (it != m_parsedUnits.end()) {
        //TranslationUnitAST * unit = **it;
        m_parsedUnits.erase(it);
        //delete(unit);
    }
}

void Driver::removeAllMacrosInFile(const QString& fileName)
{
    MacroMap::iterator it = m_macros.begin();
    while (it != m_macros.end()) {
        Macro m = (*it).second;
        if (m.fileName() == fileName) {
            m_macros.erase(it++);
        } else {
            ++it;
        }
    }
}

void Driver::usingString(const HashedString& str)
{
#ifdef CACHELEXER
    if (m_currentLexerCache) {
        m_currentLexerCache->addString(m_lexerCache.unifyString(str));
    }
#endif
}

bool Driver::hasMacro(const HashedString& name)
{
    std::pair< MacroMap::const_iterator, MacroMap::const_iterator > range = m_macros.equal_range(name);
    if (range.first == range.second) {
        return false;
    } else {
        const Macro& m((*(--range.second)).second);
        if (m.isUndef())
            return false;
        else
            return true;
    }
    return false;
}

QString deepCopy(const QString& str)
{
    return str;
    //return str.ascii();
}

const Macro& Driver::macro(const HashedString& name) const
{
    std::pair< MacroMap::const_iterator, MacroMap::const_iterator > range = m_macros.equal_range(name);
    if (range.first == range.second) {
        return (*const_cast<MacroMap&>(m_macros).insert(std::make_pair(deepCopy(name.str()), Macro()))).second;  ///Since we need to return a reference, there's no other way.
    } else {
        return (*(--range.second)).second;
    }
}
Macro& Driver::macro(const HashedString& name)
{
    std::pair< MacroMap::iterator, MacroMap::iterator > range = m_macros.equal_range(name);
    if (range.first == range.second) {
        return (*m_macros.insert(std::make_pair(deepCopy(name.str()), Macro()))).second;
    } else {
        return (*(--range.second)).second;
    }
}

void Driver::addMacro(const Macro & macro)
{
    std::pair< MacroMap::iterator, MacroMap::iterator > range = m_macros.equal_range(macro.name());

    if (range.first == range.second) {
        m_macros.insert(std::make_pair(deepCopy(macro.name()), macro));
    } else {
        ///Insert behind the other macros
        m_macros.insert(range.second, std::make_pair(deepCopy(macro.name()), macro));
        Macro cp = this->macro(macro.name());
        assert(macro == cp);
    }

#ifdef CACHELEXER
    if (m_currentLexerCache)
        m_currentLexerCache->addDefinedMacro(macro);
#endif
}

void Driver::removeMacro(const HashedString& macroName)
{
    std::pair< MacroMap::iterator, MacroMap::iterator > range = m_macros.equal_range(macroName);
    if (range.first != range.second) {
        m_macros.erase(--range.second);
    }
}

ParsedFilePointer Driver::takeTranslationUnit(const QString& fileName)
{
    QMap<QString, ParsedFilePointer>::const_iterator it = m_parsedUnits.constFind(fileName);
    ParsedFilePointer unit(*it);
    m_parsedUnits[ fileName ] = nullptr;
    return unit;
}

void Driver::takeTranslationUnit(const ParsedFile& file)
{
    m_parsedUnits[ file.fileName() ] = nullptr;
}

ParsedFilePointer Driver::translationUnit(const QString& fileName) const
{
    QMap<QString, ParsedFilePointer>::const_iterator it = m_parsedUnits.constFind(fileName);
    if (it != m_parsedUnits.constEnd())
        return it.value();
    else
        return ParsedFilePointer();
}

class Driver::ParseHelper
{
public:
    ParseHelper(const QString& fileName, bool force, Driver* driver, bool reportMessages = true, QString includedFrom = QString())
    : m_wasReset(false), m_fileName(fileName), m_previousFileName(driver->m_currentFileName),  m_previousLexer(driver->lexer),
      m_previousParsedFile(driver->m_currentParsedFile), m_previousCachedLexedFile(driver->m_currentLexerCache), m_force(force),
      m_reportMessages(reportMessages), m_includedFrom(includedFrom), m_driver(driver), m_lex(m_driver) {}

    /**
     * This function must be called after constructing the ParseHelper.
     */
    void init()
    {
        QFileInfo fileInfo(m_fileName);
        m_driver->m_currentParsedFile = new ParsedFile(m_fileName, fileInfo.lastModified());
        if (!m_includedFrom.isEmpty())
            m_driver->m_currentParsedFile->setIncludedFrom(m_includedFrom);
#ifdef CACHELEXER
        m_driver->m_currentLexerCache = new CachedLexedFile(m_fileName, &m_driver->m_lexerCache);
#endif
        m_absoluteFilePath = fileInfo.absoluteFilePath();

        QMap<QString, ParsedFilePointer>::Iterator it = m_driver->m_parsedUnits.find(m_absoluteFilePath);

        if (m_force && it != m_driver->m_parsedUnits.end()) {
            m_driver->takeTranslationUnit(m_absoluteFilePath);
        } else if (it != m_driver->m_parsedUnits.end() && *it != (ParsedFilePointer)nullptr) {
            // file already processed
            return ;
        }

        CachedLexedFilePointer lexedFileP = m_driver->m_lexerCache.lexedFile(HashedString(m_fileName));

        m_driver->m_dependences.remove(m_fileName);
        m_driver->m_problems.remove(m_fileName);

        m_driver->m_currentFileName = m_fileName;

        m_driver->lexer = &m_lex;
        m_driver->setupLexer(&m_lex);

        m_lex.setReportMessages(m_reportMessages);

        DEBUG() << "lexing file " << m_fileName ;
        m_fileContent = m_driver->sourceProvider()->contents(m_fileName);
        m_lex.setSource(m_fileContent);
        if (m_previousCachedLexedFile)
            m_previousCachedLexedFile->merge(*m_driver->m_currentLexerCache);
        else
            m_driver->findOrInsertProblemList(m_driver->m_currentMasterFileName)  += m_driver->m_currentLexerCache->problems();

        if (!lexedFileP && m_previousParsedFile) //only add the new cache-instance if a fitting isn't already stored, and if this file was included by another one.
            m_driver->m_lexerCache.addLexedFile(m_driver->m_currentLexerCache);

        //Copy the recursive include-files into the ParsedFile
        m_driver->m_currentParsedFile->addIncludeFiles(m_driver->m_currentLexerCache->includeFiles());
        m_driver->m_currentParsedFile->setSkippedLines(m_lex.skippedLines());
    }

    void parse()
    {
        QString oldMasterFileName = m_driver->m_currentMasterFileName; //Change the master-file so problems will be reported correctly
        m_driver->m_currentMasterFileName = m_absoluteFilePath;

        CachedLexedFilePointer lf = m_driver->m_currentLexerCache; //Set the lexer-cache to zero, so the problems registered through addProblem go directly into the file
        m_driver->m_currentLexerCache = nullptr;

        Parser parser(m_driver, m_driver->lexer);
        m_driver->setupParser(&parser);

        TranslationUnitAST::Node unit;
        parser.parseTranslationUnit(unit);
        m_driver->m_currentParsedFile->setTranslationUnit(unit);
        m_driver->m_parsedUnits.insert(m_fileName, m_driver->m_currentParsedFile);
        m_driver->fileParsed(*m_driver->m_currentParsedFile);

        m_driver->m_currentLexerCache = lf;

        m_driver->m_currentMasterFileName = oldMasterFileName;
    }

    ParsedFilePointer parsedFile() const
    {
        return m_driver->m_currentParsedFile;
    }

    void reset()
    {
        if (!m_wasReset) {
            m_driver->m_currentFileName = m_previousFileName;
            m_driver->lexer = m_previousLexer;
            m_driver->m_currentParsedFile = m_previousParsedFile;
            m_driver->m_currentLexerCache = m_previousCachedLexedFile;
            m_wasReset = true;
        }
    }

    ~ParseHelper()
    {
        reset();
    }


private:
    bool m_wasReset;
    QString m_fileName;
    QString m_absoluteFilePath;
    QString m_previousFileName;
    Lexer* m_previousLexer;
    ParsedFilePointer m_previousParsedFile;
    CachedLexedFilePointer m_previousCachedLexedFile;
    bool m_force;
    bool m_reportMessages;
    QString m_includedFrom;
    Driver* m_driver;
    Lexer m_lex;
    QString m_fileContent;
};


void Driver::addDependence(const QString & fileName, const Dependence & dep)
{

    // this can happen if the parser was invoked on a snippet of text and not a file
    if (fileName.isEmpty() || !m_currentParsedFile)
        return;

    //@todo prevent cyclic dependency-loops
    QFileInfo fileInfo(dep.first);
    QString fn = fileInfo.absoluteFilePath();

    if (!depresolv) {
        findOrInsertDependenceList(fileName).insert(fn, dep);
        m_currentParsedFile->addIncludeFile(dep.first, ParsedFilePointer(), dep.second == Dep_Local);
        return ;
    }

    QString file = findIncludeFile(dep);

    findOrInsertDependenceList(fileName).insert(file, dep);
    m_currentParsedFile->addIncludeFile(file, ParsedFilePointer(), dep.second == Dep_Local);

    if (!QFile::exists(file)) {
        Problem p(i18n("Could not find include file %1").arg(dep.first),
                  lexer ? lexer->currentLine() : -1,
                  lexer ? lexer->currentColumn() : -1, Problem::Level_Warning);
        addProblem(fileName, p);
        return ;
    }

    if (m_currentLexerCache)
        m_currentLexerCache->addIncludeFile(file, QDateTime()); ///The time will be overwritten in CachedLexedFile::merge(...)

    /**What should be done:
     * 1. Lex the file to get all the macros etc.
     * 2. TODO: Check what previously set macros the file was affected by, and compare those macros to any previously parsed instances of this file.
     *  2.1 If there is a parse-instance of the file where all macros that played a role had the same values, we do not need to reparse this file.
     *  2.2 If there is no such fitting instance, the file needs to be parsed and put to the code-model.
     *
     * It'll be necessary to have multiple versions of one file in the code-model.
     */

    IntIncreaser i(m_dependenceDepth);
    if (m_dependenceDepth > m_maxDependenceDepth) {
        DEBUG() << "maximum dependence-depth of " << m_maxDependenceDepth << " was reached, " << fileName << " will not be processed" ;
        return;
    }

    CachedLexedFilePointer lexedFileP = m_lexerCache.lexedFile(HashedString(file));
    if (lexedFileP) {
        CachedLexedFile& lexedFile(*lexedFileP);
        m_currentLexerCache->merge(lexedFile); //The ParseHelper will copy the include-files into the result later
        for (MacroSet::Macros::const_iterator it = lexedFile.definedMacros().macros().begin(); it != lexedFile.definedMacros().macros().end(); ++it) {
            addMacro((*it));
        }
        ///@todo fill usingMacro(...)
        return;
    }

    ParseHelper h(file, true, this, false, m_currentMasterFileName);
    h.init();

    /*if (m_parsedUnits.find(file) != m_parsedUnits.end())
        return;*/

    if (shouldParseIncludedFile(m_currentParsedFile)) ///Until the ParseHelper is destroyed, m_currentParsedFile will stay the included file
        h.parse();
}

void Driver::addProblem(const QString & fileName, const Problem & problem)
{
    Problem p(problem);
    p.setFileName(fileName);

    if (m_currentLexerCache)
        m_currentLexerCache->addProblem(p);
    else
        findOrInsertProblemList(m_currentMasterFileName).append(p);
}

QMap< QString, Dependence >& Driver::findOrInsertDependenceList(const QString & fileName)
{
    QMap<QString, QMap<QString, Dependence> >::Iterator it = m_dependences.find(fileName);
    if (it != m_dependences.end())
        return *it;

    QMap<QString, Dependence> l;
    m_dependences.insert(deepCopy(fileName), l);
    return m_dependences[ fileName ];
}

QList < Problem >& Driver::findOrInsertProblemList(const QString & fileName)
{
    QMap<QString, QList<Problem> >::Iterator it = m_problems.find(fileName);
    if (it != m_problems.end())
        return *it;

    QList<Problem> l;
    m_problems.insert(fileName, l);
    return m_problems[ fileName ];
}

QMap< QString, Dependence > Driver::dependences(const QString & fileName) const
{
    QMap<QString, QMap<QString, Dependence> >::ConstIterator it = m_dependences.find(fileName);
    if (it != m_dependences.end())
        return *it;
    return QMap<QString, Dependence>();
}

const Driver::MacroMap& Driver::macros() const
{
    return m_macros;
}

void Driver::insertMacros(const MacroSet& macros)
{
    for (MacroSet::Macros::const_iterator it = macros.m_usedMacros.begin(); it != macros.m_usedMacros.end(); ++it) {
        addMacro(*it);
    }
}

QList < Problem > Driver::problems(const QString & fileName) const
{
    QMap<QString, QList<Problem> >::ConstIterator it = m_problems.find(fileName);
    if (it != m_problems.end())
        return *it;
    return QList<Problem>();
}

void Driver::clearMacros()
{
    m_macros.clear();
}

void Driver::clearParsedMacros()
{
    //Keep global macros
    for (MacroMap::iterator it = m_macros.begin(); it != m_macros.end();) {
        if (!(*it).second.fileName().isEmpty()) {
            m_macros.erase(it++);
        } else {
            ++it;
        }
    }
}

bool Driver::parseFile(const QString& fileName, bool onlyPreProcess, bool force , bool macrosGlobal)
{
    QString oldMasterFileName = m_currentMasterFileName;
    m_currentMasterFileName = fileName;

//if(isResolveDependencesEnabled())
    clearParsedMacros(); ///Since everything will be re-lexed, we do not need any old macros

    m_lexerCache.increaseFrame();

    //Remove the problems now instead of in ParseHelper, because this way the problems reported by getCustomIncludePath(...) will not be discarded
    m_problems.remove(fileName);

    QStringList oldIncludePaths = m_includePaths;
    m_includePaths = getCustomIncludePath(fileName);

    ParseHelper p(fileName, force, this);
    p.init();
    if (!onlyPreProcess) {
        p.parse();
    }
    if (macrosGlobal) {
        for (MacroMap::iterator it = m_macros.begin(); it != m_macros.end(); ++it) {
            if ((*it).second.fileName() == fileName) {
                (*it).second.setFileName(QString());
            }
        }
    }

    m_includePaths = oldIncludePaths;
    m_currentMasterFileName = oldMasterFileName;
    return true;
}

void Driver::setupLexer(Lexer * lexer)
{
    // stl
    lexer->addSkipWord(QLatin1String("__STL_BEGIN_NAMESPACE"));
    lexer->addSkipWord(QLatin1String("__STL_END_NAMESPACE"));
    lexer->addSkipWord(QLatin1String("__STL_BEGIN_RELOPS_NAMESPACE"));
    lexer->addSkipWord(QLatin1String("__STL_END_RELOPS_NAMESPACE"));
    lexer->addSkipWord(QLatin1String("__STL_TEMPLATE_NULL"));
    lexer->addSkipWord(QLatin1String("__STL_TRY"));
    lexer->addSkipWord(QLatin1String("__STL_UNWIND"));
    lexer->addSkipWord(QLatin1String("__STL_NOTHROW"));
    lexer->addSkipWord(QLatin1String("__STL_NULL_TMPL_ARGS"));
    lexer->addSkipWord(QLatin1String("__STL_UNWIND"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("__GC_CONST"));
    lexer->addSkipWord(QLatin1String("__HASH_ALLOC_INIT"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("__STL_DEFAULT_ALLOCATOR"), SkipWordAndArguments, QLatin1String("T"));
    lexer->addSkipWord(QLatin1String("__STL_MUTEX_INITIALIZER"));
    lexer->addSkipWord(QLatin1String("__STL_NULL_TMPL_ARGS"));

    // antlr
    lexer->addSkipWord(QLatin1String("ANTLR_BEGIN_NAMESPACE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("ANTLR_USE_NAMESPACE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("ANTLR_USING_NAMESPACE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("ANTLR_END_NAMESPACE"));
    lexer->addSkipWord(QLatin1String("ANTLR_C_USING"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("ANTLR_API"));

    // gnu
    lexer->addSkipWord(QLatin1String("__extension__"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("__attribute__"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("__BEGIN_DECLS"));
    lexer->addSkipWord(QLatin1String("__END_DECLS"));
    lexer->addSkipWord(QLatin1String("__THROW"));
    lexer->addSkipWord(QLatin1String("__restrict"));
    lexer->addSkipWord(QLatin1String("__restrict__"));
    lexer->addSkipWord(QLatin1String("__attribute_pure__"));
    lexer->addSkipWord(QLatin1String("__attribute_malloc__"));
    lexer->addSkipWord(QLatin1String("__attribute_format_strfmon__"));
    lexer->addSkipWord(QLatin1String("__asm__"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("__devinit"));
    lexer->addSkipWord(QLatin1String("__devinit__"));
    lexer->addSkipWord(QLatin1String("__init"));
    lexer->addSkipWord(QLatin1String("__init__"));
    lexer->addSkipWord(QLatin1String("__signed"));
    lexer->addSkipWord(QLatin1String("__signed__"));
    lexer->addSkipWord(QLatin1String("__unsigned"));
    lexer->addSkipWord(QLatin1String("__unsigned__"));
    lexer->addSkipWord(QLatin1String("asmlinkage"));
    lexer->addSkipWord(QLatin1String("____cacheline_aligned"));
    lexer->addSkipWord(QLatin1String("__glibcpp_class_requires"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("__glibcpp_class2_requires"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("__glibcpp_class4_requires"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("__glibcpp_function_requires"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("restrict"));

    lexer->addSkipWord(QLatin1String("__BEGIN_NAMESPACE_STD"));
    lexer->addSkipWord(QLatin1String("__END_NAMESPACE_STD"));
    lexer->addSkipWord(QLatin1String("__BEGIN_NAMESPACE_C99"));
    lexer->addSkipWord(QLatin1String("__END_NAMESPACE_C99"));
    lexer->addSkipWord(QLatin1String("__USING_NAMESPACE_STD"), SkipWordAndArguments);

    // kde
    lexer->addSkipWord(QLatin1String("K_SYCOCATYPE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("EXPORT_DOCKCLASS"));
    lexer->addSkipWord(QLatin1String("K_EXPORT_COMPONENT_FACTORY"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("K_SYCOCAFACTORY"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("KDE_DEPRECATED"));

    // qt
    lexer->addSkipWord(QLatin1String("Q_OBJECT"));
    lexer->addSkipWord(QLatin1String("Q_INLINE_TEMPLATES"));
    lexer->addSkipWord(QLatin1String("Q_TEMPLATE_EXTERN"));
    lexer->addSkipWord(QLatin1String("Q_TYPENAME"));
    lexer->addSkipWord(QLatin1String("Q_REFCOUNT"));
    lexer->addSkipWord(QLatin1String("Q_EXPLICIT"));
    lexer->addSkipWord(QLatin1String("QMAC_PASCAL"));
    lexer->addSkipWord(QLatin1String("QT_STATIC_CONST"));
    lexer->addSkipWord(QLatin1String("QT_STATIC_CONST_IMPL"));
    lexer->addSkipWord(QLatin1String("QT_WIN_PAINTER_MEMBERS"));
    lexer->addSkipWord(QLatin1String("QT_NC_MSGBOX"));
    lexer->addSkipWord(QLatin1String("CALLBACK_CALL_TYPE"));

    lexer->addSkipWord(QLatin1String("ACTIVATE_SIGNAL_WITH_PARAM"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_ALIGNOF"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_ARG"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_ASSERT"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_ASSERT_X"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_BASIC_ATOMIC_INITIALIZER"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_CHECK_INVALID_SOCKETLAYER"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_CHECK_NOT_STATE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_CHECK_PTR"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_CHECK_SOCKETENGINE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_CHECK_STATE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_CHECK_STATES"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_CHECK_TYPE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_CHECK_VALID_SOCKETLAYER"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_CLASSINFO"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_CLEANUP_RESOURCE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_COMPLEX_TYPE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_CONSTRUCTOR_FUNCTION"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_CONSTRUCTOR_FUNCTION0"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_D"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECL_ALIGN"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_ASSOCIATIVE_ITERATOR"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_BUILTIN_METATYPE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_EXTENSION_INTERFACE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_FLAGS"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_INCOMPATIBLE_FLAGS"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_INTERFACE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_METATYPE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_MUTABLE_ASSOCIATIVE_ITERATOR"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_MUTABLE_SEQUENTIAL_ITERATOR"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_OPERATORS_FOR_FLAGS"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_PRIVATE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_PRIVATE_D"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_PUBLIC"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_SEQUENTIAL_ITERATOR"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_SHARED"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_SHARED_STL"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_TR_FUNCTIONS"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_TYPEINFO"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECLARE_TYPEINFO_BODY"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DECL_METATYPE_HELPER"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DESTRUCTOR_FUNCTION"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DESTRUCTOR_FUNCTION0"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DF"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DISABLE_COPY"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_DUMMY_COMPARISON_OPERATOR"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_ENUMS"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_EXPORT_PLUGIN"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_EXPORT_PLUGIN2"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_EXPORT_STATIC_PLUGIN"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_EXPORT_STATIC_PLUGIN2"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_FLAGS"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_FOREACH"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_FT_GLYPHSLOT_EMBOLDEN"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_FUNC_INFO"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_GLOBAL_STATIC"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_GLOBAL_STATIC_INIT"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_GLOBAL_STATIC_WITH_ARGS"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_GLOBAL_STATIC_WITH_INITIALIZER"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_GTK_IS_WIDGET"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_HASH_DECLARE_INT_NODES"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_IMPL_METATYPE_HELPER"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_IMPORT_PLUGIN"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_IN"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_INFINITY"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_INIT_CHECK"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_INIT_RESOURCE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_INIT_RESOURCE_EXTERN"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_INT64_C"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_INTERFACES"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_LIKELY"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_MAX_3"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_MAXINT"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_MIN_3"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_MM"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_NEAR_CLIP"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_NONE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_OVERRIDE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_PID"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_PLUGIN_INSTANCE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_PRIVATE_PROPERTY"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_PRIVATE_SLOT"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_PROPERTY"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_Q"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_QNAN"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_REGISTER_METATYPE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_RETURN_ARG"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_REVISION"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_SCRIPT_DECLARE_QMETAOBJECT"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_SETS"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_SNAN"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_TOOL_BASE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_TR"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_TYPEID"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_TYPEOF"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_UINT64_C"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_UNLIKELY"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_UNREACHABLE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_UNSET"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_UNUSED"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("Q_VARIANT_AS"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("QT_MODULE"), SkipWordAndArguments);
    lexer->addSkipWord(QLatin1String("QT_BEGIN_HEADER"));
    lexer->addSkipWord(QLatin1String("QT_BEGIN_NAMESPACE"));
    lexer->addSkipWord(QLatin1String("QT_END_HEADER"));
    lexer->addSkipWord(QLatin1String("QT_END_NAMESPACE"));

    // flex
    lexer->addSkipWord(QLatin1String("yyconst"));
    lexer->addSkipWord(QLatin1String("YY_RULE_SETUP"));
    lexer->addSkipWord(QLatin1String("YY_BREAK"));
    lexer->addSkipWord(QLatin1String("YY_RESTORE_YY_MORE_OFFSET"));

    // gtk
    lexer->addSkipWord(QLatin1String("G_BEGIN_DECLS"));
    lexer->addSkipWord(QLatin1String("G_END_DECLS"));
    lexer->addSkipWord(QLatin1String("G_GNUC_CONST"));
    lexer->addSkipWord(QLatin1String("G_CONST_RETURN"));
    lexer->addSkipWord(QLatin1String("GTKMAIN_C_VAR"));
    lexer->addSkipWord(QLatin1String("GTKVAR"));
    lexer->addSkipWord(QLatin1String("GDKVAR"));
    lexer->addSkipWord(QLatin1String("G_GNUC_PRINTF"), SkipWordAndArguments);

    // windows
    lexer->addSkipWord(QLatin1String("WINAPI"));
    lexer->addSkipWord(QLatin1String("__stdcall"));
    lexer->addSkipWord(QLatin1String("__cdecl"));
    lexer->addSkipWord(QLatin1String("_cdecl"));
    lexer->addSkipWord(QLatin1String("CALLBACK"));

    // gcc extensions
    if (!hasMacro("__asm__")) addMacro(Macro(QLatin1String("__asm__"), QLatin1String("asm")));
    if (!hasMacro("__inline")) addMacro(Macro(QLatin1String("__inline"), QLatin1String("inline")));
    if (!hasMacro("__inline__")) addMacro(Macro(QLatin1String("__inline__"), QLatin1String("inline")));
    if (!hasMacro("__const")) addMacro(Macro(QLatin1String("__const"), QLatin1String("const")));
    if (!hasMacro("__const__")) addMacro(Macro(QLatin1String("__const__"), QLatin1String("const")));
    if (!hasMacro("__volatile__")) addMacro(Macro(QLatin1String("__volatile__"), QLatin1String("volatile")));
    if (!hasMacro("__complex__")) addMacro(Macro(QLatin1String("__complex__"), QString()));

    // c++ macros
    if (!hasMacro("__cplusplus")) addMacro(Macro(QLatin1String("__cplusplus"), QString()));
}

void Driver::setupParser(Parser * parser)
{
    Q_UNUSED(parser);
}

void Driver::clearIncludePaths()
{
    m_includePaths.clear();
}

void Driver::addIncludePath(const QString &path)
{
    if (!path.trimmed().isEmpty())
        m_includePaths << path;
}

QString Driver::findIncludeFile(const Dependence& dep, const QString& fromFile)
{
    QString fileName = dep.first;

    if (dep.second == Dep_Local) {
        QString path = QFileInfo(fromFile).absolutePath();
        QFileInfo fileInfo(QFileInfo(path, fileName));
        if (fileInfo.exists() && fileInfo.isFile())
            return fileInfo.absoluteFilePath();
    }

    QStringList includePaths = getCustomIncludePath(fromFile);

    for (QStringList::ConstIterator it = includePaths.constBegin(); it != includePaths.constEnd(); ++it) {
        QFileInfo fileInfo(*it, fileName);
        if (fileInfo.exists() && fileInfo.isFile())
            return fileInfo.absoluteFilePath();
    }

    return QString();
}

QString Driver::findIncludeFile(const Dependence& dep) const
{
    QString fileName = dep.first;

    if (dep.second == Dep_Local) {
        QString path = QFileInfo(currentFileName()).absolutePath();
        QFileInfo fileInfo(QFileInfo(path, fileName));
        if (fileInfo.exists() && fileInfo.isFile())
            return fileInfo.absoluteFilePath();
    }

    for (QStringList::ConstIterator it = m_includePaths.begin(); it != m_includePaths.end(); ++it) {
        QFileInfo fileInfo(*it, fileName);
        if (fileInfo.exists() && fileInfo.isFile())
            return fileInfo.absoluteFilePath();
    }

    return QString();
}

void Driver::setResolveDependencesEnabled(bool enabled)
{
    depresolv = enabled;
    if (depresolv)
        setupPreProcessor();
}

bool Driver::shouldParseIncludedFile(const ParsedFilePointer& /*file*/)
{
    return false;
}

void Driver::setupPreProcessor() {}

void Driver::fileParsed(ParsedFile & fileName)
{
    Q_UNUSED(fileName);
}

void Driver::usingMacro(const Macro& macro)
{
    if (m_currentParsedFile)
        m_currentParsedFile->usedMacros().addMacro(macro);
#ifdef CACHELEXER
    if (m_currentLexerCache)
        m_currentLexerCache->addUsedMacro(macro);
#endif
}

// void Macro::computeHash() const {
//     m_idHash = 7 * (HashedString::hashString(m_name) + m_argumentList.count() * 13);
//     int a = 1;
//     m_idHash += 31 * m_argumentList.count();
//
//     m_valueHash = 27 * (HashedString::hashString(m_body) +  (m_isUndefMacro ? 1 : 0));
//
//     for(QList<Argument>::const_iterator it = m_argumentList.begin(); it != m_argumentList.end(); ++it) {
//         a *= 19;
//         m_valueHash += a * HashedString::hashString(*it);
//     }
//     m_valueHashValid = true;
//     m_idHashValid = true;
// }

// MacroSet::MacroSet() : m_idHashValid(false), m_valueHashValid(false) {
// }

void MacroSet::addMacro(const Macro& macro)
{
    std::pair<Macros::const_iterator, bool> r = m_usedMacros.insert(macro);
    if (!r.second) {
        //Make sure the macro added later will be used
        m_usedMacros.erase(r.first);
        m_usedMacros.insert(macro);
    }

    m_idHashValid = m_valueHashValid = false;
}

void MacroSet::merge(const MacroSet& macros)
{
    Macros m = macros.m_usedMacros; //Swap is needed so the merged macros take precedence
    m.insert(m_usedMacros.begin(), m_usedMacros.end());
    m_usedMacros = m;
    m_idHashValid = m_valueHashValid = false;
}


size_t MacroSet::idHash() const
{
    if (!m_idHashValid) computeHash();
    return m_idHash;
}

size_t MacroSet::valueHash() const
{
    if (!m_valueHashValid) computeHash();
    return m_valueHash;
}

void MacroSet::computeHash() const
{
    m_idHash = 0;
    m_valueHash = 0;
    int mult = 1;
    for (Macros::const_iterator it = m_usedMacros.begin(); it != m_usedMacros.end(); ++it) {
        mult *= 31;
        m_idHash += (*it).idHash();
        m_valueHash += (*it).valueHash();
    }
}

// void MacroSet::read(QDataStream& stream)  {
//     stream >> m_idHashValid >> m_idHash >> m_valueHashValid >> m_valueHash;
//     int cnt;
//     stream >> cnt;
//     m_usedMacros.clear();
//     Macro m;
//     for(int a = 0; a < cnt; a++) {
//         m.read(stream);
//         m_usedMacros.insert(m);
//     }
// }
//
// void MacroSet::write(QDataStream& stream) const {
//     stream << m_idHashValid << m_idHash << m_valueHashValid << m_valueHash;
//     stream << m_usedMacros.size();
//     for(Macros::const_iterator it = m_usedMacros.begin(); it != m_usedMacros.end(); ++it) {
//         (*it).write(stream);
//     }
// }

/**
 * @return All Macros that were used while processing this translation-unit
 * */
MacroSet& ParsedFile::usedMacros()
{
    return m_usedMacros;
}

const MacroSet& ParsedFile::usedMacros() const
{
    return m_usedMacros;
}

ParsedFile::ParsedFile(const QString& fileName, const QDateTime& timeStamp)
  : m_translationUnit(nullptr),
    m_skippedLines(0),
    m_fileName(fileName),
    m_timeStamp(timeStamp)
{
    m_includeFiles.insert(fileName);
}

ParsedFile::ParsedFile(const QByteArray& array)
  : m_translationUnit(nullptr),
    m_skippedLines(0)
{
    QByteArray a(array);
    QBuffer b(&a);
    QDataStream d(&b);
    read(d);
}

QString ParsedFile::includedFrom() const
{
    return m_includedFrom;
}

void ParsedFile::setIncludedFrom(const QString& str)
{
    m_includedFrom = str;
}

QByteArray ParsedFile::serialize() const
{
    QByteArray array;
    QBuffer b(&array);
    QDataStream d(&b);
    write(d);
    return array;
}

// void ParsedFile::read(QDataStream& stream) {
//     int directIncludeFilesCount;
//     stream >> directIncludeFilesCount;
//     m_directIncludeFiles.clear();
//     for(int a = 0; a < directIncludeFilesCount; a++) {
//         IncludeDesc i;
//         stream >> i.local;
//         stream >> i.includePath;
//         //"parsed" will not be reconstructed
//         m_directIncludeFiles.push_back(i);
//     }
//     stream >> m_fileName;
//     stream >> m_timeStamp;
//     m_usedMacros.read(stream);
//     m_translationUnit = 0;
//     m_includeFiles.read(stream);
// }
//
// void ParsedFile::write(QDataStream& stream) const {
//     for(QList<IncludeDesc>::const_iterator it = m_directIncludeFiles.begin(); it != m_directIncludeFiles.end(); ++it) {
//         stream << (*it).local;
//         stream << (*it).includePath;
//     }
//     stream << m_fileName;
//     stream << m_timeStamp;
//     m_usedMacros.write(stream);
//     m_includeFiles.write(stream);
// }

void ParsedFile::setTranslationUnit(const TranslationUnitAST::Node& trans)
{
    m_translationUnit = trans;
}

// HashedStringSet& ParsedFile::includeFiles() {
//     return m_includeFiles;
// }

int ParsedFile::skippedLines() const
{
    return m_skippedLines;
}

void ParsedFile::setSkippedLines(int lines)
{
    m_skippedLines = lines;
}

const HashedStringSet& ParsedFile::includeFiles() const
{
    return m_includeFiles;
}

QString ParsedFile::fileName() const
{
    return m_fileName;
}

QDateTime ParsedFile::timeStamp() const
{
    return m_timeStamp;
}

void ParsedFile::addIncludeFiles(const HashedStringSet& includeFiles)
{
    m_includeFiles += includeFiles;
}

void ParsedFile::addIncludeFile(const QString& includePath, const ParsedFilePointer& parsed, bool localInclude)
{
    m_includeFiles.insert(includePath);
    if (parsed)
        m_includeFiles += parsed->includeFiles();
    IncludeDesc d;
    d.local = localInclude;
    d.includePath = includePath;
    d.parsed = parsed;
    m_directIncludeFiles << d;
}

const QList<ParsedFile::IncludeDesc>& ParsedFile::directIncludeFiles() const
{
    return m_directIncludeFiles;
}

bool MacroSet::hasMacro(const QString& name) const
{
    Macros::const_iterator it = m_usedMacros.find(Macro(name, QString()));
    if (it != m_usedMacros.end()) {
        return true;
    } else {
        return false;
    }
}

bool MacroSet::hasMacro(const HashedString& name) const
{
    Macros::const_iterator it = m_usedMacros.find(Macro(name.str(), QString()));
    if (it != m_usedMacros.end()) {
        return true;
    } else {
        return false;
    }
}

Macro MacroSet::macro(const QString& name) const
{
    Macros::const_iterator it = m_usedMacros.find(Macro(name, QString()));

    if (it != m_usedMacros.end()) {
        return *it;
    } else {
        return Macro();
    }
}

LexerCache* Driver::lexerCache()
{
    return &m_lexerCache;
}

