/*
    SPDX-FileCopyrightText: 2007 Piyush verma <piyush.verma@gmail.com>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PHP_PARSEJOB_H
#define PHP_PARSEJOB_H

#include <language/backgroundparser/parsejob.h>
#include <language/duchain/problem.h>

#include <QStringList>

#include <ksharedptr.h>
#include <ktexteditor/range.h>

#include <language/duchain/topducontext.h>

namespace Php
{

class AstNode;
class LanguageSupport;
class EditorIntegrator;

class ParseJob : public KDevelop::ParseJob
{
    Q_OBJECT

public:
    enum {
        Resheduled = KDevelop::TopDUContext::LastFeature
    };

    explicit ParseJob(const KDevelop::IndexedString& url, KDevelop::ILanguageSupport* LanguageSupport);
    virtual ~ParseJob();

    void setParentJob(ParseJob *job);

protected:
    LanguageSupport* php() const;
    virtual void run();

private:
    ParseJob *m_parentJob; ///< parent job if this one is an include

    /**
     * Checks if a parent job parses already \p document. Used to prevent
     * endless recursions in include statements
     */
    bool hasParentDocument(const KDevelop::IndexedString &document);

    /// create a problem pointer for the current document
    KDevelop::ProblemPointer createProblem(const QString &description, AstNode* node,
                                           EditorIntegrator * editor, KDevelop::ProblemData::Source source,
                                           KDevelop::ProblemData::Severity severity = KDevelop::ProblemData::Error);
};

}

#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on; auto-insert-doxygen on
