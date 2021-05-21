/*
    SPDX-FileCopyrightText: 2006 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MACRO_H
#define MACRO_H

#include <qpair.h>
#include <QStringList>
#include <QString>
#include <qdatastream.h>
#include <qmap.h>
#include <qdatetime.h>
#include <map>
#include <set>
#include <hashedstring.h>
#include <kdatastream.h>
#include <codemodel.h>

typedef signed char Q_INT8;

//This files should be renamed to something like "helpers.h"

/**
 * Encapsulates a problem in a piece of source code.
 */
class Problem
{
public:
    enum {
        Level_Error = 0,   ///< Indicates an error that will prevent the code from compiling
        Level_Warning,   ///< Indicates a warning
        Level_Todo,   ///< Indicates there is still something left to do
        Level_Fixme ///< Indicates that something needs to be fixed
    };

public:
    Problem() {}
    Problem(const Problem& source)
        : m_text(source.m_text), m_line(source.m_line),
          m_column(source.m_column), m_level(source.m_level), m_file(source.m_file) {}
    Problem(const QString& text, int line, int column, int level = Level_Error)
        : m_text(text), m_line(line), m_column(column), m_level(level) {}

    Problem(const Problem& source, bool /*threadSafeClone*/)
        : m_text(QString::fromUtf8(source.m_text.toUtf8().data())), m_line(source.m_line),
          m_column(source.m_column), m_level(source.m_level), m_file(QString::fromUtf8(source.m_file.toUtf8().data())) {}

    Problem& operator = (const Problem& source)
    {
        m_text = source.m_text;
        m_line = source.m_line;
        m_column = source.m_column;
        m_level = source.m_level;
        m_file = source.m_file;
        return (*this);
    }

    void setFileName(const QString& fileName)
    {
        m_file = fileName;
    }

    bool operator == (const Problem& p) const
    {
        return m_text == p.m_text && m_line == p.m_line && m_column == p.m_column && m_level == p.m_level && m_file == p.m_file;
    }

    /** Get the filename in which the problem was encountered */
    QString fileName() const
    {
        return m_file;
    }

    /** Get the text for the problem */
    QString text() const
    {
        return m_text;
    }
    /** Get the line number of the problem */
    int line() const
    {
        return m_line;
    }
    /** Get the column of the problem */
    int column() const
    {
        return m_column;
    }
    /**
     * Get the seriousness of the problem. There are four possibilities:
     * \li Error
     * \li Warning
     * \li Todo
     * \li Fixme
     */
    int level() const
    {
        return m_level;
    }

private:
    QString m_text;
    int m_line;
    int m_column;
    int m_level;
    QString m_file;
};


/**
 * A datatype that represents a preprocessor macro.
 * Most of the functions in this class need to be inline, so we do not have to import cppparser to many modules. The other solution would be moving macro into interfaces.
 */
class Macro
{
public:
    typedef QString Argument;

public:
    explicit Macro(bool hasArguments = false)
        : m_idHashValid(false),
          m_valueHashValid(false),
          m_idHash(0),
          m_valueHash(0),
          m_line(0),
          m_column(0),
          m_hasArguments(hasArguments),
          m_isUndefMacro(false)
    {}
    Macro(const QString &n, const QString &b)
        : m_idHashValid(false),
          m_valueHashValid(false),
          m_idHash(0),
          m_valueHash(0),
          m_name(n),
          m_line(0),
          m_column(0),
          m_body(b),
          m_hasArguments(false),
          m_isUndefMacro(false)
    {}

    //Sorts the macros by their hash-value, then by their name.
    class NameArgCompare {
    public:
        bool operator () (const Macro& lhs, const Macro& rhs) const
        {
            size_t lhash = lhs.idHash();
            size_t rhash = rhs.idHash();
            if (lhash < rhash) return true;
            else if (lhash > rhash) return false;

            int df = lhs.m_name.compare(rhs.m_name);
            if (df < 0)
                return true;
            if (df == 0) {
                if (!lhs.m_hasArguments && rhs.m_hasArguments) {
                    return true;
                } else if (lhs.m_hasArguments == rhs.m_hasArguments) {
                    return lhs.m_argumentList.count() < rhs.m_argumentList.count();

                } else {
                    return false;
                }
            }
            return false;
        }
    };
    class NameCompare {
    public:
        bool operator () (const Macro& lhs, const Macro& rhs) const
        {
            size_t lhash = lhs.idHash();
            size_t rhash = rhs.idHash();
            if (lhash < rhash) return true;
            else if (lhash > rhash) return false;

            int df = lhs.m_name.compare(rhs.m_name);
            return df < 0;
        }
    };

