/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef DUCHAINTESTBASE_H
#define DUCHAINTESTBASE_H

#include <QObject>
#include <QByteArray>
#include <QTest>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>

#include <tests/testhelpers.h>

#include "phpduchainexport.h"

#include "completion/item.h"
namespace KDevelop
{
class TopDUContext;
}

namespace Php
{
/**
 * Manage pointer to TopDUContexts and release them properly, even if a test fails
 */
struct DUChainReleaser {
    DUChainReleaser(KDevelop::TopDUContext* top) : m_top(top) {}
    ~DUChainReleaser() {
        KDevelop::DUChainWriteLocker lock(KDevelop::DUChain::lock());
        KDevelop::DUChain::self()->removeDocumentChain(m_top);
    }
    KDevelop::TopDUContext* m_top;
};

class KDEVPHPDUCHAIN_EXPORT DUChainTestBase : public QObject
{
    Q_OBJECT

public:
    DUChainTestBase();

public:
    enum DumpArea {
        DumpNone = 0,
        DumpAST = 1,
        DumpDUChain = 2,
        DumpType = 4,
        DumpAll = 7
    };
    Q_DECLARE_FLAGS(DumpAreas, DumpArea)

public slots:
    void initTestCase();
    void cleanupTestCase();

protected:
    KDevelop::TopDUContext* parse(const QByteArray& unit,
                                  DUChainTestBase::DumpAreas dump = DumpAreas(DumpAll),
                                  QString url = QString(), KDevelop::TopDUContext* update = 0);

    KDevelop::TopDUContext* parseAdditionalFile(const KDevelop::IndexedString& fileName, const QByteArray& contents);

    KDevelop::CompletionTreeItemPointer searchDeclaration(QList<KDevelop::CompletionTreeItemPointer> items, KDevelop::Declaration* declaration);
    bool hasImportedParentContext(KDevelop::TopDUContext* top, KDevelop::DUContext* lookingFor);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DUChainTestBase::DumpAreas)
}

#endif
