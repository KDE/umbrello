/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "structuretype.h"

#include <language/duchain/types/typeregister.h>

using namespace KDevelop;

namespace Php
{
REGISTER_TYPE(StructureType);

StructureType::StructureType(const StructureType& rhs)
  : KDevelop::StructureType(copyData<StructureType>(*rhs.d_func()))
{
}

StructureType::StructureType(StructureTypeData& data)
  : KDevelop::StructureType(data)
{
}

StructureType::StructureType()
  : KDevelop::StructureType(createData<StructureType>())
{
}

QString StructureType::toString() const
{
    if ( d_func()->prettyName.isEmpty() ) {
        return KDevelop::StructureType::toString();
    } else {
        return prettyName().str();
    }
}

KDevelop::IndexedString StructureType::prettyName() const
{
    return d_func()->prettyName;
}

void StructureType::setPrettyName( const KDevelop::IndexedString& name )
{
    d_func_dynamic()->prettyName = name;
}

KDevelop::AbstractType* StructureType::clone() const
{
    return new StructureType(*this);
}

uint StructureType::hash() const
{
    return 4 * KDevelop::StructureType::hash();
}

}
