/*
    SPDX-FileCopyrightText: 2006 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "lexercache.h"

#define DBG_SRC QLatin1String("LexerCache")
#include "debug_utils.h"
#include "driver.h"

DEBUG_REGISTER_DISABLED(LexerCache)

LexerCache::LexerCache(Driver* d) : m_driver(d) {}

void LexerCache::addLexedFile(const CachedLexedFilePointer& file)
{
    DEBUG() << "adding an instance of " << file->fileName().str() ;

    std::pair< CachedLexedFileMap::iterator, CachedLexedFileMap::iterator> files = m_files.equal_range(file->fileName());

    if (files.first == files.second) {
        m_files.insert(std::make_pair(file->fileName(), file));
    } else {
        //Make sure newer files appear first
        m_files.insert(files.first, std::make_pair(file->fileName(), file));
    }

    int cnt = 0;
    while (files.first != files.second) {
        if (sourceChanged(*(*(files.first)).second)) {
            m_files.erase(files.first++);
        } else {
            cnt++;
            files.first++;
        }
    }
    DEBUG() << "new count of cached instances for the file: " << cnt ;
}

CachedLexedFilePointer LexerCache::lexedFile(const HashedString& fileName)
{
    initFileModificationCache();
    std::pair< CachedLexedFileMap::iterator, CachedLexedFileMap::iterator> files = m_files.equal_range(fileName);

    ///@todo optimize with standard-algorithms(by first computing the intersection)

    /*  if(files.first != files.second)
          DEBUG() << "LexerCache: cache for file " << fileName.str() << " is not empty" ;
      else
          DEBUG() << "LexerCache: cache for file " << fileName.str() << " is empty" ;*/

    while (files.first != files.second) {
        const CachedLexedFile& file(*(*(files.first)).second);
        if (sourceChanged(file)) {
            DEBUG() <<  "cache for file " << fileName.str() << " is being discarded because the file was modified" ;
            m_files.erase(files.first++);
            continue;
        }
        bool success = true;
        //Make sure that none of the macros stored in the driver affect the file in a different way than the one before
        Driver::MacroMap::const_iterator end = m_driver->macros().end();
        for (Driver::MacroMap::const_iterator rit = m_driver->macros().begin(); rit != end;) {
            Driver::MacroMap::const_iterator it = rit;
            ++rit;
            if (rit != end && (*it).first == (*rit).first) continue; //Always only use the last macro of the same name for comparison, it is on top of the macro-stack
            if ((*it).second.isUndef()) continue; //Undef-macros theoretically don't exist

            if (file.hasString((*it).first)) {
                if (file.m_usedMacros.hasMacro((*it).first)) {
                    Macro m(file.m_usedMacros.macro((*it).first.str()));
                    if (!(m == (*it).second)) {
                        DEBUG() << "The cached file " << fileName.str() << " depends on the string \"" << (*it).first.str() << "\" and used a macro for it with the body \"" << m.body() << "\"(from " << m.fileName() << "), but the driver contains the same macro with body \"" << (*it).second.body() << "\"(from " << (*it).second.fileName() << "), cache is not used" ;

                        //Macro with the same name was used, but it is different
                        success = false;
                        break;
                    }

                } else {
                    //There is a macro that affects the file, but was not used while the previous parse
                    DEBUG() << "The cached file " << fileName.str() << " depends on the string \"" << (*it).first.str() << "\" and the driver contains a macro of that name with body \"" << (*it).second.body() << "\"(from " << (*it).second.fileName() << "), the cached file is not used" ;
                    success = false;
                    break;
                }
            }
        }
        //Make sure that all external macros used by the file now exist too
        MacroSet::Macros::const_iterator end2 = file.usedMacros().macros().end();
        for (MacroSet::Macros::const_iterator it = file.usedMacros().macros().begin(); it != end2; ++it) {
            if (!m_driver->hasMacro(HashedString((*it).name()))) {
                DEBUG() << "The cached file " << fileName.str() << " used a macro called \"" << it->name() << "\"(from " << it->fileName() << "), but the driver does not contain that macro, the cached file is not used" ;
                success = false;
                break;
            }
        }

        if (success) {
            DEBUG() << "Using cached file " << fileName.str() ;
            (*files.first).second->access();
            return (*files.first).second;
        }
        ++files.first;
    }
    return CachedLexedFilePointer();
}

QDateTime LexerCache::fileModificationTimeCached(const HashedString& fileName)
{
    FileModificationMap::const_iterator it = m_fileModificationCache.constFind(fileName);
    if (it != m_fileModificationCache.constEnd()) {
        ///Use the cache for 10 seconds
        if ((*it).m_readTime.secsTo(m_currentDateTime) < 10) {
            return (*it).m_modificationTime;
        }
    }

    QFileInfo fileInfo(fileName.str());
    m_fileModificationCache[fileName].m_readTime = QDateTime::currentDateTime();
    m_fileModificationCache[fileName].m_modificationTime = fileInfo.lastModified();
    return fileInfo.lastModified();

}

