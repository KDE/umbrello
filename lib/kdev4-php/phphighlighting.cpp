/*
    This file is part of KDevelop PHP
    Copyright (C) 2010 Milian Wolff

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
