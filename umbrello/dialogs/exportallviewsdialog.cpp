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

    // Cannot give an object name to the layout when using QtDesigner,
    // therefore go and use an editor and add it by hand.
    ui_imageTypeLayout->addWidget(m_imageType);


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

