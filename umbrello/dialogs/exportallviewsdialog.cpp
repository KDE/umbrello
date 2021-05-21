/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "exportallviewsdialog.h"

// kde include files
#include <KLocalizedString>

// application specific includes
#include "umlviewimageexportermodel.h"

/**
 * Constructor for UMLViewImageExporterModel.
 *
 * @param parent The parent of the dialog.
 * @param name The internal name.
 *
 * @see KDialog::KDialog
 */
ExportAllViewsDialog::ExportAllViewsDialog(QWidget* parent, const char* name)
  : SinglePageDialogBase(parent)
{
    setObjectName(QString::fromLatin1(name));
    setupUi(mainWidget());

    // create and initialize m_imageType
    m_imageType = new ImageTypeWidget(UMLViewImageExporterModel::supportedMimeTypes(), QLatin1String("image/png"), this);
    m_imageResolution = new ResolutionWidget(this);

    // Cannot give an object name to the layout when using QtDesigner,
    // therefore go and use an editor and add it by hand.
    ui_imageTypeLayout->addWidget(m_imageType);
    ui_imageResolutionLayout->addWidget(m_imageResolution);

    // reload the strings so the m_imageType tooltip is added
    languageChange();
    connect(m_imageType, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotImageTypeChanged(QString)));

    m_kURL->setMode(KFile::Directory | KFile::ExistingOnly);
}

/**
 * Destructor for UMLViewImageExporterModel.
 */
ExportAllViewsDialog::~ExportAllViewsDialog()
{
}

/**
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ExportAllViewsDialog::languageChange()
{
    //ExportAllViewsDialogBase::languageChange();
    m_imageType->setToolTip(i18n("The format that the images will be exported to"));
}

/**
 * Enable image resolution widget only for supported export types.
 *
 * @param imageType String with selected image type
 */
void ExportAllViewsDialog::slotImageTypeChanged(QString imageType)
{
    Q_UNUSED(imageType);
    QString mimeType = m_imageType->currentType();
    bool hide = mimeType == QLatin1String("image/x-dot") ||
        mimeType == QLatin1String("image/x-eps") ||
        mimeType == QLatin1String("image/svg+xml");

    m_imageResolution->setVisible(!hide);
}