    class NameArgHash {
    public:
        size_t operator () (const Macro& macro) const
        {
            return macro.idHash();
        }
    };

    class NameArgEqual {
    public:
        bool operator () (const Macro& lhs, const Macro& rhs) const
        {
            int df = lhs.m_name.compare(rhs.m_name);
            if (df == 0) {
                if (lhs.m_hasArguments != rhs.m_hasArguments) {
                    return false;
                } else {
                    if (lhs.m_argumentList.count() != rhs.m_argumentList.count()) return false;
                    /*QStringList::const_iterator it2 = rhs.m_argumentList.begin();
                    for(QStringList::const_iterator it = lhs.m_argumentList.begin(); it != lhs.m_argumentList.end();) {
                        if(*it != *it2) return false;

                        ++it;
                        ++it2;
                    }*/
                    return true;

                }
            }
            return false;
        }
    };

    Macro(const Macro& source)
        : m_idHashValid(source.m_idHashValid),
          m_valueHashValid(source.m_valueHashValid),
          m_idHash(source.m_idHash),
          m_valueHash(source.m_valueHash),
          m_name(source.m_name),
          m_fileName(source.m_fileName),
          m_line(source.m_line),
          m_column(source.m_column),
          m_body(source.m_body),
          m_hasArguments(source.m_hasArguments),
          m_argumentList(source.m_argumentList),
          m_isUndefMacro(source.m_isUndefMacro)
    {}

    Macro& operator = (const Macro& source)
    {
        m_idHashValid = source.m_idHashValid;
        m_valueHashValid = source.m_valueHashValid;
        m_idHash = source.m_idHash;
        m_valueHash = source.m_valueHash;
        m_name = source.m_name;
        m_fileName = source.m_fileName;
        m_line = source.m_line;
        m_column = source.m_column;
        m_body = source.m_body;
        m_hasArguments = source.m_hasArguments;
        m_argumentList = source.m_argumentList;
        m_isUndefMacro = source.m_isUndefMacro;
        return *this;
    }

    bool operator == (const Macro& source) const
    {
        if (!m_idHashValid || !m_valueHashValid) computeHash();
        if (!source.m_idHashValid || !source.m_valueHashValid) source.computeHash();

        if (m_idHash != source.m_idHash) return false;
        if (m_valueHash != source.m_valueHash) return false;

        return m_name == source.m_name &&
               m_fileName == source.m_fileName &&
               m_body == source.m_body &&
               m_hasArguments == source.m_hasArguments &&
               m_argumentList == source.m_argumentList && m_isUndefMacro == source.m_isUndefMacro;
    }

    void read(QDataStream& stream)
    {
        Q_INT8 i;
        stream >> i;
        m_idHashValid = i;
        stream >> i;
        m_valueHashValid = i;
        stream >> i;
        m_hasArguments = i;

        stream >> m_idHash;
        stream >> m_valueHash;
        stream >> m_name;
        stream >> m_line;
        stream >> m_column;
        stream >> m_body;
        stream >> m_fileName;
        stream >> m_argumentList;
    }

    void write(QDataStream& stream) const
    {
        Q_INT8 i;
        i = m_idHashValid;
        stream << i;
        i = m_valueHashValid;
        stream << i;
        i = m_hasArguments;
        stream << i;

        stream << m_idHash;
        stream << m_valueHash;
        stream << m_name;
        stream << m_line;
        stream << m_column;
        stream << m_body;
        stream << m_fileName;
        stream << m_argumentList;
    }

    /** Get the name for this macro */
    QString name() const
    {
        return m_name;
    }
    /** Set the name for this macro */
    void setName(const QString& name)
    {
        m_name = name;
        invalidateHash();
    }

    /** Get the file name that contains this macro */
    QString fileName() const
    {
        return m_fileName;
    }
    /** Set the file name that contains this macro */
    void setFileName(const QString& fileName)
    {
        m_fileName = fileName;
        invalidateHash();
    }

    /** Get the line the macro is defined on */
    int line() const
    {
        return m_line;
    }
    /** Set the line the macro is defined on */
    void setLine(int line)
    {
        m_line = line;
    }

    /** Get the column the macro starts at */
    int column() const
    {
        return m_column;
    }
    /** Set the column the macro starts at */
    void setColumn(int column)
    {
        m_column = column;
    }

    /** Get the body of the macro */
    QString body() const
    {
        return m_body;
    }
    /** Set the body of the macro */
    void setBody(const QString& body)
    {
        m_body = body;
        invalidateHash();
    }

