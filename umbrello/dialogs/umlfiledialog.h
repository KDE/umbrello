/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLFILEDIALOG_H
#define UMLFILEDIALOG_H

#include <QtGlobal>

#if QT_VERSION >= 0x050000
#include <QFileDialog>
#else
#include <kfiledialog.h>
#endif

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
#if QT_VERSION >= 0x050000
    UMLFileDialog(const QUrl &startDir, const QString &filter, QWidget *parent, QWidget *widget=0);
#else
    UMLFileDialog(const KUrl &startDir, const QString &filter, QWidget *parent, QWidget *widget=0);
#endif
    ~UMLFileDialog();

    int exec();
    void setCaption(const QString &caption);
#if QT_VERSION >= 0x050000
    void setAcceptMode(QFileDialog::AcceptMode mode);
    void setMimeTypeFilters(const QStringList &filters);

    void selectUrl(const QUrl &url);
    void setUrl(const QUrl &url);
    QUrl selectedUrl();
#else
    void setOperationMode(KFileDialog::OperationMode mode);
    void setUrl(const KUrl &url, bool clearforward=true);
    KUrl selectedUrl();
#endif
    void setSelection(const QString &name);

#if QT_VERSION < 0x050000
    void setMimeFilter(const QStringList &types, const QString &defaultType=QString());
    QString currentMimeFilter();
#endif

protected:
#if QT_VERSION >= 0x050000
    QFileDialog *m_dialog;
#else
    KFileDialog *m_dialog;
#endif
};

#endif
