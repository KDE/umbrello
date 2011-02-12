/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "cmdlineexportallviewsevent.h"

// app includes
#include "debug_utils.h"
#include "uml.h"
#include "umlviewimageexportermodel.h"

// kde includes
#include <kapplication.h>

// qt includes
#include <QtCore/QStringList>
#include <QtGui/QCloseEvent>

const QEvent::Type CmdLineExportAllViewsEvent::type_ =
    (QEvent::Type)QEvent::registerEventType(QEvent::User + 1);

/**
 * Returns the type of the event.
 * @return event type
 */
QEvent::Type CmdLineExportAllViewsEvent::eventType()
{
    return type_;
}

/**
 * Constructor.
 * @param imageType The type of the images the views will be exported to.
 * @param directory The url of the directory where the images will be saved.
 * @param useFolders If the tree structure of the views in the document must be created
 *                   in the target directory.
 */
CmdLineExportAllViewsEvent::CmdLineExportAllViewsEvent(const QString &imageType, const KUrl &directory, const bool useFolders)
  : QEvent(type_),
    m_imageType(imageType),
    m_directory(directory),
    m_useFolders(useFolders)
{
    uDebug() << "created with type value " << type_;
}

/**
 * Destructor for CmdLineExportAllViewsEvent
 */
CmdLineExportAllViewsEvent::~CmdLineExportAllViewsEvent()
{
}

/**
 * Exports all the views using UMLViewImageExporterModel, prints the errors
 * occurred in the error output and quits the application.
 * To export the views, it uses the attributes set when the event was created.
 */
void CmdLineExportAllViewsEvent::exportAllViews()
{
    QStringList errors = UMLViewImageExporterModel().exportAllViews(m_imageType, m_directory, m_useFolders);
    if (!errors.isEmpty()) {
        uError() << "CmdLineExportAllViewsEvent::exportAllViews(): Errors while exporting:";
        for (QStringList::Iterator it = errors.begin(); it != errors.end(); ++it) {
            uError() << *it;
        }
    }

    kapp->quit();
}
