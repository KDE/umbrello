/*
    SPDX-FileCopyrightText: 2006 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HASHED_STRING_H
#define HASHED_STRING_H

#include "ast.h"

#include <QHash>
#include <QSet>
#include <QString>
#include <qdatastream.h>
#include <set>
#include <string>

///A simple class that stores a string together with it's appropriate hash-key
class HashedString
{
public:
    HashedString() : m_hash(0) {}

    HashedString(const QString& str) : m_str(str) // krazy:exclude=explicit
    {
        initHash();
    }

    HashedString(const char* str) : m_str(QLatin1String(str)) // krazy:exclude=explicit
    {
        initHash();
    }

    inline size_t hash() const
    {
        return m_hash;
    }

    QString str() const
    {
        return m_str;
    }

    bool operator == (const HashedString& rhs) const
    {
        if (m_hash != rhs.m_hash)
            return false;
        return m_str == rhs.m_str;
    }

    ///Does not compare alphabetically, uses the hash-key for ordering.
    bool operator < (const HashedString& rhs) const
    {
        if (m_hash < rhs.m_hash)
            return true;
        if (m_hash == rhs.m_hash)
            return m_str < rhs.m_str;
        return false;
    }

    static size_t hashString(const QString& str);

private:
    void initHash();

    QString m_str;
    size_t m_hash;

    friend QDataStream& operator << (QDataStream& stream, const HashedString& str);
    friend QDataStream& operator >> (QDataStream& stream, HashedString& str);
};

QDataStream& operator << (QDataStream& stream, const HashedString& str);

QDataStream& operator >> (QDataStream& stream, HashedString& str);

class HashedStringSetData;
class HashedStringSetGroup;

///This is a reference-counting string-set optimized for fast lookup of hashed strings
class HashedStringSet
{
public:
    HashedStringSet();

    ~HashedStringSet();

    ///Constructs a string-set from one single file
    explicit HashedStringSet(const HashedString& file);

    HashedStringSet(const HashedStringSet& rhs);

    int size() const;

    HashedStringSet& operator = (const HashedStringSet& rhs);
    ///@return whether the given file-name was included
    bool operator[] (const HashedString& rhs) const;

    void insert(const HashedString& str);

    HashedStringSet& operator +=(const HashedStringSet&);

    HashedStringSet& operator -=(const HashedStringSet&);

    ///intersection-test
    ///Returns true if all files that are part of this set are also part of the given set
    bool operator <= (const HashedStringSet& rhs) const;

    bool operator == (const HashedStringSet& rhs) const;

    void read(QDataStream& stream);
    void write(QDataStream& stream) const;

    std::string print() const;

    size_t hash() const;
private:
    friend class HashedStringSetGroup;
    void makeDataPrivate();
    KSharedPtr<HashedStringSetData> m_data; //this implies some additional cost because KShared's destructor is virtual. Maybe change that by copying KShared without the virtual destructor.
    friend HashedStringSet operator + (const HashedStringSet& lhs, const HashedStringSet& rhs);
};

HashedStringSet operator + (const HashedStringSet& lhs, const HashedStringSet& rhs);

///Used to find all registered HashedStringSet's that contain all strings given to findGroups(..)
class HashedStringSetGroup
{
public:
    typedef std::set<size_t> ItemSet;
    void addSet(size_t id, const HashedStringSet& set);
    void enableSet(size_t id);
    bool isDisabled(size_t id) const;
    void disableSet(size_t id);
    void removeSet(size_t id);

    //Writes the ids of all registered and not disabled HashedStringSet's that are completely included in the given HashedStringSet efficiently)
    void findGroups(HashedStringSet strings, ItemSet& target) const;

private:
    typedef QHash<HashedString, ItemSet> GroupMap;
    typedef QHash<size_t, size_t> SizeMap;
    GroupMap m_map;
    SizeMap m_sizeMap;
    ItemSet m_disabled;
    ItemSet m_global;
};

uint qHash(const HashedString &key);

#endif
