/*  This file is part of the KDevelop PHP Documentation Plugin

    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "phpdocumentation.h"
#include "phpdocumentationwidget.h"
#include "phpdocsplugin.h"

PhpDocumentation::PhpDocumentation(const QUrl& url, const QString& name, const QByteArray& description, PhpDocsPlugin* parent)
    : m_url(url)
    , m_name(name)
    , m_description(description)
    , m_parent(parent)
{
}

PhpDocumentation::~PhpDocumentation()
{
}

KDevelop::IDocumentationProvider* PhpDocumentation::provider() const
{
    return m_parent;
}

QString PhpDocumentation::description() const
{
    return QString::fromUtf8( m_description );
}

QWidget* PhpDocumentation::documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent )
{
    return new PhpDocumentationWidget(findWidget, m_url, m_parent, parent);
}

QString PhpDocumentation::name() const
{
    return m_name;
}
