/**
    This file is part of KDevelop
    Copyright (C) 2011 Sven Brauch <svenbrauch@googlemail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/


#ifndef INDEXEDCONTAINER_H
#define INDEXEDCONTAINER_H

#include "structuretype.h"
#include <language/duchain/types/typesystemdata.h>

#include "phpduchainexport.h"

using namespace KDevelop;

namespace Php {

KDEVPHPDUCHAIN_EXPORT DECLARE_LIST_MEMBER_HASH(IndexedContainerData, m_values, IndexedType)

class KDEVPHPDUCHAIN_EXPORT IndexedContainerData : public Php::StructureTypeData
{
public:
    /// Constructor
    IndexedContainerData()
        : Php::StructureTypeData()
    {
        initializeAppendedLists(m_dynamic);
    }
    /// Copy constructor. \param rhs data to copy
    IndexedContainerData( const IndexedContainerData& rhs )
        : Php::StructureTypeData(rhs)
    {
        initializeAppendedLists(m_dynamic);
        copyListsFrom(rhs);
    }

    ~IndexedContainerData() {
        freeAppendedLists();
    };

    START_APPENDED_LISTS_BASE(IndexedContainerData, StructureTypeData)
    APPENDED_LIST_FIRST(IndexedContainerData, IndexedType, m_values)
    END_APPENDED_LISTS(IndexedContainerData, m_values)
};


class KDEVPHPDUCHAIN_EXPORT IndexedContainer : public Php::StructureType
{
public:
    typedef TypePtr<IndexedContainer> Ptr;

    IndexedContainer();
    IndexedContainer(const IndexedContainer& rhs);
    IndexedContainer(IndexedContainerData& data);
    void addEntry(AbstractType::Ptr typeToAdd);
    AbstractType* clone() const override;
    uint hash() const override;
    int typesCount() const;
    const IndexedType& typeAt(int index) const;
    void replaceType(int index, AbstractType::Ptr newType);
    QString toString() const override;
    // "toString"s only the container type, not the content; used in declarationnavigationcontext to create
    // separate links for the content and container type
    // by keeping toString separate, it is possible to have a pretty type in mixed types etc. without additional
    // efforts being necessary
    QString containerToString() const;

    bool equals(const AbstractType* rhs) const override;

    enum {
        Identity = 52
    };

    typedef IndexedContainerData Data;

protected:
    TYPE_DECLARE_DATA(IndexedContainer);
};

}

#endif // INDEXEDCONTAINER_H
