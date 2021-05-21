/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#include "benchmarks.h"

#include <QtTest/QtTest>
#include <QtCore/QFile>

#include <KFilterDev>
#include <KMimeType>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>

#include "phpparsejob.h"
#include "parsesession.h"
#include "../builders/declarationbuilder.h"
#include "../builders/usebuilder.h"

using namespace KDevelop;

QTEST_MAIN(Php::Benchmarks)

namespace Php
{

// macro defined by cmake, points to the sourcedir of _this_ file
const QString srcPath(KDESRCDIR);

Benchmarks::Benchmarks()
{
}

QIODevice* getInternalFile()
{
    QString fileName = srcPath + "../../phpfunctions.php";
    QString mimeType = KMimeType::findByPath(fileName, 0, false)->name ();
    QIODevice* file = KFilterDev::deviceForFile (fileName, mimeType, false);
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
        StartAst* ast = 0;
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
    StartAst* ast = 0;
    session.parse(&ast);
    EditorIntegrator editor(&session);
    QBENCHMARK {
        DeclarationBuilder builder(&editor);
        ReferencedTopDUContext top = builder.build(internalFunctionFile(), ast);

        if ( true ) {
            DUChainWriteLocker lock(DUChain::lock());
            kDebug() << top->localDeclarations().size();
        }
    }
}

void Benchmarks::useBuilder()
{
    QIODevice* file = getInternalFile();
    ParseSession session = ParseSession();
    session.setContents(file->readAll());
    delete file;
    StartAst* ast = 0;
    session.parse(&ast);
    EditorIntegrator editor(&session);
    DeclarationBuilder builder(&editor);
    KDevelop::ReferencedTopDUContext chain = builder.build(IndexedString("BigTestFile.php"), ast);
    QBENCHMARK {
        UseBuilder useBuilder(&editor);
        useBuilder.buildUses(ast);
    }
}

}

#include "benchmarks.moc"
