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
#include "umlviewimageexporter.h"

// include files for Qt
#include <qstring.h>
#include <qstringlist.h>

//kde include files
#include <klocale.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>

// application specific includes
#include "umlviewimageexportermodel.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"


UMLViewImageExporter::UMLViewImageExporter(UMLView* view) {
    m_view = view;
    m_imageMimeType = UMLApp::app()->getImageMimeType();
}

void UMLViewImageExporter::exportView() {
    if (!prepareExportView()) {
        return;
    }

    UMLApp *app = UMLApp::app();

    // export the view
    app->getDocument()->writeToStatusBar(i18n("Exporting view..."));
    QString error = UMLViewImageExporterModel().exportView(m_view,
                            UMLViewImageExporterModel::mimeTypeToImageType(m_imageMimeType), m_imageURL);
    if (!error.isNull()) {
        KMessageBox::error(app, i18n("An error happened when exporting the image:\n") + error);
    }
    app->getDocument()->writeToStatusBar(i18n("Ready."));
}

bool UMLViewImageExporter::prepareExportView() {
    bool exportPrepared = false;

    do {
        if (!getParametersFromUser()) {
            return false;
        }

        // check if the file exists
        if (KIO::NetAccess::exists(m_imageURL, true, UMLApp::app())) {
            int wantSave = KMessageBox::warningContinueCancel(0,
                                i18n("The selected file %1 exists.\nDo you want to overwrite it?").arg(m_imageURL.prettyURL()),
                                i18n("File Already Exists"), i18n("&Overwrite"));
            if (wantSave == KMessageBox::Continue) {
                exportPrepared = true;
            }
        } else {
            exportPrepared = true;
        }
    } while (!exportPrepared);

    return true;
}

bool UMLViewImageExporter::getParametersFromUser() {
    UMLApp *app = UMLApp::app();

    // configure & show the file dialog
    KFileDialog fileDialog(QString::null, QString::null, m_view,
                           ":export-image", true);
    prepareFileDialog(fileDialog);
    fileDialog.exec();

    if (fileDialog.selectedURL().isEmpty())
        return false;
    m_view->clearSelected();   // Thanks to Peter Soetens for the idea

    // update image url and mime type
    m_imageMimeType = fileDialog.currentMimeFilter();
    app->setImageMimeType(m_imageMimeType);
    m_imageURL = fileDialog.selectedURL();

    // check if the extension is the extension of the mime type
    QFileInfo info(m_imageURL.filename());
    QString ext = info.extension(false);
    QString extDef = UMLViewImageExporterModel::mimeTypeToImageType(m_imageMimeType);
    if(ext != extDef) {
        m_imageURL.setFileName(m_imageURL.fileName() + '.' + extDef);
    }

    return true;
}

void UMLViewImageExporter::prepareFileDialog(KFileDialog &fileDialog) {
    // get all supported mime types
    QStringList mimeTypes = UMLViewImageExporterModel::supportedMimeTypes();

    fileDialog.setCaption(i18n("Save As"));
    fileDialog.setOperationMode(KFileDialog::Saving);
    fileDialog.setMimeFilter(mimeTypes, m_imageMimeType);

    // set a sensible default filename
    if (m_imageURL.isEmpty()) {
        KURL docURL = UMLApp::app()->getDocument()->URL();
        KURL directory = docURL;
        directory.setPath(docURL.directory());

        fileDialog.setURL(directory);
        fileDialog.setSelection(m_view->getName() + '.' + UMLViewImageExporterModel::mimeTypeToImageType(m_imageMimeType));
    } else {
        fileDialog.setURL(m_imageURL);
        fileDialog.setSelection(m_imageURL.fileName());
    }
}
