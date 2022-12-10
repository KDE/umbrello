/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "cmdlineexportallviewsevent.h"

// app includes
#define DBG_SRC QStringLiteral("CmdLineExportAllViewsEvent")
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umlviewimageexportermodel.h"

// kde includes
#if QT_VERSION < 0x050000
#include <kapplication.h>
#endif

// qt includes
#if QT_VERSION >= 0x050000
#include <QApplication>
#endif
#include <QCloseEvent>
#include <QStringList>

DEBUG_REGISTER(CmdLineExportAllViewsEvent)

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
CmdLineExportAllViewsEvent::CmdLineExportAllViewsEvent(const QString &imageType, const QUrl &directory, const bool useFolders)
  : QEvent(type_),
    m_imageType(imageType),
    m_directory(directory),
    m_useFolders(useFolders)
{
    logDebug1("CmdLineExportAllViewsEvent created with type value %1", type_);
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
    UMLViewList views = UMLApp::app()->document()->viewIterator();
    QStringList errors = UMLViewImageExporterModel().exportViews(views, m_imageType, m_directory, m_useFolders);
    if (!errors.isEmpty()) {
        logError0("CmdLineExportAllViewsEvent::exportAllViews(): Errors while exporting:");
        for (QStringList::Iterator it = errors.begin(); it != errors.end(); ++it) {
            logError1("- %1", *it);
        }
    }

#if QT_VERSION >= 0x050000
    qApp->quit();
#else
    kapp->quit();
#endif
}
