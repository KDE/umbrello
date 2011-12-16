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
#include "umlviewimageexporter.h"

// application specific includes
#include "umlviewimageexportermodel.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"

//kde include files
#include <klocale.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>

// Qt include files
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtCore/QStringList>

/**
 * Constructor for UMLViewImageExporter.
 */
UMLViewImageExporter::UMLViewImageExporter(UMLScene* scene)
{
    m_scene = scene;
    m_imageMimeType = UMLApp::app()->imageMimeType();
}

/**
 * Destructor for UMLViewImageExporter
 */
UMLViewImageExporter::~UMLViewImageExporter()
{
}

/**
 * Shows a save dialog to the user to get the needed parameters and then exports
 * the view.
 * If the selected file already exists, an overwrite confirmation
 * dialog is shown. If the user doesn't want to overwrite the file,
 * the save dialog is shown again.
 * The dialog remembers values between calls (in the same application instance,
 * although it's not persistent between Umbrello executions).
 *
 * The status bar shows an information message until the export finishes.
 *
 * If something went wrong while exporting, an error dialog is shown to the
 * user with the error message explaining the problem that happened.
 */
void UMLViewImageExporter::exportView()
{
    if (!prepareExport()) {
        return;
    }

    UMLApp* app = UMLApp::app();

    // export the view
    app->document()->writeToStatusBar(i18n("Exporting view..."));
    QString error = UMLViewImageExporterModel().exportView(m_scene,
                            UMLViewImageExporterModel::mimeTypeToImageType(m_imageMimeType), m_imageURL);
    if (!error.isNull()) {
        KMessageBox::error(app, i18n("An error happened when exporting the image:\n") + error);
    }
    app->document()->writeToStatusBar(i18nc("reset status bar", "Ready."));
}

/**
 * Shows a save file dialog to the user to get the parameters used
 * to export the scene.
 * If the selected file already exists, an overwrite confirmation
 * dialog is shown. If the user doesn't want to overwrite the file,
 * the save dialog is shown again.
 *
 * @return True if the user wants to save the image,
 *         false if the operation is cancelled.
 */
bool UMLViewImageExporter::prepareExport()
{
    bool exportPrepared = false;

    do {
        if (!getParametersFromUser()) {
            return false;
        }

        // check if the file exists
        if (KIO::NetAccess::exists(m_imageURL, KIO::NetAccess::SourceSide, UMLApp::app())) {
            int wantSave = KMessageBox::warningContinueCancel(0,
                                i18n("The selected file %1 exists.\nDo you want to overwrite it?", m_imageURL.pathOrUrl()),
                                i18n("File Already Exists"), KGuiItem(i18n("&Overwrite")));
            if (wantSave == KMessageBox::Continue) {
                exportPrepared = true;
            }
        } else {
            exportPrepared = true;
        }
    } while (!exportPrepared);

    return true;
}

/**
 * Shows a save file dialog to the user to get the parameters used
 * to export the view and updates the attributes with the parameters got.
 *
 * @return True if the user wants to save the image,
 *         false if the operation is cancelled.
 */
bool UMLViewImageExporter::getParametersFromUser()
{
    bool success = true;

    // configure & show the file dialog
    KUrl url;
    QPointer<KFileDialog> dialog = new KFileDialog(url, QString(), UMLApp::app());
    prepareFileDialog(dialog);
    dialog->exec();

    if (dialog->selectedUrl().isEmpty()) {
        success = false;
    }
    else {
        m_scene->clearSelected();   // Thanks to Peter Soetens for the idea

        // update image url and mime type
        m_imageMimeType = dialog->currentMimeFilter();
        UMLApp::app()->setImageMimeType(m_imageMimeType);
        m_imageURL = dialog->selectedUrl();

        // check if the extension is the extension of the mime type
        QFileInfo info(m_imageURL.fileName());
        QString ext = info.suffix();
        QString extDef = UMLViewImageExporterModel::mimeTypeToImageType(m_imageMimeType);
        if (ext != extDef) {
            m_imageURL.setFileName(m_imageURL.fileName() + '.' + extDef);
        }
    }
    delete dialog;
    return success;
}

/**
 * Prepares the save file dialog.
 * Sets the mime type filter, sensible default values...
 *
 * @param fileDialog The dialog to prepare.
 */
void UMLViewImageExporter::prepareFileDialog(KFileDialog *fileDialog)
{
    // get all supported mime types
    QStringList mimeTypes = UMLViewImageExporterModel::supportedMimeTypes();

    fileDialog->setCaption(i18n("Save As"));
    fileDialog->setOperationMode(KFileDialog::Saving);
    fileDialog->setMimeFilter(mimeTypes, m_imageMimeType);

    // set a sensible default filename
    if (m_imageURL.isEmpty()) {
        KUrl docURL = UMLApp::app()->document()->url();
        KUrl directory = docURL;
        directory.setPath(docURL.directory());

        fileDialog->setUrl(directory);
        fileDialog->setSelection(m_scene->name() + '.' + UMLViewImageExporterModel::mimeTypeToImageType(m_imageMimeType));
    } else {
        fileDialog->setUrl(m_imageURL);
        fileDialog->setSelection(m_imageURL.fileName());
    }
}
