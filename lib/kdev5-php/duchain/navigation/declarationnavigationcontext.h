/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef DECLARATIONNAVIGATIONCONTEXT_H
#define DECLARATIONNAVIGATIONCONTEXT_H

#include <language/duchain/navigation/abstractdeclarationnavigationcontext.h>
#include <language/duchain/types/abstracttype.h>

namespace Php
{

class DeclarationNavigationContext : public KDevelop::AbstractDeclarationNavigationContext
{
public:
    DeclarationNavigationContext(KDevelop::DeclarationPointer decl, KDevelop::TopDUContextPointer topContext, KDevelop::AbstractNavigationContext *previousContext = nullptr);

protected:
    KDevelop::NavigationContextPointer registerChild(KDevelop::DeclarationPointer declaration);
    virtual KDevelop::QualifiedIdentifier prettyQualifiedIdentifier( KDevelop::DeclarationPointer decl ) const;
    virtual void htmlClass();
    virtual void htmlAdditionalNavigation();

    void makeLink( const QString& name, KDevelop::DeclarationPointer declaration, KDevelop::NavigationAction::Type actionType );

    virtual QString declarationKind(KDevelop::DeclarationPointer decl);
};

}

#endif
