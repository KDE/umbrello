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
#include "exportallviewsdialog.h"

// kde include files
#include <kfilefiltercombo.h>
#include <klocale.h>

// application specific includes
#include "umlviewimageexportermodel.h"

/**
 * Constructor for UMLViewImageExporterModel.
 *
 * @param parent The parent of the dialog.
 * @param name The internal name.
 * @param modal If modal is true the dialog will block input to other the windows
 *              in the application until it's closed.
 * @param fl Window flags.
 * @param defaultMimeType The default mime type that appears in the mime types list.
 *
 * @see KDialog::KDialog
 */
ExportAllViewsDialog::ExportAllViewsDialog(
        QWidget* parent /* = 0 */,
        const char* name /* = 0 */,
        bool modal /* = false */,
        Qt::WindowFlags fl /* = 0*/,
        const QString& defaultMimeType /*= "image/png"*/)
  : KDialog(parent,fl)
{
    setObjectName(name);
    setModal(modal);
    setupUi(mainWidget());

    // create and initialize m_imageType
    m_imageType = new KFileFilterCombo(this);
    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sp.setHorizontalStretch(0);
    sp.setVerticalStretch(0);
    sp.setHeightForWidth(m_imageType->sizePolicy().hasHeightForWidth());
    m_imageType->setSizePolicy(sp);
    m_imageType->setEditable(false);

    m_imageType->setMimeFilter(UMLViewImageExporterModel::supportedMimeTypes(), defaultMimeType);

    // Cannot give an object name to the layout when using QtDesigner,
    // therefore go and use an editor and add it by hand.
    ui_imageTypeLayout->addWidget(m_imageType);

    ui_imageTypeLabel->setBuddy(m_imageType);

    // reload the strings so the m_imageType tooltip is added
    languageChange();

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

#include "exportallviewsdialog.moc"
