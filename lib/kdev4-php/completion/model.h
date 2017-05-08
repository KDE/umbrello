/*
 * KDevelop Php Code Completion Support
 *
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
 * Copyright 2008 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef PHPCODECOMPLETIONMODEL_H
#define PHPCODECOMPLETIONMODEL_H

#include <QPair>
#include <QMap>
#include <QPointer>
#include <ksharedptr.h>
#include <ktexteditor/codecompletionmodelcontrollerinterface.h>

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

    bool shouldAbortCompletion(KTextEditor::View* view, const KTextEditor::Range &range, const QString &currentCompletion);
    KTextEditor::Range completionRange(KTextEditor::View* view, const KTextEditor::Cursor &position);

protected:
    virtual KDevelop::CodeCompletionWorker* createCompletionWorker();
};

}

#endif

