/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
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
#ifndef DECLARATIONNAVIGATIONCONTEXT_H
#define DECLARATIONNAVIGATIONCONTEXT_H

#include <language/duchain/navigation/abstractdeclarationnavigationcontext.h>
#include <language/duchain/types/abstracttype.h>

namespace Php
{

class DeclarationNavigationContext : public KDevelop::AbstractDeclarationNavigationContext
{
public:
    DeclarationNavigationContext(KDevelop::DeclarationPointer decl, KDevelop::TopDUContextPointer topContext, KDevelop::AbstractNavigationContext* previousContext = 0);

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
