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
#include "exportallviewsdialog.h"

// include files for Qt
#include <QLayout>
#include <QLabel>

// kde include files
#include <kfilefiltercombo.h>
#include <klocale.h>

// application specific includes
#include "../umlviewimageexportermodel.h"

ExportAllViewsDialog::ExportAllViewsDialog(
        QWidget* parent /* = 0 */,
        const char* name /* = 0 */,
        bool modal /* = false */,
        Qt::WindowFlags fl /* = 0*/,
        const QString &defaultMimeType /*= "image/png"*/)
  : QDialog(parent,fl) {

    setObjectName(name);
    setModal(modal);
    setupUi(this);

    // create and initialize m_imageType
    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sp.setHorizontalStretch(0);
    sp.setVerticalStretch(0);
    sp.setHeightForWidth(m_imageType->sizePolicy().hasHeightForWidth());
    m_imageType = new KFileFilterCombo(this);
    m_imageType->setSizePolicy(sp);
    m_imageType->setEditable(false);

    m_imageType->setMimeFilter(UMLViewImageExporterModel::supportedMimeTypes(), defaultMimeType);

/// GC: @todo verify if it should be ported to KDE4 or just removed
//     imageTypeLayout->addWidget(m_imageType);

    imageTypeLabel->setBuddy(m_imageType);

    // reload the strings so the m_imageType tooltip is added
    languageChange();
}

void ExportAllViewsDialog::languageChange() {
    //ExportAllViewsDialogBase::languageChange();
    m_imageType->setToolTip(i18n("The format that the images will be exported to"));
}

#include "exportallviewsdialog.moc"

