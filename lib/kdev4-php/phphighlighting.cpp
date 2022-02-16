/*
    This file is part of KDevelop PHP
    SPDX-FileCopyrightText: 2010 Milian Wolff

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "phphighlighting.h"
#include <language/duchain/declaration.h>

#include "duchain/declarations/variabledeclaration.h"

using namespace KDevelop;
using namespace Php;

//BEGIN HighlightingInstance

class HighlightingInstance : public CodeHighlightingInstance {
public:
    HighlightingInstance(const CodeHighlighting* highlighting);
    virtual Types typeForDeclaration(KDevelop::Declaration* decl, KDevelop::DUContext* context) const;
    virtual bool useRainbowColor( Declaration* dec ) const;
};

HighlightingInstance::HighlightingInstance(const CodeHighlighting* highlighting)
    : CodeHighlightingInstance(highlighting)
{
}

HighlightingEnumContainer::Types HighlightingInstance::typeForDeclaration(Declaration* decl, DUContext* context) const
{
    if (decl && !decl->isFunctionDeclaration() && decl->abstractType()
        && decl->abstractType()->modifiers() & AbstractType::ConstModifier
        && !dynamic_cast<VariableDeclaration*>(decl) )
    {
        return EnumType;
    } else {
        return CodeHighlightingInstance::typeForDeclaration(decl, context);
    }
}

bool HighlightingInstance::useRainbowColor( Declaration* dec ) const
{
    return dynamic_cast<VariableDeclaration*>(dec);
}

//BEGIN Highlighting

Highlighting::Highlighting(QObject* parent)
    : CodeHighlighting(parent)
{

}

CodeHighlightingInstance* Highlighting::createInstance() const
{
    return new HighlightingInstance(this);
}
