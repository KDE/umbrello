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
#include "dontaskagain.h"

// kde includes
#include <KMessageBox>
#include <KLocalizedString>
#include <klineedit.h>
#include <kcombobox.h>

// qt includes
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

DefineDontAskAgainItem(allItem, QLatin1String("all"), i18n("Enable all messages"));
DefineDontAskAgainItem(askDeleteAssociationItem, QLatin1String("delete-association"), i18n("Enable 'delete association' related messages"));
DefineDontAskAgainItem(askDeleteDiagramItem, QLatin1String("delete-diagram"), i18n("Enable 'delete diagram' related messages"));

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
 *                              set to 0, if the user presses Cancel in the dialog.
 * @param dialogTitle           Title of the dialog.
 * @param dialogPrompt          Prompt of the dialog.
 * @param defaultName           Default value of the name field.
 */
void askNameForWidget(UMLWidget * &targetWidget, const QString& dialogTitle,
                      const QString& dialogPrompt, const QString& defaultName)
{
    QString name = defaultName;
    if (askName(dialogTitle, dialogPrompt, name)) {
        targetWidget->setName(name);
    }
    else {
        delete targetWidget;
        targetWidget = 0;
    }
}

/**
 * Helper function for requesting a name using a dialog.
 *
 * @param title           Title of the dialog.
 * @param prompt          Prompt of the dialog.
 * @param name            Default value of the name field.
 * @return true on user pressed okay
 * @return false on user pressed cancel
 */
bool askName(const QString& title, const QString& prompt, QString& name)
{
    bool ok;
#if QT_VERSION >= 0x050000
     name = QInputDialog::getText((QWidget*)UMLApp::app(), title, prompt, QLineEdit::Normal, name, &ok);
#else
     name = KInputDialog::getText(title, prompt, name, &ok, (QWidget*)UMLApp::app());
#endif
     return ok;
}

/**
 * Ask the user for permission to delete an association.
 *
 * @return true - user want to continue
 * @return false - user want to cancel
 */
bool askDeleteAssociation()
{
    return KMessageBox::warningContinueCancel(
        UMLApp::app(),
        i18n("You are about to delete an association. Do you really want to continue?"),
        i18n("Delete Association"),
        KStandardGuiItem::cont(),
        KStandardGuiItem::cancel(),
        askDeleteAssociationItem.name()) == KMessageBox::Continue;
}

/**
 * Ask the user for permission to delete a diagram.
 *
 * @return true - user want to continue
 * @return false - user want to cancel
 */
bool askDeleteDiagram(const QString &name)
{
    QString text = name.isEmpty() ? i18n("You are about to delete the entire diagram.\nAre you sure?")
                                  : i18n("Are you sure you want to delete diagram %1?", name);

    return KMessageBox::warningContinueCancel(
        UMLApp::app(),
        text,
        i18n("Delete Diagram?"),
        KGuiItem(i18n("&Delete")),
        KStandardGuiItem::cancel(),
        askDeleteDiagramItem.name()) == KMessageBox::Continue;
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

