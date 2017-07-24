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

#ifndef INTEGRALTYPEEXTENDED_H
#define INTEGRALTYPEEXTENDED_H

#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/typesystemdata.h>
#include "phpduchainexport.h"

namespace Php
{

typedef KDevelop::IntegralTypeData IntegralTypeExtendedData;

/**
 * Drop-In replacement for the IntegralType in KDevplatform with
 * some extended logic specific for PHP
 */
class KDEVPHPDUCHAIN_EXPORT IntegralTypeExtended: public KDevelop::IntegralType
{
public:
    typedef TypePtr<IntegralTypeExtended> Ptr;

    enum PHPIntegralTypes {
        TypeResource = KDevelop::IntegralType::TypeLanguageSpecific
    };

    /// Default constructor
    IntegralTypeExtended(uint type = TypeNone);
    /// Copy constructor. \param rhs type to copy
    IntegralTypeExtended(const IntegralTypeExtended& rhs);
    /// Constructor using raw data. \param data internal data.
    IntegralTypeExtended(IntegralTypeExtendedData& data);

    virtual QString toString() const;

    virtual KDevelop::AbstractType* clone() const;

    virtual bool equals(const KDevelop::AbstractType* rhs) const;

    virtual uint hash() const;

    enum {
        ///TODO: is that value OK?
        Identity = 50
    };

  typedef KDevelop::IntegralTypeData Data;
  typedef KDevelop::IntegralType BaseType;

protected:
    TYPE_DECLARE_DATA(IntegralTypeExtended);
};

}

namespace KDevelop
{

template<>
inline Php::IntegralTypeExtended* fastCast<Php::IntegralTypeExtended*>(AbstractType* from) {
    if ( !from || from->whichType() != AbstractType::TypeIntegral ) {
        return 0;
    } else {
        return dynamic_cast<Php::IntegralTypeExtended*>(from);
    }
}

}

#endif // PHPINTEGRALTYPE_H

