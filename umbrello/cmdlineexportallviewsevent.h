/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CMDLINEEXPORTALLVIEWSEVENT_H
#define CMDLINEEXPORTALLVIEWSEVENT_H

#include <qevent.h>
#include <kurl.h>

/**
 * This class provides an event that is posted to the UMLApp when the "export all views"
 * option was set in the command line. Once the QT main loop begins, the event is processed.
 *
 * The processing made in UMLApp is execute the exportAllViews method in the event.
 * This method exports all the views using UMLViewImageExporterModel and then finishes
 * the application using a close event.
 *
 * @see UMLViewImageExporterModel
 */
class CmdLineExportAllViewsEvent : public QCustomEvent {
public:

    static int getType();

    /**
     * Constructor for CmdLineExportAllViewsEvent.
     *
     * @param imageType The type of the images the views will be exported to.
     * @param directory The url of the directory where the images will be saved.
     * @param useFolders If the tree structure of the views in the document must be created
     *                   in the target directory.
     */
    CmdLineExportAllViewsEvent(const QString &imageType, const KUrl &directory, const bool useFolders);

    /**
     * Destructor for CmdLineExportAllViewsEvent
     */
    virtual ~CmdLineExportAllViewsEvent() {
    }

    void exportAllViews();

private:

    /**
     * The type of the images the views will be exported to.
     */
    QString m_imageType;

    /**
     * The url of the directory where the images will be saved.
     */
    KUrl m_directory;

    /**
     * If the tree structure of the views in the document must be created
     * in the target directory.
     */
    bool m_useFolders;
};

#endif
