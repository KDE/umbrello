/* This file is part of KDevelop
   Copyright 2009 Milian Wolff <mail@milianw.de>

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

    StartAst* ast = 0;
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

