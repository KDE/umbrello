/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "cmdlineexportallviewsevent.h"

// qt includes
#include <QtCore/QStringList>

// kde includes
#include <kapplication.h>
#include <kdebug.h>

// app includes
#include "uml.h"
#include "umlviewimageexportermodel.h"


/**
 * Returns the type of the event.
 */
int CmdLineExportAllViewsEvent::getType()
{
    return QEvent::User + 1;
}

CmdLineExportAllViewsEvent::CmdLineExportAllViewsEvent(const QString &imageType, const KUrl &directory, const bool useFolders)
  : QCustomEvent(CmdLineExportAllViewsEvent::getType())
{
    m_imageType = imageType;
    m_directory = directory;
    m_useFolders = useFolders;
}

/**
 * Exports all the views using UMLViewImageExporterModel, prints the errors
 * occurred in the error output and sends a close event to the application to finish it.
 * To export the views, it uses the attributes set when the event was created.
 */
void CmdLineExportAllViewsEvent::exportAllViews()
{
    QStringList errors = UMLViewImageExporterModel().exportAllViews(m_imageType, m_directory, m_useFolders);
    if (!errors.isEmpty()) {
        uError() << "Errors while exporting:";
        for (QStringList::Iterator it = errors.begin(); it != errors.end(); ++it) {
            uError() << *it << endl;
        }
    }

    kapp->sendEvent(UMLApp::app(), new QCloseEvent());
}
