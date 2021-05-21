/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "declarationnavigationcontext.h"

#include <QtGui/QTextDocument>

#include <klocale.h>
#include <klocalizedstring.h>

#include <language/duchain/functiondefinition.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/forwarddeclaration.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/types/structuretype.h>

#include "../declarations/classdeclaration.h"
#include <declarations/classmethoddeclaration.h>
#include <declarations/traitmethodaliasdeclaration.h>
#include <declarations/traitmemberaliasdeclaration.h>
#include "helper.h"

namespace Php
{
using namespace KDevelop;

DeclarationNavigationContext::DeclarationNavigationContext(DeclarationPointer decl, KDevelop::TopDUContextPointer topContext, AbstractNavigationContext* previousContext)
        : AbstractDeclarationNavigationContext(decl, topContext, previousContext)
{
}

NavigationContextPointer DeclarationNavigationContext::registerChild(DeclarationPointer declaration)
{
    return AbstractDeclarationNavigationContext::registerChild(new DeclarationNavigationContext(declaration, m_topContext, this));
}

void DeclarationNavigationContext::htmlClass()
{
    StructureType::Ptr klass = m_declaration->abstractType().cast<StructureType>();
    Q_ASSERT(klass);
    ClassDeclaration* classDecl = dynamic_cast<ClassDeclaration*>(klass->declaration(m_topContext.data()));
    if (classDecl) {
        // write class modifier
        switch (classDecl->classModifier()) {
        case ClassDeclarationData::Abstract:
            modifyHtml() += "abstract ";
            break;
        case ClassDeclarationData::Final:
            modifyHtml() += "final ";
            break;
        default:
            //nothing
            break;
        }
        // write class type
        if (classDecl->classType() == ClassDeclarationData::Interface) {
            modifyHtml() += "interface ";
        } else if (classDecl->classType() == ClassDeclarationData::Trait) {
            modifyHtml() += "trait ";
        } else {
            modifyHtml() += "class ";
        }
        // write class identifier
        eventuallyMakeTypeLinks(m_declaration->abstractType());
        // write inheritance
        if (classDecl->baseClassesSize() > 0) {
            AbstractType::Ptr extends;
            QList<AbstractType::Ptr> implements;
            FOREACH_FUNCTION(const BaseClassInstance& base, classDecl->baseClasses) {
                StructureType::Ptr stype = base.baseClass.type<StructureType>();
                if (stype) {
                    ClassDeclaration *classDecl = dynamic_cast<ClassDeclaration*>(stype->declaration(m_topContext.data()));
                    if (classDecl) {
                        if (classDecl->classType() == ClassDeclarationData::Interface) {
                            implements.append(base.baseClass.abstractType());
                        } else {
                            extends = base.baseClass.abstractType();
                        }
                    }
                }
            }
            // write parent class
            if (extends) {
                modifyHtml() += " extends ";
                eventuallyMakeTypeLinks(extends);
            }
            // write implemented interfaces
            if (!implements.isEmpty()) {
                modifyHtml() += " implements ";
                for (QList<AbstractType::Ptr>::iterator i = implements.begin(); ;) {
                    eventuallyMakeTypeLinks(*i);
                    ++i;
                    if (i != implements.end()) {
                        modifyHtml() += ", ";
                    } else {
                        break;
                    }
                }
            }
        }
        modifyHtml() += " ";
    }
}

void DeclarationNavigationContext::htmlAdditionalNavigation()
{
    if (auto member = dynamic_cast<TraitMethodAliasDeclaration*>(m_declaration.data())) {
        Declaration *dec = member->aliasedDeclaration().data();
        if (dec && dec->context() && dec->context()->owner()) {
            modifyHtml() += i18n("Use of %1 from %2<br />")
                            .arg(createLink(prettyQualifiedIdentifier(DeclarationPointer(dec)).toString(),
                                            QString("jump_to_used"),
                                            NavigationAction(DeclarationPointer(dec),
                                                             KDevelop::NavigationAction::NavigateDeclaration)))
                            .arg(createLink(prettyQualifiedIdentifier(DeclarationPointer(dec->context()->owner())).toString(),
                                            QString("jump_to_used_container"),
                                            NavigationAction(DeclarationPointer(dec->context()->owner()),
                                                             KDevelop::NavigationAction::NavigateDeclaration)));
        }
    } else if (auto member = dynamic_cast<TraitMemberAliasDeclaration*>(m_declaration.data())) {
        Declaration *dec = member->aliasedDeclaration().data();
        if (dec && dec->context() && dec->context()->owner()) {
            modifyHtml() += i18n("Use of %1 from %2<br />")
                            .arg(createLink(prettyQualifiedIdentifier(DeclarationPointer(dec)).toString(),
                                            QString("jump_to_used"),
                                            NavigationAction(DeclarationPointer(dec),
                                                             KDevelop::NavigationAction::NavigateDeclaration)))
                            .arg(createLink(prettyQualifiedIdentifier(DeclarationPointer(dec->context()->owner())).toString(),
                                            QString("jump_to_used_container"),
                                            NavigationAction(DeclarationPointer(dec->context()->owner()),
                                                             KDevelop::NavigationAction::NavigateDeclaration)));
        } else {
            modifyHtml() += i18n("Broken member alias trait.");
        }
    }

    KDevelop::AbstractDeclarationNavigationContext::htmlAdditionalNavigation();
}

QualifiedIdentifier DeclarationNavigationContext::prettyQualifiedIdentifier( DeclarationPointer decl ) const
{
    return QualifiedIdentifier(prettyName(decl.data()));
}

void DeclarationNavigationContext::makeLink(const QString& name, DeclarationPointer declaration, NavigationAction::Type actionType)
{
    if ( actionType == NavigationAction::JumpToSource && declaration->url() == internalFunctionFile() ) {
        modifyHtml() += i18n("PHP internal");
        return;
    }
    AbstractDeclarationNavigationContext::makeLink(name, declaration, actionType);
}

QString DeclarationNavigationContext::declarationKind(DeclarationPointer decl)
{
    if ( decl->kind() == Declaration::Instance && decl->abstractType()
         && decl->abstractType()->modifiers() & AbstractType::ConstModifier ) {
        return i18nc("kind of a php-constant, as shown in the declaration tooltip", "Constant");
    }
    return AbstractDeclarationNavigationContext::declarationKind(decl);
}

}
