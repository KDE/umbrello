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

private slots:
    /// used to inject some custom CSS to alter the remote php.net site
    void documentLoaded();
    void linkClicked(const QUrl& url);

private:
    KDevelop::StandardDocumentationView* m_part;
    QWidget* m_loading;
    QTemporaryFile* m_styleSheet;
    PhpDocsPlugin* m_provider;
};

#endif // PHPDOCUMENTATIONWIDGET_H
