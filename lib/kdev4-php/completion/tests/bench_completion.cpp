/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "bench_completion.h"

#include "helper.h"
#include "../context.h"

#include <language/codecompletion/codecompletiontesthelper.h>

#include <KMimeType>
#include <KFilterDev>

#ifdef USE_VALGRIND
  #include <valgrind/callgrind.h>
#endif

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <language/codegen/coderepresentation.h>

using namespace Php;
using namespace KDevelop;

QTEST_MAIN(Php::BenchmarkCodeCompletion)

namespace Php {

// macro defined by cmake, points to the sourcedir of _this_ file
const QString srcPath(KDESRCDIR);

QFile* getFile(const QString& path)
{
    QFile* file = new QFile(srcPath + path);
    kDebug() << file->fileName();
    Q_ASSERT(file->exists());
    file->open(QIODevice::ReadOnly);
    Q_ASSERT(!file->error());
    Q_ASSERT(file->isReadable());
    return file;
}

typedef CodeCompletionItemTester<CodeCompletionContext> PhpCompletionTester;

void BenchmarkCodeCompletion::initTestCase()
{
    AutoTestShell::init();
    TestCore* core = new TestCore();
    core->initialize(KDevelop::Core::NoUi);

    DUChain::self()->disablePersistentStorage();

    // make sure we have a valid duchain for the global file
    DUChainReadLocker lock(DUChain::lock());
    if ( !DUChain::self()->chainForDocument(internalFunctionFile()) ) {
        kDebug() << "no internal function file found in DUChain, loading it manually";
        QString fileName = internalFunctionFile().str();
        QString mimeType = KMimeType::findByPath(fileName, 0, false)->name ();
        QScopedPointer<QIODevice> file(KFilterDev::deviceForFile (fileName, mimeType, false));
        if ( !file->open(QIODevice::ReadOnly) ) {
            kDebug() << "Could not open file" << fileName;
            return;
        }
        lock.unlock();
        parseAdditionalFile(internalFunctionFile(), file->readAll());
    }
}

void BenchmarkCodeCompletion::globalCompletion()
{
    kDebug() << "benching global completion";
    TopDUContext* top = parse("<?php ", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    #ifdef USE_VALGRIND
        CALLGRIND_TOGGLE_COLLECT
    #endif

    QBENCHMARK {
        PhpCompletionTester tester(top, "<?php ");
    }

    #ifdef USE_VALGRIND
        CALLGRIND_TOGGLE_COLLECT
    #endif
}

void BenchmarkCodeCompletion::globalCompletionBigFile()
{
    QFile* file(getFile("../../create_functions.php"));
    const QString contents( file->readAll() );
    delete file;

    TopDUContext* top = parse(contents.toUtf8(), DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    #ifdef USE_VALGRIND
        CALLGRIND_TOGGLE_COLLECT
    #endif

    QBENCHMARK {
        PhpCompletionTester tester(top, contents);
    }

    #ifdef USE_VALGRIND
        CALLGRIND_TOGGLE_COLLECT
    #endif
}

void BenchmarkCodeCompletion::completionData()
{
    kDebug() << "benching global completion";
    TopDUContext* top = parse("<?php ", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    PhpCompletionTester tester(top, "<?php ");

    #ifdef USE_VALGRIND
        CALLGRIND_TOGGLE_COLLECT
    #endif

    const int size = tester.items.size();
    QBENCHMARK {
        for ( int i = 0; i < size; ++i ) {
            tester.itemData(i, KTextEditor::CodeCompletionModel::Prefix);
            tester.itemData(i, KTextEditor::CodeCompletionModel::Name);
            tester.itemData(i, KTextEditor::CodeCompletionModel::Postfix);
        }
    }
    #ifdef USE_VALGRIND
        CALLGRIND_TOGGLE_COLLECT
    #endif
}

}

#include "bench_completion.moc"
