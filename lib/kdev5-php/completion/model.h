/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHPCODECOMPLETIONMODEL_H
#define PHPCODECOMPLETIONMODEL_H

#include <QPair>
#include <QMap>
#include <QPointer>

#include <KTextEditor/CodeCompletionModelControllerInterface>

#include <language/codecompletion/codecompletionmodel.h>
#include <language/duchain/duchainpointer.h>

#include "context.h"

namespace Php
{

class KDEVPHPCOMPLETION_EXPORT CodeCompletionModel : public KDevelop::CodeCompletionModel
{
    Q_OBJECT

public:
    CodeCompletionModel(QObject* parent = 0);
    virtual ~CodeCompletionModel();

    bool shouldAbortCompletion(KTextEditor::View* view,
                               const KTextEditor::Range &range,
                               const QString &currentCompletion) override;

    KTextEditor::Range completionRange(KTextEditor::View* view,
                                       const KTextEditor::Cursor &position) override;

protected:
    KDevelop::CodeCompletionWorker* createCompletionWorker() override;
};

}

#endif

