/*  This file is part of the KDevelop PHP Documentation Plugin

    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PHPDOCUMENTATION_H
#define PHPDOCUMENTATION_H

#include <interfaces/idocumentation.h>

#include <QObject>
#include <QString>
#include <QUrl>

class QWidget;
class QStackedWidget;

class PhpDocsPlugin;

class PhpDocumentation : public KDevelop::IDocumentation
{
    Q_OBJECT
public:
    explicit PhpDocumentation(const QUrl& url, const QString& name, const QByteArray& description, PhpDocsPlugin* parent);
    ~PhpDocumentation();

    QString name() const override;
    QString description() const override;
    QWidget* documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent = 0) override;
    KDevelop::IDocumentationProvider* provider() const override;

private:
    const QUrl m_url;
    const QString m_name;
    const QByteArray m_description;
    PhpDocsPlugin* m_parent;
};

#endif // PHPDOCUMENTATION_H
