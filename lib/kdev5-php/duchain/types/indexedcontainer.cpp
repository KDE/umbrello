/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2011 Sven Brauch <svenbrauch@googlemail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "indexedcontainer.h"

#include <language/duchain/types/typeregister.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>

#include <KLocalizedString>

using namespace KDevelop;

namespace Php {

DEFINE_LIST_MEMBER_HASH(IndexedContainerData, m_values, IndexedType)
REGISTER_TYPE(IndexedContainer);

IndexedContainer::IndexedContainer() : Php::StructureType(createData<IndexedContainer>())
{

}

IndexedContainer::IndexedContainer(const IndexedContainer& rhs)
    : StructureType(copyData<IndexedContainer>(*rhs.d_func()))
{

}

IndexedContainer::IndexedContainer(IndexedContainerData& data)
    : StructureType(data)
{

}

void IndexedContainer::addEntry(AbstractType::Ptr typeToAdd)
{
    Q_ASSERT(typeToAdd && "trying to add a null type to indexedContainer");
    d_func_dynamic()->m_valuesList().append(typeToAdd->indexed());
}

const IndexedType& IndexedContainer::typeAt(int index) const
{
    Q_ASSERT((uint) index < d_func()->m_valuesSize());
    return d_func()->m_values()[index];
}

void IndexedContainer::replaceType(int index, AbstractType::Ptr newType)
{
    Q_ASSERT((uint) index < d_func()->m_valuesSize());
    d_func_dynamic()->m_valuesList()[index] = newType->indexed();
}

KDevelop::AbstractType* IndexedContainer::clone() const
{
    auto* n = new IndexedContainer(*this);
    return n;
}

QString IndexedContainer::toString() const
{
    QString prefix = Php::StructureType::toString();
    QStringList typesArray;
    for ( int i = 0; i < typesCount(); i++ ) {
        if ( i >= 5 ) {
            // Don't print more than five types explicitly
            typesArray << "...";
            break;
        }
        typesArray << typeAt(i).abstractType()->toString();
    }
    const QString contentType = QStringLiteral("(") + typesArray.join(", ") + ")";
    return i18nc("as in list of int, set of string", "%1 of %2", prefix, contentType);
}

QString IndexedContainer::containerToString() const
{
    return Php::StructureType::toString();
}

int IndexedContainer::typesCount() const
{
    return d_func()->m_valuesSize();
}

bool IndexedContainer::equals(const AbstractType* rhs) const
{
    if ( this == rhs ) {
        return true;
    }
    if ( ! Php::StructureType::equals(rhs) ) {
        return false;
    }
    const IndexedContainer* c = dynamic_cast<const IndexedContainer*>(rhs);
    if ( ! c ) {
        return false;
    }
    if ( typesCount() != c->typesCount() ) {
        return false;
    }
    for ( int i = 0; i < typesCount(); i++ ) {
        if ( c->typeAt(i) != typeAt(i) ) {
            return false;
        }
    }
    return true;
}

uint IndexedContainer::hash() const
{
    uint h = StructureType::hash();
    for ( uint i = 0; i < d_func()->m_valuesSize(); i++ ) {
        h += i*d_func()->m_values()[i];
    }
    return h;
}

}
