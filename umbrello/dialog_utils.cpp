/*
 *  copyright (C) 2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "dialog_utils.h"

#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>

#include "inputdialog.h"
#include "uml.h"
#include "umlwidget.h"

namespace Umbrello {

QLineEdit* makeLabeledEditField(QGroupBox *containingBox, QGridLayout *layout, int row,
                                QLabel * &label, QString labelText,
                                QLineEdit * &editField, QString editFieldText /* = QString::null */)
{
    label = new QLabel(labelText, containingBox);
    layout->addWidget(label, row, 0);
    editField = new QLineEdit(editFieldText, containingBox);
    layout->addWidget(editField, row, 1 );
    label->setBuddy(editField);
    return editField;
}


void askNameForWidget(
    UMLWidget * &targetWidget, QString dialogTitle, QString dialogPrompt, QString defaultName) {

    bool pressedOK = false;

    QString name = KInputDialog::getText(dialogTitle, dialogPrompt, defaultName, &pressedOK, UMLApp::app());

    if (pressedOK) {
        targetWidget->setName(name);
    } else {
        targetWidget->cleanup();
        delete targetWidget;
        targetWidget = NULL;
    }
}


}  // end namespace Umbrello

