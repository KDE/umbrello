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

#include "integraltypeextended.h"

#include <language/duchain/types/typeregister.h>

using namespace KDevelop;

namespace Php
{

REGISTER_TYPE(IntegralTypeExtended);

IntegralTypeExtended::IntegralTypeExtended(const IntegralTypeExtended& rhs)
  : IntegralType(copyData<IntegralTypeExtended>(*rhs.d_func()))
{
}

IntegralTypeExtended::IntegralTypeExtended(IntegralTypeExtendedData& data)
  : IntegralType(data)
{
}

IntegralTypeExtended::IntegralTypeExtended(uint type)
  : IntegralType(createData<IntegralTypeExtended>())
{
  setDataType(type);
  setModifiers(ConstModifier);
}


QString IntegralTypeExtended::toString() const
{
    if ( d_func()->m_dataType == TypeResource ) {
        return QStringLiteral("resource");
    }
    return KDevelop::IntegralType::toString();
}

KDevelop::AbstractType* IntegralTypeExtended::clone() const
{
    return new IntegralTypeExtended(*this);
}

uint IntegralTypeExtended::hash() const
{
    return 4 * KDevelop::IntegralType::hash();
}

bool IntegralTypeExtended::equals(const KDevelop::AbstractType* rhs) const
{
    if( this == rhs ) {
        return true;
    }

    if ( !IntegralType::equals(rhs) ) {
        return false;
    }

    Q_ASSERT( fastCast<const IntegralTypeExtended*>(rhs) );

    const IntegralTypeExtended* type = static_cast<const IntegralTypeExtended*>(rhs);

    return d_func()->m_dataType == type->d_func()->m_dataType;
}

}
