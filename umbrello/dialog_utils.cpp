/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "dialog_utils.h"

// qt includes
#include <q3groupbox.h>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QGridLayout>

// kde includes
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <klineedit.h>

// app includes
#include "uml.h"
#include "umlwidget.h"

namespace Dialog_Utils {

/* deprecated */
KLineEdit* makeLabeledEditField(Q3GroupBox *containingBox, QGridLayout *layout, int row,QLabel * &label, const QString& labelText, KLineEdit * &editField, const QString& editFieldText /* = QString() */)
{
    label = new QLabel(labelText, containingBox);
    layout->addWidget(label, row, 0);
    editField = new KLineEdit(editFieldText, containingBox);
    layout->addWidget(editField, row, 1 );
    label->setBuddy(editField);
    return editField;
}

KLineEdit* makeLabeledEditField(QGroupBox *containingBox, QGridLayout *layout, int row,QLabel * &label, const QString& labelText, KLineEdit * &editField, const QString& editFieldText /* = QString() */)
{
    label = new QLabel(labelText, containingBox);
    layout->addWidget(label, row, 0);
    editField = new KLineEdit(editFieldText, containingBox);
    layout->addWidget(editField, row, 1 );
    label->setBuddy(editField);
    return editField;
}


void askNameForWidget(UMLWidget * &targetWidget, const QString& dialogTitle,
                      const QString& dialogPrompt, const QString& defaultName)
{
    bool pressedOK = false;

    QString name = KInputDialog::getText(dialogTitle, dialogPrompt, defaultName, &pressedOK, UMLApp::app());

    if (pressedOK) {
        targetWidget->setName(name);
    }
    else {
        targetWidget->cleanup();
        delete targetWidget;
        targetWidget = NULL;
    }
}

}  // end namespace Dialog_Utils

