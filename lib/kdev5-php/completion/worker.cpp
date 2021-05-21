/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "worker.h"

#include <KTextEditor/View>
#include <KTextEditor/Document>

#include <language/duchain/declaration.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainbase.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchain.h>

#include "model.h"
#include "completiondebug.h"

using namespace KDevelop;

namespace Php
{

CodeCompletionWorker::CodeCompletionWorker(CodeCompletionModel* parent)
        : KDevelop::CodeCompletionWorker(parent)
{
}

KDevelop::CodeCompletionContext* CodeCompletionWorker::createCompletionContext(KDevelop::DUContextPointer context, const QString &contextText, const QString &followingText, const KDevelop::CursorInRevision& position) const
{
    return new CodeCompletionContext(context, contextText, followingText, position);
}

}

