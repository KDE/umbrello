/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
        return "resource";
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
