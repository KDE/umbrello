/*  This file is part of the KDevelop PHP Documentation Plugin

    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "phpdocumentationwidget.h"

#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QTemporaryFile>
#include <QTextStream>

#include <KLocalizedString>

#include "phpdocsplugin.h"
#include <documentation/standarddocumentationview.h>

QTemporaryFile* createStyleSheet(QObject* parent)
{
    QTemporaryFile* file = new QTemporaryFile(parent);
    bool ret = file->open();
    Q_ASSERT(ret);
    Q_UNUSED(ret);

    QTextStream ts(file);
    ts << ".page-tools { float: none !important; } body { background: white !important; };";
    return file;
}

PhpDocumentationWidget::PhpDocumentationWidget(KDevelop::DocumentationFindWidget* find, const QUrl &url,
                                               PhpDocsPlugin* provider, QWidget* parent)
: QStackedWidget(parent)
, m_loading(new QWidget(this))
, m_styleSheet(createStyleSheet(this))
, m_provider(provider)
{
    m_part = new KDevelop::StandardDocumentationView(find, this);
    m_part->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    addWidget(m_part);
    addWidget(m_loading);

    QProgressBar* progressbar = new QProgressBar;
    progressbar->setValue(0);
    progressbar->setMinimum(0);
    progressbar->setMaximum(100);
    progressbar->setAlignment(Qt::AlignCenter);

    connect( m_part, &KDevelop::StandardDocumentationView::loadProgress,
             progressbar, &QProgressBar::setValue );

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addStretch();
    QLabel* label = new QLabel(i18n("...loading documentation..."));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    layout->addWidget(progressbar);
    layout->addStretch();
    m_loading->setLayout(layout);
    setCurrentWidget(m_loading);


    connect(m_part, &KDevelop::StandardDocumentationView::linkClicked, this, &PhpDocumentationWidget::linkClicked);
    connect(m_part, &KDevelop::StandardDocumentationView::loadFinished, this, &PhpDocumentationWidget::documentLoaded);

    m_part->load( url );
}

PhpDocumentationWidget::~PhpDocumentationWidget()
{
    // make sure we don't get called by any of the m_part signals on shutdown, see also:
    // https://codereview.qt-project.org/#/c/83800/
    disconnect(m_part, 0, this, 0);
}

void PhpDocumentationWidget::linkClicked(const QUrl& url)
{
    m_part->load(url);
    m_provider->addToHistory(url);
}

void PhpDocumentationWidget::documentLoaded()
{
    m_part->settings()->setUserStyleSheetUrl(QUrl::fromLocalFile(m_styleSheet->fileName()));

    setCurrentWidget(m_part);
    removeWidget(m_loading);
    delete m_loading;
    m_loading = 0;
}