    /** This is used so the lexer does not have to remove macros that should really stay(they are just temporarily shadowed by an isUndef-macro */
    bool isUndef() const
    {
        return m_isUndefMacro;
    };

    void setUndef()
    {
        m_isUndefMacro = true;
        invalidateHash();
    };

    /** Check whether the macro has arguments that are passed to it */
    bool hasArguments() const
    {
        return m_hasArguments;
    }
    void setHasArguments(bool hasArguments)
    {
        m_hasArguments = hasArguments;
        invalidateHash();
    }
    /** Get a list of arguments passed to this macro */
    QList<Argument> argumentList() const
    {
        return m_argumentList;
    }

    /** Clear the list of arguments this macro has */
    void clearArgumentList()
    {
        m_argumentList.clear();
        m_hasArguments = false;
        invalidateHash();
    }
    /** Add an argument to this macro */
    void addArgument(const Argument& argument)
    {
        m_argumentList << argument;
    }
    /** Add a list of arguments to this macro */
    void addArgumentList(const QList<Argument>& arguments)
    {
        m_argumentList += arguments;
        invalidateHash();
    }

    ///This hash respects macro-name and argument-count
    size_t idHash() const
    {
        if (!m_idHashValid) computeHash();
        return m_idHash;
    }

    ///This hash respects body and if it is an undef-macro
    size_t valueHash() const
    {
        if (!m_valueHashValid) computeHash();
        return m_valueHash;
    }

private:
    inline void invalidateHash() const
    {
        m_idHashValid = m_valueHashValid = false;
    }

    void computeHash() const
    {
        m_idHash = 7 * (HashedString::hashString(m_name));
        int a = 1;
        //m_idHash += 31 * m_argumentList.count();

        m_valueHash = 27 * (HashedString::hashString(m_body) +  (m_isUndefMacro ? 1 : 0));

        for (QList<Argument>::const_iterator it = m_argumentList.begin(); it != m_argumentList.end(); ++it) {
            a *= 19;
            m_valueHash += a * HashedString::hashString(*it);
        }
        m_valueHashValid = true;
        m_idHashValid = true;
    }

    mutable bool m_idHashValid;
    mutable bool m_valueHashValid;
    mutable size_t m_idHash; //Hash that represents the ids of all macros
    mutable size_t m_valueHash; //Hash that represents the values of all macros

    QString m_name;
    QString m_fileName;
    int m_line;
    int m_column;
    QString m_body;
    bool m_hasArguments;
    QStringList m_argumentList; //While identification, only the count plays a role, not the values.
    bool m_isUndefMacro;
    friend class NameCompare;
    friend class NameArgEqual;
};

class MacroSet
{
public:
    //typedef __gnu_cxx::hash_set< Macro, Macro::NameArgHash, Macro::NameArgEqual > Macros;
    typedef std::set< Macro, Macro::NameCompare > Macros;
    MacroSet()
      : m_idHashValid(false),
        m_valueHashValid(false),
        m_idHash(0),
        m_valueHash(0)
    {
    }

    void addMacro(const Macro& macro);

    void read(QDataStream& stream)
    {
        //stream >> m_idHashValid >> m_idHash >> m_valueHashValid >> m_valueHash;
        m_idHashValid = false;
        m_valueHashValid = false;
        int cnt;
        stream >> cnt;
        m_usedMacros.clear();
        Macro m;
        for (int a = 0; a < cnt; a++) {
            m.read(stream);
            m_usedMacros.insert(m);
        }
    }

    void write(QDataStream& stream) const
    {
        //stream << m_idHashValid << m_idHash << m_valueHashValid << m_valueHash;
        stream << int(m_usedMacros.size());
        for (Macros::const_iterator it = m_usedMacros.begin(); it != m_usedMacros.end(); ++it) {
            (*it).write(stream);
        }
    }

    bool hasMacro(const QString& name) const;
    bool hasMacro(const HashedString& name) const;
    Macro macro(const QString& name) const;

    size_t idHash() const;
    size_t valueHash() const;

    const Macros& macros() const
    {
        return m_usedMacros;
    }

    void merge(const MacroSet& macros);
private:
    void computeHash() const;
    Macros m_usedMacros;
    mutable bool m_idHashValid;
    mutable bool m_valueHashValid;
    mutable size_t m_idHash; //Hash that represents the ids of all macros
    mutable size_t m_valueHash; //Hash that represents the values of all macros

    friend class Driver;
};

#endif
