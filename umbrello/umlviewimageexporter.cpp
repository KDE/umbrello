/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umlviewimageexporter.h"

// application specific includes
#include "debug_utils.h"
#include "dotgenerator.h"
#include "umlfiledialog.h"
#include "umlviewimageexportermodel.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"

//kde include files
#if QT_VERSION >= 0x050000
#include <KIO/Job>
#include <KJobWidgets>
#else
#include <kfiledialog.h>
#include <kurl.h>
#include <kio/netaccess.h>
#endif
#include <KLocalizedString>
#include <KMessageBox>

// Qt include files
#if QT_VERSION >= 0x050000
#include <QFileDialog>
#endif
#include <QPointer>
#include <QString>
#include <QStringList>

#define DBG_IE QLatin1String("UMLViewImageExporter")
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
#if QT_VERSION >= 0x050000
        KIO::StatJob *job = KIO::stat(m_imageURL, KIO::StatJob::SourceSide, 0);
        KJobWidgets::setWindow(job, UMLApp::app());
        job->exec();
        bool result = !job->error();
#else
        bool result = KIO::NetAccess::exists(m_imageURL, KIO::NetAccess::SourceSide, UMLApp::app());
#endif
        if (result) {
            int wantSave = KMessageBox::warningContinueCancel(0,
#if QT_VERSION >= 0x050000
                                i18n("The selected file %1 exists.\nDo you want to overwrite it?", m_imageURL.url(QUrl::PreferLocalFile)),
#else
                                i18n("The selected file %1 exists.\nDo you want to overwrite it?", m_imageURL.pathOrUrl()),
#endif
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
#if QT_VERSION >= 0x050000
    QUrl url;
#else
    KUrl url;
#endif
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
#if QT_VERSION >= 0x050000
        QFileInfo f(m_imageURL.toLocalFile());
        m_imageMimeType = UMLViewImageExporterModel::imageTypeToMimeType(f.suffix());
#else
        m_imageMimeType = dialog->currentMimeFilter();
#endif
        UMLApp::app()->setImageMimeType(m_imageMimeType);
        DEBUG(DBG_IE) << "image mime type=" << m_imageMimeType << " / URL=" << m_imageURL;
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
        mimeTypes.removeOne(QLatin1String("image/x-dot"));

    fileDialog->setCaption(i18n("Save As"));
#if QT_VERSION >= 0x050000
    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog->setMimeTypeFilters(mimeTypes);
#else
    fileDialog->setOperationMode(KFileDialog::Saving);
    fileDialog->setMimeFilter(mimeTypes, m_imageMimeType);
#endif

    // set a sensible default filename
    if (m_imageURL.isEmpty()) {
#if QT_VERSION >= 0x050000
        QUrl docURL = UMLApp::app()->document()->url();
        docURL.setUrl(docURL.toString(QUrl::RemoveFilename)
                      + m_scene->name() + QLatin1Char('.')
                      + UMLViewImageExporterModel::mimeTypeToImageType(m_imageMimeType));
        fileDialog->selectUrl(docURL);
#else
        KUrl docURL = UMLApp::app()->document()->url();
        KUrl directory = docURL;
        directory.setPath(docURL.directory());

        fileDialog->setUrl(directory);
#endif
        fileDialog->setSelection(m_scene->name() + QLatin1Char('.') + UMLViewImageExporterModel::mimeTypeToImageType(m_imageMimeType));
    } else {
        fileDialog->setUrl(m_imageURL);
        fileDialog->setSelection(m_imageURL.fileName());
    }
}