//Should be cached too!
bool LexerCache::sourceChanged(const CachedLexedFile& file)
{
    //@todo Check if any of the dependencies changed

    QDateTime modTime = fileModificationTimeCached(file.fileName());

    if (modTime != file.modificationTime())
        return true;

    for (QMap<HashedString, QDateTime>::const_iterator it = file.allModificationTimes().begin(); it != file.allModificationTimes().end(); ++it) {
        QDateTime modTime = fileModificationTimeCached(it.key());
        if (modTime != *it)
            return true;
    }

    return false;
}


void LexerCache::clear()
{
    m_files.clear();
    m_totalStringSet.clear();
    m_fileModificationCache.clear();
}

void LexerCache::erase(const CacheNode* node)
{
    std::pair< CachedLexedFileMap::iterator, CachedLexedFileMap::iterator> files = m_files.equal_range(((const CachedLexedFile*)(node))->fileName());
    while (files.first != files.second) {
        if ((*files.first).second == ((const CachedLexedFile*)(node))) {
            m_files.erase(files.first);
            return;
        }
        files.first++;
    }
    DEBUG() << "Error: could not find a node in the list for file " << ((const CachedLexedFile*)(node))->fileName().str() ;
}

CachedLexedFile::CachedLexedFile(const HashedString& fileName, LexerCache* manager) : CacheNode(manager), m_fileName(fileName)
{
    QFileInfo fileInfo(fileName.str());
    m_modificationTime = fileInfo.lastModified();
    m_allModificationTimes[ fileName ] = m_modificationTime;
}

void CachedLexedFile::addDefinedMacro(const Macro& macro)
{
    DEBUG() << "defined macro " << macro.name() ;

    m_definedMacros.addMacro(macro);
    m_definedMacroNames.insert(HashedString(macro.name()));
}

void CachedLexedFile::addUsedMacro(const Macro& macro)
{
    if (!m_definedMacros.hasMacro(macro.name())) {
        DEBUG() << "used macro " << macro.name() ;
        m_usedMacros.addMacro(macro);
    }
}

void CachedLexedFile::addIncludeFile(const HashedString& file, const QDateTime& modificationTime)
{
    m_includeFiles.insert(file);
    m_allModificationTimes[file] = modificationTime;
}


QDateTime CachedLexedFile::modificationTime() const
{
    return m_modificationTime;
}

void CachedLexedFile::addProblem(const Problem& p)
{
    m_problems << p;
}

QList<Problem>  CachedLexedFile::problems() const
{
    return m_problems;
}

//The parameter should be a CachedLexedFile that was lexed AFTER the content of this file
void CachedLexedFile::merge(const CachedLexedFile& file)
{
    DEBUG() << fileName().str() << ": merging " << file.fileName().str() << endl << "defined in this: " << m_definedMacroNames.print().c_str() << endl << "defined macros in other: " << file.m_definedMacroNames.print().c_str() ;;

    HashedStringSet tempStrings = file.m_strings;
    tempStrings -= m_definedMacroNames;
    m_strings += tempStrings;
    m_includeFiles += file.m_includeFiles;
    //Only add macros to the usedMacros-list that were not defined locally
    for (MacroSet::Macros::const_iterator it = file.m_usedMacros.macros().begin(); it != file.m_usedMacros.macros().end(); ++it) {
        if (!m_definedMacros.hasMacro((*it).name())) { ///If the macro was not defined locally, add it to the macros-list.
            DEBUG() << "inserting used macro " << (*it).name() ;
            m_usedMacros.addMacro(*it);
        }
    }

    m_definedMacros.merge(file.m_definedMacros);
    m_definedMacroNames += file.m_definedMacroNames;

    for (QMap<HashedString, QDateTime>::const_iterator it = file.m_allModificationTimes.begin(); it != file.m_allModificationTimes.end(); ++it)
        m_allModificationTimes[it.key()] = *it;

    DEBUG() << fileName().str() << ": defined in this after merge: " << m_definedMacroNames.print().c_str() ;

    m_problems += file.m_problems;
}

size_t CachedLexedFile::hash() const
{
    return m_usedMacros.valueHash() + m_usedMacros.idHash() + m_definedMacros.idHash() + m_definedMacros.valueHash() + m_strings.hash();
}

void LexerCache::initFileModificationCache()
{
    m_currentDateTime = QDateTime::currentDateTime();
}

void LexerCache::saveMemory()
{
    m_fileModificationCache.clear();

    m_totalStringSet.clear(); ///it's unclear how often this should be emptied. It may happen that completely unused strings end up in this set, then deleting it will save us memory.
}
