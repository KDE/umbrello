/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHPCODECOMPLETIONWORKER_H
#define PHPCODECOMPLETIONWORKER_H

#include <language/codecompletion/codecompletionworker.h>
#include <language/codecompletion/codecompletionitem.h>

#include "context.h"

namespace Php
{

class CodeCompletionModel;

class KDEVPHPCOMPLETION_EXPORT CodeCompletionWorker : public KDevelop::CodeCompletionWorker
{
    Q_OBJECT

public:
    CodeCompletionWorker(CodeCompletionModel* parent);

protected:
    virtual KDevelop::CodeCompletionContext* createCompletionContext(KDevelop::DUContextPointer context, const QString &contextText, const QString &followingText, const KDevelop::CursorInRevision& position) const;
};

}

#endif // PHPCODECOMPLETIONWORKER_H
