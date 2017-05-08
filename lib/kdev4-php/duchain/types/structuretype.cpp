/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2009 Milian Wolff <mail@milianw.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

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
