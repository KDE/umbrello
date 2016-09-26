/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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
