/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umlviewimageexporterall.h"

// application specific includes
#include "diagramprintpage.h"
#include "exportallviewsdialog.h"
#include "umlviewimageexportermodel.h"
#include "uml.h"
#include "umldoc.h"

// kde include files
#include <KComboBox>
#include <KLocalizedString>
#include <kurlrequester.h>
#include <KMessageBox>

// Qt include files
#include <QString>
#include <QStringList>
#include <QUrl>

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
 * Export views selected by the DiagramPrintPage instance.
 *
 * See @ref exportViews(const UMLViewList &views) for more details.
 *
 * @param selectPage instance of DiagramPrintPage
 */
void UMLViewImageExporterAll::exportViews(DiagramPrintPage *selectPage)
{
    UMLViewList views;
    int count = selectPage->printUmlCount();
    for (int i = 0; i < count; ++i) {
        QString sID = selectPage->printUmlDiagram(i);
        Uml::ID::Type id = Uml::ID::fromString(sID);
        UMLView *v = UMLApp::app()->document()->findView(id);
        if (v)
            views.append(v);
    }
    exportViews(views);
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
void UMLViewImageExporterAll::exportViews(const UMLViewList &views)
{
    UMLApp* app = UMLApp::app();
    UMLDoc* umlDoc = app->document();

    // default url can't be set when creating the action because the
    // document wasn't loaded
    if (m_dialog->m_kURL->url().isEmpty()) {
        QUrl directory(umlDoc->url());
        m_dialog->m_kURL->setUrl(directory.adjusted(QUrl::RemoveFilename));
    }

    if (m_dialog->exec() == QDialog::Rejected) {
        return;
    }

    app->setImageMimeType(m_dialog->m_imageType->currentType());
    float resolution = m_dialog->m_imageResolution->currentResolution();
    // export all views
    umlDoc->writeToStatusBar(i18n("Exporting all views..."));
    QStringList errors = UMLViewImageExporterModel(resolution).exportViews(views,
                                UMLViewImageExporterModel::mimeTypeToImageType(m_dialog->m_imageType->currentType()),
                                QUrl(m_dialog->m_kURL->url()),
                                m_dialog->m_useFolders->isChecked());
    if (!errors.empty()) {
        KMessageBox::errorList(app, i18n("Some errors happened when exporting the images:"), errors);
    }
    umlDoc->writeToStatusBar(i18nc("reset status bar", "Ready."));
}
