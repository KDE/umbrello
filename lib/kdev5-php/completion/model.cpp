/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "model.h"

#include <QIcon>
#include <QMetaType>
#include <QTextFormat>
#include <QBrush>
#include <QDir>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>

#include <language/duchain/declaration.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>
#include <editorintegrator.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainbase.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchainutils.h>

#include "../duchain/declarations/classmethoddeclaration.h"

#include "context.h"
#include "worker.h"
#include "completiondebug.h"

using namespace KTextEditor;
using namespace KDevelop;

namespace Php
{

CodeCompletionModel::CodeCompletionModel(QObject * parent)
        : KDevelop::CodeCompletionModel(parent)
{
}

KDevelop::CodeCompletionWorker* CodeCompletionModel::createCompletionWorker()
{
    return new CodeCompletionWorker(this);
}

CodeCompletionModel::~CodeCompletionModel()
{
}

Range CodeCompletionModel::completionRange(View* view, const Cursor &position)
{
    Range range = KDevelop::CodeCompletionModel::completionRange(view, position);
    if (range.start().column() > 0) {
        KTextEditor::Range preRange(Cursor(range.start().line(), range.start().column() - 1),
                                    Cursor(range.start().line(), range.start().column()));
        qCDebug(COMPLETION) << preRange << view->document()->text(preRange);
        const QString contents = view->document()->text(preRange);
        if ( contents == QLatin1String("$") ) {
            range.expandToRange(preRange);
            qCDebug(COMPLETION) << "using custom completion range" << range;
        }
    }
    return range;
}

bool CodeCompletionModel::shouldAbortCompletion(View* view, const Range &range, const QString &currentCompletion)
{
    if(view->cursorPosition() < range.start() || view->cursorPosition() > range.end())
        return true; //Always abort when the completion-range has been left
    //Do not abort completions when the text has been empty already before and a newline has been entered

    static const QRegularExpression allowedText("^\\$?(\\w*)");
    return !allowedText.exactMatch(currentCompletion);
}

}

