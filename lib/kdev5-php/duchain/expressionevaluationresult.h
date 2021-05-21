/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
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
