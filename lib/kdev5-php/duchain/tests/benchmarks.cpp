/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#include "benchmarks.h"

#include <QtTest>
#include <QFile>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>

#include "../../parser/parsesession.h"

#include "../builders/declarationbuilder.h"
#include "../builders/usebuilder.h"

using namespace KDevelop;

QTEST_MAIN(Php::Benchmarks)

namespace Php
{

Benchmarks::Benchmarks()
{
}

QIODevice* getInternalFile()
{
    QIODevice* file = new QFile(internalFunctionFile().str());
    bool opened = file->open(QIODevice::ReadOnly);
    Q_ASSERT(opened);
    Q_UNUSED(opened);
    return file;
}

void Benchmarks::parser()
{
    QIODevice* file = getInternalFile();
    QBENCHMARK {
        ParseSession session = ParseSession();
        session.setContents(file->readAll());
        StartAst* ast = nullptr;
        session.parse(&ast);
    }
    delete file;
}

void Benchmarks::declarationBuilder()
{
    QIODevice* file = getInternalFile();
    ParseSession session;
    session.setContents(file->readAll());
    delete file;
    StartAst* ast = nullptr;
    session.parse(&ast);
    EditorIntegrator editor(&session);
    QBENCHMARK {
        DeclarationBuilder builder(&editor);
        builder.build(internalFunctionFile(), ast);
    }
}

void Benchmarks::useBuilder()
{
    const auto document = IndexedString(QUrl(QStringLiteral("file:///internal/BigTestFile.php")));

    QIODevice* file = getInternalFile();
    ParseSession session = ParseSession();
    session.setCurrentDocument(document);
    session.setContents(file->readAll());
    delete file;

    StartAst* ast = nullptr;
    session.parse(&ast);
    EditorIntegrator editor(&session);
    DeclarationBuilder builder(&editor);

    KDevelop::ReferencedTopDUContext chain = builder.build(document, ast);
    QBENCHMARK {
        UseBuilder useBuilder(&editor);
        useBuilder.buildUses(ast);
    }
}

}

