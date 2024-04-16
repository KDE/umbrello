/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "duchaintestbase.h"

#include <QtTest>

#include <language/duchain/parsingenvironment.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchaindumper.h>
#include <language/codegen/coderepresentation.h>

#include <serialization/indexedstring.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include "../completion/codemodelitem.h"

#include "../builders/declarationbuilder.h"
#include "../builders/usebuilder.h"

#include "../dumptypes.h"
#include "../../parser/parsesession.h"
#include "../../parser/phpdebugvisitor.h"

#include "../helper.h"

using namespace KDevelop;

namespace Php
{

void DUChainTestBase::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    DUChain::self()->disablePersistentStorage();
    CodeRepresentation::setDiskChangesForbidden(true);

    //yeah... adding a testcase here is kinda strange, but anyways - we have to check for special
    //handling of the internal functions file
    //see e.g. testDeclarationReturnTypeDocBlock
    QByteArray content("<?php "
                        "class Exception {} "
                        //test start
                        "/** @return Exception **/ function should_return_exception() {}\n"
                       "class internal_test_class {/** @return Exception **/ function should_return_exception() {}}\n"
                        // test end
                       "function define() {} function substr() {} class stdClass {}\n"
                       "/**\n * @superglobal\n **/\n$_GET = array();\n"
                       "interface testInterface {}\n");
    content.append("interface Iterator { function rewind(); function current(); function key(); function next(); function valid(); } ");
    QVERIFY(!internalFunctionFile().isEmpty());
    TopDUContext* ctx = parseAdditionalFile(internalFunctionFile(), content);
    QVERIFY(ctx);

    DUChainWriteLocker lock;
    QVERIFY(ctx->problems().isEmpty());

    // set features and modification revision, to prevent test cases that use
    // the full language plugin from re-parsing the big internal function file
    ctx->setFeatures(TopDUContext::AllDeclarationsAndContexts);
    ParsingEnvironmentFilePointer file = ctx->parsingEnvironmentFile();
    QVERIFY(file);
    file->setModificationRevision(ModificationRevision::revisionForFile(internalFunctionFile()));
    DUChain::self()->updateContextEnvironment(ctx, file.data());
}

void DUChainTestBase::cleanupTestCase()
{
    TestCore::shutdown();
}

CompletionTreeItemPointer DUChainTestBase::searchDeclaration(QList<CompletionTreeItemPointer> items, Declaration* declaration)
{
    Q_FOREACH(const CompletionTreeItemPointer &item, items) {
        if (item->declaration().data() == declaration) {
            return item;
        }
    }
    return CompletionTreeItemPointer();
}

bool DUChainTestBase::hasImportedParentContext(TopDUContext* top, DUContext* lookingFor)
{
    qDebug() << "this topcontext has " << top->importedParentContexts().count() << " imported parent contexts"
    << "\n we are looking for: " << lookingFor->url().byteArray();
    Q_FOREACH(const DUContext::Import &import, top->importedParentContexts()) {
        if (import.context(top)) {
            qDebug() << import.context(top)->url().byteArray();
        }
        if (import.context(top) == lookingFor) {
            return true;
        }
    }
    return false;
}

TopDUContext* DUChainTestBase::parseAdditionalFile(const IndexedString& fileName, const QByteArray& contents)
{
    ParseSession session;
    session.setContents(contents);
    StartAst* ast = 0;
    if (!session.parse(&ast)) qFatal("can't parse");

    EditorIntegrator editor(&session);
    session.setCurrentDocument(fileName);
    DeclarationBuilder declarationBuilder(&editor);
    TopDUContext* top = declarationBuilder.build(fileName, ast);

    if ( fileName != internalFunctionFile() ) {
        UseBuilder useBuilder(&editor);
        useBuilder.buildUses(ast);
    }

    if (!session.problems().isEmpty()) {
        DUChainWriteLocker lock;
        Q_FOREACH( const ProblemPointer& p, session.problems() ) {
            top->addProblem(p);
        }
    }

    return top;
}

TopDUContext* DUChainTestBase::parse(const QByteArray& unit, DumpAreas dump,
                                     QUrl url, TopDUContext* update)
{
    if (dump)
        qDebug() << "==== Beginning new test case...:" << endl << unit;

    ParseSession session;
    session.setContents(unit);
    StartAst* ast = 0;
    if (!session.parse(&ast)) {
        qDebug() << "Parse failed";
        return 0;
    }

    if (dump & DumpAST) {
        qDebug() << "===== AST:";
        DebugVisitor debugVisitor(session.tokenStream(), session.contents());
        debugVisitor.visitNode(ast);
    }

    static int testNumber = 0;
    if (url.isEmpty()) {
        url = QUrl(QStringLiteral("file:///internal/%1").arg(testNumber++));
    }

    EditorIntegrator editor(&session);
    session.setCurrentDocument(IndexedString(url));
    DeclarationBuilder declarationBuilder(&editor);
    TopDUContext* top = declarationBuilder.build(session.currentDocument(), ast, ReferencedTopDUContext(update));

    if (!session.problems().isEmpty()) {
        DUChainWriteLocker lock;
        Q_FOREACH( const ProblemPointer& p, session.problems() ) {
            top->addProblem(p);
        }
    }

    if ( IndexedString(url) != internalFunctionFile() ) {
        UseBuilder useBuilder(&editor);
        useBuilder.buildUses(ast);
    }

    if (dump & DumpDUChain) {
        qDebug() << "===== DUChain:";

        DUChainWriteLocker lock(DUChain::lock());
        DUChainDumper d;
        d.dump(top);
    }

    if (dump & DumpType) {
        qDebug() << "===== Types:";
        DUChainWriteLocker lock(DUChain::lock());
        DumpTypes dt;
        Q_FOREACH(const AbstractType::Ptr& type, declarationBuilder.topTypes()) {
            dt.dump(type.data());
        }
    }



    if (dump)
        qDebug() << "===== Finished test case.";

    return top;
}
}


