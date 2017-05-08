/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>
    Copyright 2008 Niko Sams <niko.sams@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef EXPRESSIONEVALUATIONRESULT_H
#define EXPRESSIONEVALUATIONRESULT_H

#include "phpduchainexport.h"
#include <language/duchain/appendedlist.h>
#include <language/duchain/declarationid.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/referencetype.h>
#include <language/duchain/types/indexedtype.h>

#include <QList>

namespace KDevelop
{
class IndexedType;
class TypeIdentifier;
class Declaration;
}

class QString;

namespace Php
{

class KDEVPHPDUCHAIN_EXPORT ExpressionEvaluationResult
{
public:
    ExpressionEvaluationResult();
    ~ExpressionEvaluationResult();

    ///NOTE: Chain needs to be locked when calling this
    void setDeclaration(KDevelop::Declaration* declaration);
    void setDeclaration(KDevelop::DeclarationPointer declaration);
    ///NOTE: Chain needs to be locked when calling this
    void setDeclarations(QList<KDevelop::Declaration*> declarations);
    void setDeclarations(QList<KDevelop::DeclarationPointer> declarations);
    void setType(KDevelop::AbstractType::Ptr type);
    void setHadUnresolvedIdentifiers(bool v);

    KDevelop::AbstractType::Ptr type() const;
    QList<KDevelop::DeclarationId> allDeclarationIds() const;
    QList<KDevelop::DeclarationPointer> allDeclarations() const;
    bool hadUnresolvedIdentifiers() const;

private:
    QList<KDevelop::DeclarationPointer> m_allDeclarations;
    QList<KDevelop::DeclarationId> m_allDeclarationIds;
    KDevelop::AbstractType::Ptr m_type; ///Type the expression evaluated to, may be zero when the expression failed to evaluate
    bool m_hadUnresolvedIdentifiers;

    //bool isInstance; ///Whether the result of this expression is an instance(as it normally should be)
    //KDevelop::DeclarationId instanceDeclaration; ///If this expression is an instance of some type, this either contains the declaration of the instance, or the type
};

}

#endif
