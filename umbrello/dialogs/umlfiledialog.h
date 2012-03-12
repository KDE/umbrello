/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLFILEDIALOG_H
#define UMLFILEDIALOG_H

#include <kfiledialog.h>

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
    UMLFileDialog(const KUrl &startDir, const QString &filter, QWidget *parent, QWidget *widget=0);
    ~UMLFileDialog();

    int exec();
    void setCaption(const QString &caption);
    void setOperationMode(KFileDialog::OperationMode mode);
    void setUrl(const KUrl &url, bool clearforward=true);
    KUrl selectedUrl();
    void setSelection(const QString &name);
    void setMimeFilter(const QStringList &types, const QString &defaultType=QString());
    QString currentMimeFilter();

protected:
    KFileDialog *m_dialog;
};

#endif
