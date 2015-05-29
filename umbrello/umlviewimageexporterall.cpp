/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "umlviewimageexporterall.h"

// application specific includes
#include "exportallviewsdialog.h"
#include "umlviewimageexportermodel.h"
#include "uml.h"
#include "umldoc.h"

// kde include files
#include <KLocalizedString>
#if QT_VERSION < 0x050000
#include <kurl.h>
#endif
#include <kurlrequester.h>
#include <kfilefiltercombo.h>
#include <KMessageBox>

// Qt include files 
#include <QString>
#include <QStringList>
#if QT_VERSION >= 0x050000
#include <QUrl>
#endif

/**
 * Constructor for UMLViewImageExporterAll
 */
UMLViewImageExporterAll::UMLViewImageExporterAll()
{
    m_dialog = new ExportAllViewsDialog(0, "exportAllViewsDialog");
}

/**
 * Destructor for UMLViewImageExporterAll
 */
UMLViewImageExporterAll::~UMLViewImageExporterAll()
{
    delete m_dialog;
}

/**
 * Shows a dialog to the user to get the needed parameters and then exports
 * the views.
 * The dialog remembers values between calls (in the same application instance,
 * although it's not persistent between Umbrello executions).
 *
 * Once the export begins, it can't be stopped until it ends itself. The status
 * bar shows an information message until the export finishes.
 *
 * If something went wrong while exporting, an error dialog is shown to the
 * user with the error messages explaining the problems occurred.
 */
void UMLViewImageExporterAll::exportAllViews()
{
    UMLApp* app = UMLApp::app();
    UMLDoc* umlDoc = app->document();

    // default url can't be set when creating the action because the
    // document wasn't loaded
    if (m_dialog->m_kURL->url().isEmpty()) {
#if QT_VERSION >= 0x050000
        QUrl directory(umlDoc->url());
        directory.adjusted(QUrl::RemoveFilename);
        m_dialog->m_kURL->setUrl(directory);
#else
        m_dialog->m_kURL->setUrl(umlDoc->url().directory());
#endif
    }

    if (m_dialog->exec() == QDialog::Rejected) {
        return;
    }

#if QT_VERSION >= 0x050000
    app->setImageMimeType(m_dialog->m_imageType->currentText());
#else
    app->setImageMimeType(m_dialog->m_imageType->currentFilter());
#endif

    // export all views
    umlDoc->writeToStatusBar(i18n("Exporting all views..."));
    QStringList errors = UMLViewImageExporterModel().exportAllViews(
#if QT_VERSION >= 0x050000
                                UMLViewImageExporterModel::mimeTypeToImageType(m_dialog->m_imageType->currentText()),
                                QUrl(m_dialog->m_kURL->url()),
#else
                                UMLViewImageExporterModel::mimeTypeToImageType(m_dialog->m_imageType->currentFilter()),
                                KUrl(m_dialog->m_kURL->url()),
#endif
                                m_dialog->m_useFolders->isChecked());
    if (!errors.empty()) {
        KMessageBox::errorList(app, i18n("Some errors happened when exporting the images:"), errors);
    }
    umlDoc->writeToStatusBar(i18nc("reset status bar", "Ready."));
}
