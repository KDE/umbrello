/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDLINEEXPORTALLVIEWSEVENT_H
#define CMDLINEEXPORTALLVIEWSEVENT_H

#include <QUrl>
#include <QEvent>

/**
 * This class provides an event that is posted to the UMLApp when the "export all views"
 * option was set in the command line. Once the Qt main loop begins, the event is processed.
 *
 * The processing made in UMLApp is execute the exportAllViews method in the event.
 * This method exports all the views using UMLViewImageExporterModel and then finishes
 * the application using a close event.
 *
 * @see UMLViewImageExporterModel
 */
class CmdLineExportAllViewsEvent : public QEvent 
{
public:

    static QEvent::Type eventType();

    CmdLineExportAllViewsEvent(const QString &imageType, const QUrl &directory, const bool useFolders);

    virtual ~CmdLineExportAllViewsEvent();

    void exportAllViews();

private:

    static const QEvent::Type type_;

    QString m_imageType;   ///< The type of the images the views will be exported to.
    QUrl    m_directory;   ///< The url of the directory where the images will be saved.
    bool    m_useFolders;  ///< If tree structure of the views in the document must be created in the target directory.

};

#endif
