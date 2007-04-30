/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "cmdlineexportallviewsevent.h"

// qt includes
#include <qstringlist.h>

// kde includes
#include <kapplication.h>
#include <kdebug.h>

// app includes
#include "uml.h"
#include "umlviewimageexportermodel.h"


int CmdLineExportAllViewsEvent::getType() {
    return QEvent::User + 1;
}

CmdLineExportAllViewsEvent::CmdLineExportAllViewsEvent(const QString &imageType, const KURL &directory, const bool useFolders)
  : QCustomEvent(CmdLineExportAllViewsEvent::getType()) {
    m_imageType = imageType;
    m_directory = directory;
    m_useFolders = useFolders;
}

void CmdLineExportAllViewsEvent::exportAllViews() {
    QStringList errors = UMLViewImageExporterModel().exportAllViews(m_imageType, m_directory, m_useFolders);
    if (!errors.isEmpty()) {
        kError() << "Errors while exporting:" << endl;
        for (QStringList::Iterator it = errors.begin(); it != errors.end(); ++it) {
            kError() << *it << endl;
        }
    }

    kapp->sendEvent(UMLApp::app(), new QCloseEvent());
}
