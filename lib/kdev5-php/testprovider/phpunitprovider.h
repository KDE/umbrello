/*
    This file is part of KDevelop PHP support
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHPUNITPROVIDER_H
#define PHPUNITPROVIDER_H

#include <interfaces/iplugin.h>

#include <language/duchain/indexeddeclaration.h>
#include <language/duchain/topducontext.h>

#include <QVariant>

class QVariant;

class PhpUnitProvider : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    PhpUnitProvider(QObject* parent, const QList<QVariant>& args = QList<QVariant>());

public slots:
    void updateReady(const KDevelop::IndexedString& document, const KDevelop::ReferencedTopDUContext& context);

private:
    KDevelop::IndexedString m_phpUnitDeclarationsFile;
    KDevelop::IndexedDeclaration m_testCaseDeclaration;
    QList<KDevelop::ReferencedTopDUContext> m_pendingContexts;

    void processContext(KDevelop::ReferencedTopDUContext context);
    void processTestCaseDeclaration(KDevelop::Declaration* declaration);
};

#endif // PHPUNITPROVIDER_H
