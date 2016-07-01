/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLFILEDIALOG_H
#define UMLFILEDIALOG_H

#include <QtGlobal>
#include <QFileDialog>

/**
 * Wrapper class for KFileDialog supporting graphviz *.dot files.
 *
 * This class is required because KFileDialog do not support
 * mixing of mime type based filters with other filters.
 *
 * Because the graphviz .dot extension do not have an official
 * mimetype, we use a fake mime type named image/x-dot
 */
class UMLFileDialog : public QObject
{
    Q_OBJECT
public:
    UMLFileDialog(const QUrl &startDir, const QString &filter, QWidget *parent, QWidget *widget=0);
    ~UMLFileDialog();

    int exec();
    void setCaption(const QString &caption);
    void setAcceptMode(QFileDialog::AcceptMode mode);
    void setMimeTypeFilters(const QStringList &filters);

    void selectUrl(const QUrl &url);
    void setUrl(const QUrl &url);
    QUrl selectedUrl();
    void setSelection(const QString &name);

protected:
    QFileDialog *m_dialog;
};

#endif
