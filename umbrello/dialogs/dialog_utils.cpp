/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "dialog_utils.h"

// app includes
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "stereotype.h"
#include "umlwidget.h"
#include "dontaskagain.h"
#include "model_utils.h"
#include "widget_utils.h"

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
                                KLineEdit*  &editField, const QString& editFieldText /* = QString() */,
                                int columnOffset /* = 0 */)
{
    label = new QLabel(labelText);
    layout->addWidget(label, row, columnOffset);
    editField = new KLineEdit(editFieldText);
    layout->addWidget(editField, row, columnOffset + 1);
    label->setBuddy(editField);
    return editField;
}

/**
 * Make labeled edit fields for stereotype tags.
 * The label/line-edit pairs are arranged horizontally on the same row.
 * The label texts are taken from the AttributeDefs of the object's
 * UMLStereotype.
 */
void makeTagEditFields(UMLObject *o, QGridLayout *genLayout,
                       QLabel    *pTagLabel   [N_STEREOATTRS],
                       KLineEdit *pTagLineEdit[N_STEREOATTRS],
                       int row /* = 1 */)
{
    UMLStereotype *stereo = o->umlStereotype();
    if (stereo == 0)
        return;
    const UMLStereotype::AttributeDefs& attrDefs = stereo->getAttributeDefs();
    const QStringList& tags = o->tags();
    for (int i = 0; i < attrDefs.size() && i < N_STEREOATTRS; i++) {
        const UMLStereotype::AttributeDef& adef = attrDefs[i];
        QString tagInitVal;
        if (i < tags.size())
            tagInitVal = tags.at(i);
        if (tagInitVal.isEmpty())
            tagInitVal = adef.defaultVal;
        Dialog_Utils::makeLabeledEditField(genLayout, row,
                                           pTagLabel[i], adef.name,
                                           pTagLineEdit[i], tagInitVal, 2 + (i * 2));
    }
}

/**
 * Remake labeled edit fields for stereotype tags.
 * "Remake" means that the existing label/line-edit pairs are deleted
 * and new ones are created afresh.
 * This is useful when the object's stereotype has changed.
 * The label/line-edit pairs are arranged horizontally on the same row.
 * The label texts are taken from the AttributeDefs of the object's
 * UMLStereotype.
 */
void remakeTagEditFields(const QString &stereoText,
                         UMLObject *, QGridLayout * genLayout,
                         QLabel    * pTagLabel[N_STEREOATTRS],
                         KLineEdit * pTagLineEdit[N_STEREOATTRS],
                         int row /* = 1 */)
{
    // Remove existing tag input fields
    for (int i = N_STEREOATTRS - 1; i >= 0; --i) {
        if (pTagLabel[i]) {
            delete pTagLabel [i];
            delete pTagLineEdit[i];
            pTagLabel [i] = 0;
            pTagLineEdit[i] = 0;
        }
    }
    UMLStereotype *stereo = 0;
    foreach (UMLStereotype *st, UMLApp::app()->document()->stereotypes()) {
        if (st->name() == stereoText) {
            stereo = st;
            break;
        }
    }
    if (stereo == 0)
        return;
    const UMLStereotype::AttributeDefs& attrDefs = stereo->getAttributeDefs();
    for (int i = 0; i < attrDefs.size() && i < N_STEREOATTRS; i++) {
        const UMLStereotype::AttributeDef& adef = attrDefs[i];
        QString tagInitVal = adef.defaultVal;
        Dialog_Utils::makeLabeledEditField(genLayout, row,
                                           pTagLabel[i], adef.name,
                                           pTagLineEdit[i], tagInitVal, 2 + (i * 2));
    }
}

/**
 * Update the stereotype tag values of the given UMLObject from the
 * corresponding values in the given array of KLineEdit widgets.
 * This is useful as the action in the slot method when the Apply or
 * OK button is pressed.
 */
void updateTagsFromEditFields(UMLObject * o,
                              KLineEdit *pTagLineEdit[N_STEREOATTRS])
{
    UMLStereotype *stereo = o->umlStereotype();
    if (stereo == 0)
        return;
    const UMLStereotype::AttributeDefs& attrDefs = stereo->getAttributeDefs();
    QStringList& tags = o->tags();
    tags.clear();
    for (int i = 0; i < attrDefs.size() && i < N_STEREOATTRS; i++) {
        if (pTagLineEdit[i] == 0) {
            uError() << "updateTagsFromEditFields(" << o->name() << "): " << stereo->name(true)
                     << " pTagLineEdit[" << i << "] is null";
            break;
        }
        QString tag = pTagLineEdit[i]->text();
        tags.append(tag);
    }
}

/**
 * Helper function for requesting a name for a UMLWidget using a dialog.
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
 * Ask the user for a new widget name
 *
 * @return true on user pressed okay
 * @return false on user pressed cancel
 */
bool askNewName(WidgetBase::WidgetType type, QString &name)
{
    QString title = Widget_Utils::newTitle(type);
    QString text = Widget_Utils::newText(type);
    return askName(title, text, name);
}

/**
 * Ask the user for renaming a widget name
 *
 * @return true on user pressed okay
 * @return false on user pressed cancel
 */
bool askRenameName(WidgetBase::WidgetType type, QString &name)
{
    QString title = Widget_Utils::renameTitle(type);
    QString text = Widget_Utils::renameText(type);
    return askName(title, text, name);
}

/**
 * Ask the user for a default new widget name
 *
 * The name is predefined by the widgets type default name
 *
 * @return true on user pressed okay
 * @return false on user pressed cancel
 */
bool askDefaultNewName(WidgetBase::WidgetType type, QString &name)
{
    name = Widget_Utils::defaultWidgetName(type);
    return askNewName(type, name);
}

/**
 * Ask the user for a new object name
 *
 * @return true on user pressed okay
 * @return false on user pressed cancel
 */
bool askNewName(UMLObject::ObjectType type, QString &name)
{
    QString title = Model_Utils::newTitle(type);
    QString text = Model_Utils::newText(type);
    return askName(title, text, name);
}

/**
 * Ask the user for renaming a widget name
 *
 * @return true on user pressed okay
 * @return false on user pressed cancel
 */
bool askRenameName(UMLObject::ObjectType type, QString &name)
{
    QString title = Model_Utils::renameTitle(type);
    QString text = Model_Utils::renameText(type);
    return askName(title, text, name);
}

/**
 * Ask the user for a default new widget name
 *
 * The name is predefined by the widgets type default name
 *
 * @return true on user pressed okay
 * @return false on user pressed cancel
 */
bool askDefaultNewName(UMLObject::ObjectType type, QString &name)
{
    name = Model_Utils::uniqObjectName(type, nullptr);
    return askNewName(type, name);
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

