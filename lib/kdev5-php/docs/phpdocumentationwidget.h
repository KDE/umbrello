/*  This file is part of the KDevelop PHP Documentation Plugin

    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PHPDOCUMENTATIONWIDGET_H
#define PHPDOCUMENTATIONWIDGET_H

#include <QStackedWidget>

class QTemporaryFile;
class QUrl;

namespace KDevelop {
    class StandardDocumentationView;
    class DocumentationFindWidget;
}

class PhpDocsPlugin;

class PhpDocumentationWidget : public QStackedWidget
{
    Q_OBJECT

public:
    explicit PhpDocumentationWidget(KDevelop::DocumentationFindWidget* find, const QUrl &url,
                                    PhpDocsPlugin* provider, QWidget* parent = nullptr);
    ~PhpDocumentationWidget();

    /// used to inject some custom CSS to alter the remote php.net site
    Q_SLOT void documentLoaded();
    Q_SLOT void linkClicked(const QUrl& url);

private:
    KDevelop::StandardDocumentationView* m_part = nullptr;
    QWidget* m_loading = nullptr;
    QTemporaryFile* m_styleSheet;
    PhpDocsPlugin* m_provider;
};

#endif // PHPDOCUMENTATIONWIDGET_H
