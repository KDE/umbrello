/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHP_STRUCTURETYPE_H
#define PHP_STRUCTURETYPE_H

#include <language/duchain/types/structuretype.h>
#include <language/duchain/types/typesystemdata.h>
#include "phpduchainexport.h"

namespace Php
{

class KDEVPHPDUCHAIN_EXPORT StructureTypeData : public KDevelop::StructureTypeData
{
public:
    /// Constructor
    StructureTypeData()
        : KDevelop::StructureTypeData()
    {
    }
    /// Copy constructor. \param rhs data to copy
    StructureTypeData( const StructureTypeData& rhs )
        : KDevelop::StructureTypeData(rhs), prettyName(rhs.prettyName)
    {
    }

    KDevelop::IndexedString prettyName;
};

/**
 * Drop-In replacement for the StructureType in KDevplatform which
 * makes it possible to store the type as lower case but
 * keeping the "pretty" name intact.
 */
class KDEVPHPDUCHAIN_EXPORT StructureType: public KDevelop::StructureType
{
public:
    typedef KDevelop::TypePtr<StructureType> Ptr;

    /// Default constructor
    StructureType();
    /// Copy constructor. \param rhs type to copy
    StructureType(const StructureType& rhs);
    /// Constructor using raw data. \param data internal data.
    StructureType(StructureTypeData& data);

    void setPrettyName(const KDevelop::IndexedString& name);
    KDevelop::IndexedString prettyName() const;

    virtual QString toString() const;

    virtual KDevelop::AbstractType* clone() const;

    virtual uint hash() const;

    enum {
        ///TODO: is that value OK?
        Identity = 51
    };

  typedef StructureTypeData Data;
  typedef KDevelop::StructureType BaseType;

protected:
    TYPE_DECLARE_DATA(StructureType);
};

}

namespace KDevelop
{

template<>
inline Php::StructureType* fastCast<Php::StructureType*>(AbstractType* from) {
    if ( !from || from->whichType() != AbstractType::TypeStructure ) {
        return 0;
    } else {
        return dynamic_cast<Php::StructureType*>(from);
    }
}

}

#endif // PHP_STRUCTURETYPE_H

