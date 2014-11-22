/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "dialog_utils.h"

// app includes
#include "uml.h"
#include "umldoc.h"
#include "stereotype.h"
#include "umlwidget.h"

// kde includes
#include <kmessagebox.h>
#include <KLocalizedString>
#include <klineedit.h>
#include <kcombobox.h>

// qt includes
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>

namespace Dialog_Utils {

/**
 * Create a labeled text lineedit widget.
 *
 * @param layout                The QGridLayout to use.
 * @param row                   The row number within the QGridLayout.
 * @param label                 The QLabel object allocated (return value)
 * @param labelText             The label text.
 * @param editField             The KLineEdit object allocated (return value)
 * @param editFieldText         Initialization text in the editField (optional.)
 * @return a pointer to the KLineEdit so you can setFocus() if necessary
 */
KLineEdit* makeLabeledEditField(QGridLayout* layout,    int row,
                                QLabel*     &label,     const QString& labelText,
                                KLineEdit*  &editField, const QString& editFieldText /* = QString() */)
{
    label = new QLabel(labelText);
    layout->addWidget(label, row, 0);
    editField = new KLineEdit(editFieldText);
    layout->addWidget(editField, row, 1);
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

    QString name = QInputDialog::getText(UMLApp::app(),
                                         dialogTitle, dialogPrompt,
                                         QLineEdit::Normal,
                                         defaultName, &pressedOK);

    if (pressedOK) {
        targetWidget->setName(name);
    }
    else {
        delete targetWidget;
        targetWidget = NULL;
    }
}

/**
 * Helper function for inserting available stereotypes into a KComboBox
 *
 * @param kcb    The KComboBox into which to insert the stereotypes
 * @param type   The stereotype to activate
 */
void insertStereotypesSorted(KComboBox *kcb, const QString& type)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    QStringList types;
    types << QString();  // an empty stereotype is the default
    foreach (UMLStereotype* ust, umldoc->stereotypes()) {
        types << ust->name();
    }
    // add the given parameter
    if (!types.contains(type)) {
        types << type;
    }
    types.sort();

    kcb->clear();
    kcb->insertItems(-1, types);

    // select the given parameter
    int currentIndex = kcb->findText(type);
    if (currentIndex > -1) {
        kcb->setCurrentIndex(currentIndex);
    }
    kcb->completionObject()->addItem(type);
}

}  // end namespace Dialog_Utils

