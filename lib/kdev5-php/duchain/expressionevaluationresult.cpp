/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "expressionevaluationresult.h"
#include <language/duchain/identifier.h>
#include <QString>
#include <language/duchain/duchainlock.h>
#include <serialization/itemrepository.h>
#include <language/duchain/duchain.h>
#include <language/duchain/types/identifiedtype.h>
#include <language/duchain/declaration.h>

#include "duchaindebug.h"

#define ifDebug(x)

using namespace KDevelop;
namespace Php
{

ExpressionEvaluationResult::~ExpressionEvaluationResult()
{
}

ExpressionEvaluationResult::ExpressionEvaluationResult()
    : m_hadUnresolvedIdentifiers(false)/*, isInstance(false)*/
{
}

void ExpressionEvaluationResult::setDeclaration( Declaration* declaration )
{
    ENSURE_CHAIN_READ_LOCKED
    setDeclaration(DeclarationPointer(declaration));
}

void ExpressionEvaluationResult::setDeclaration( DeclarationPointer declaration)
{
    QList<DeclarationPointer> decs;
    if (declaration) {
        decs << declaration;
    }
    setDeclarations(decs);
}

void ExpressionEvaluationResult::setDeclarations( QList< Declaration* > declarations )
{
    ENSURE_CHAIN_READ_LOCKED
    QList<DeclarationPointer> decs;
    decs.reserve(declarations.size());
    Q_FOREACH(Declaration* dec, declarations) {
        decs << DeclarationPointer(dec);
    }
    setDeclarations(decs);
}

void ExpressionEvaluationResult::setDeclarations(QList<DeclarationPointer> declarations)
{
    ifDebug(qCDebug(DUCHAIN) << "setting declarations" << declarations.size();)

    m_allDeclarations = declarations;
    if (!m_allDeclarations.isEmpty()) {
        setType(m_allDeclarations.last()->abstractType());
    } else {
        setType(AbstractType::Ptr());
    }
    m_allDeclarationIds.clear();
    DUChainReadLocker lock(DUChain::lock());
    Q_FOREACH(const DeclarationPointer& dec, m_allDeclarations) {
        m_allDeclarationIds << dec->id();
        ifDebug(qCDebug(DUCHAIN) << dec->toString();)
    }
}

AbstractType::Ptr ExpressionEvaluationResult::type() const
{
    return m_type;
}

QList<DeclarationPointer> ExpressionEvaluationResult::allDeclarations() const
{
    return m_allDeclarations;
}

QList<DeclarationId> ExpressionEvaluationResult::allDeclarationIds() const
{
    return m_allDeclarationIds;
}

void ExpressionEvaluationResult::setType(AbstractType::Ptr type)
{
    ifDebug(qCDebug(DUCHAIN) << "setting type" << (type ? type->toString() : QStringLiteral("no type"));)

    m_type = type;
}

void ExpressionEvaluationResult::setHadUnresolvedIdentifiers(bool v)
{
    m_hadUnresolvedIdentifiers = v;
}

bool ExpressionEvaluationResult::hadUnresolvedIdentifiers() const
{
    return m_hadUnresolvedIdentifiers;
}


}
