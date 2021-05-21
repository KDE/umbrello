/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "dumptypes.h"

#include <language/duchain/types/alltypes.h>

using namespace KDevelop;
namespace Php
{

DumpTypes::DumpTypes()
        : indent(0)
{
}

DumpTypes::~ DumpTypes()
{
}

void DumpTypes::dump(const AbstractType * type)
{
    if (type) type->accept(this);
    m_encountered.clear();
}

bool DumpTypes::preVisit(const AbstractType * type)
{
    ++indent;
    kDebug() << QString(indent*2, ' ') << type->toString();
    return true;
}

void DumpTypes::postVisit(const AbstractType *)
{
    --indent;
}

void DumpTypes::visit(const IntegralType *)
{
}

bool DumpTypes::visit(const KDevelop::AbstractType *type)
{
    return !seen(type);
}

bool DumpTypes::visit(const PointerType * type)
{
    return !seen(type);
}

void DumpTypes::endVisit(const PointerType *)
{
}

bool DumpTypes::visit(const ReferenceType * type)
{
    return !seen(type);
}

void DumpTypes::endVisit(const ReferenceType *)
{
}

bool DumpTypes::visit(const FunctionType * type)
{
    return !seen(type);
}

void DumpTypes::endVisit(const FunctionType *)
{
}

bool DumpTypes::visit(const StructureType * type)
{
    return !seen(type);
}

void DumpTypes::endVisit(const StructureType *)
{
}

bool DumpTypes::visit(const ArrayType * type)
{
    return !seen(type);
}

void DumpTypes::endVisit(const ArrayType *)
{
}


bool DumpTypes::seen(const AbstractType * type)
{
    if (m_encountered.contains(type))
        return true;

    m_encountered.insert(type);
    return false;
}

}
