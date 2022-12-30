/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umlfiledialog.h"
#include "umlviewimageexportermodel.h"

// kde includes
#include <KLocalizedString>

UMLFileDialog::UMLFileDialog(const QUrl &startDir, const QString &filter, QWidget *parent, QWidget *widget)
  : m_dialog(new QFileDialog(parent, QString(), startDir.toLocalFile(), filter))
{
    Q_UNUSED(widget);
}

UMLFileDialog::~UMLFileDialog()
{
    delete m_dialog;
}

int UMLFileDialog::exec()
{
    return m_dialog->exec();
}

void UMLFileDialog::setCaption(const QString &caption)
{
    m_dialog->setWindowTitle(caption);
}

void UMLFileDialog::setAcceptMode(QFileDialog::AcceptMode mode)
{
    m_dialog->setAcceptMode(mode);
}

void UMLFileDialog::setMimeTypeFilters(const QStringList &filters)
{
    m_dialog->setMimeTypeFilters(filters);
}

void UMLFileDialog::selectUrl(const QUrl &url)
{
    m_dialog->selectUrl(url);
}

void UMLFileDialog::setUrl(const QUrl &url)
{
    m_dialog->selectUrl(url);
}

QUrl UMLFileDialog::selectedUrl()
{
    QList<QUrl> urls = m_dialog->selectedUrls();
    if (urls.size() > 0)
        return urls.first();
    else
        return QUrl();
}

void UMLFileDialog::setSelection(const QString &name)
{
    m_dialog->selectFile(name);
}
