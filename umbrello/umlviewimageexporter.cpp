/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umlviewimageexporter.h"

// application specific includes
#define DBG_SRC QStringLiteral("UMLViewImageExporter")
#include "debug_utils.h"
#include "dotgenerator.h"
#include "umlfiledialog.h"
#include "umlviewimageexportermodel.h"
#include "umlapp.h"
#include "umldoc.h"
#include "umlscene.h"

//kde include files
#include <KIO/Job>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>

// Qt include files
#include <QFileDialog>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <kio/statjob.h>

DEBUG_REGISTER_DISABLED(UMLViewImageExporter)

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
    UMLViewImageExporterModel theExporter;
    QString error = theExporter.exportView(m_scene,
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
        KIO::StatJob *job = KIO::stat(m_imageURL, KIO::StatJob::SourceSide, KIO::StatDetails(0));
        KJobWidgets::setWindow(job, UMLApp::app());
        job->exec();
        bool result = !job->error();
        if (result) {
            int wantSave = KMessageBox::warningContinueCancel(nullptr,
                                i18n("The selected file %1 exists.\nDo you want to overwrite it?", m_imageURL.url(QUrl::PreferLocalFile)),
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
    QUrl url;
    QPointer<UMLFileDialog> dialog = new UMLFileDialog(url, QString(), UMLApp::app());
    prepareFileDialog(dialog);
    dialog->exec();

    if (dialog->selectedUrl().isEmpty()) {
        success = false;
    }
    else {
        m_scene->clearSelected();   // Thanks to Peter Soetens for the idea

        // update image url and mime type
        m_imageURL = dialog->selectedUrl();
        QFileInfo f(m_imageURL.toLocalFile());
        m_imageMimeType = UMLViewImageExporterModel::imageTypeToMimeType(f.suffix());
        UMLApp::app()->setImageMimeType(m_imageMimeType);
        logDebug2("UMLViewImageExporter::getParametersFromUser: image mime type=%1"
                  " / URL=%2", m_imageMimeType, m_imageURL.path());
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
void UMLViewImageExporter::prepareFileDialog(UMLFileDialog *fileDialog)
{
    // get all supported mime types
    QStringList mimeTypes = UMLViewImageExporterModel::supportedMimeTypes();

    QHash<QString, QString> configFiles;
    if (!DotGenerator::availableConfigFiles(m_scene, configFiles) || configFiles.size() == 0)
        mimeTypes.removeOne(QStringLiteral("image/x-dot"));

    fileDialog->setCaption(i18n("Save As"));
    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog->setMimeTypeFilters(mimeTypes);

    // set a sensible default filename
    if (m_imageURL.isEmpty()) {
        QUrl docURL = UMLApp::app()->document()->url();
        docURL.setUrl(docURL.toString(QUrl::RemoveFilename)
                      + m_scene->name() + QLatin1Char('.')
                      + UMLViewImageExporterModel::mimeTypeToImageType(m_imageMimeType));
        fileDialog->selectUrl(docURL);
        fileDialog->setSelection(m_scene->name() + QLatin1Char('.') + UMLViewImageExporterModel::mimeTypeToImageType(m_imageMimeType));
    } else {
        fileDialog->setUrl(m_imageURL);
        fileDialog->setSelection(m_imageURL.fileName());
    }
}
