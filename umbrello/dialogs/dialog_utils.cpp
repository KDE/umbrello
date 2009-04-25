/***************************************************************************
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

// app includes
#include "uml.h"
#include "umlwidget.h"

// kde includes
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <klineedit.h>

// qt includes
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>

namespace Dialog_Utils {

/**
 * Create a labeled text lineedit widget.
 *
 * @param containingBox         The containing QGroupBox (unused).
 * @param layout                The QGridLayout to use.
 * @param row                   The row number within the QGridLayout.
 * @param label                 The QLabel object allocated (return value)
 * @param labelText             The label text.
 * @param editField             The KLineEdit object allocated (return value)
 * @param editFieldText         Initialization text in the editField (optional.)
 * @return a pointer to the KLineEdit so you can setFocus() if necessary
 */
KLineEdit* makeLabeledEditField(QGroupBox *containingBox, QGridLayout *layout, int row,QLabel * &label, const QString& labelText, KLineEdit * &editField, const QString& editFieldText /* = QString() */)
{
    Q_UNUSED(containingBox);
    label = new QLabel(labelText);
    layout->addWidget(label, row, 0);
    editField = new KLineEdit(editFieldText);
    layout->addWidget(editField, row, 1 );
    label->setBuddy(editField);
    return editField;
}

/**
 * Helper function for requesting a name for an UMLWidget using a dialog.
 *
 * @param targetWidget          By-reference pointer to the widget to request the name for.
 *                              The widget may be deallocated, and the pointer returned
 *                              set to NULL, if the user presses Cancel in the dialog.
 * @param dialogTitle           Title of the dialog.
 * @param dialogPrompt          Prompt of the dialog.
 * @param defaultName           Default value of the name field.
 */
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

