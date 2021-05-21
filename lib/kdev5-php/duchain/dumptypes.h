/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef DUMPTYPES_H
#define DUMPTYPES_H

#include <language/duchain/types/typesystem.h>
#include "phpduchainexport.h"

namespace Php
{

class KDEVPHPDUCHAIN_EXPORT DumpTypes : protected KDevelop::TypeVisitor
{
public:
    DumpTypes();
    virtual ~DumpTypes();

    void dump(const KDevelop::AbstractType* type);

protected:
    virtual bool preVisit(const KDevelop::AbstractType * type);
    virtual void postVisit(const KDevelop::AbstractType *);

    virtual void visit(const KDevelop::IntegralType *);

    virtual bool visit(const KDevelop::AbstractType *);

    virtual bool visit(const KDevelop::PointerType * type);
    virtual void endVisit(const KDevelop::PointerType *);

    virtual bool visit(const KDevelop::ReferenceType * type);
    virtual void endVisit(const KDevelop::ReferenceType *);

    virtual bool visit(const KDevelop::FunctionType * type);
    virtual void endVisit(const KDevelop::FunctionType *);

    virtual bool visit(const KDevelop::StructureType * type);
    virtual void endVisit(const KDevelop::StructureType *);

    virtual bool visit(const KDevelop::ArrayType * type);
    virtual void endVisit(const KDevelop::ArrayType *);

private:
    bool seen(const KDevelop::AbstractType* type);

    class CppEditorIntegrator* m_editor;
    int indent;
    QSet<const KDevelop::AbstractType*> m_encountered;
};
}

#endif // DUMPTYPES_H

