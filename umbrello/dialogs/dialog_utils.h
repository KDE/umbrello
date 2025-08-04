/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DIALOG_UTILS_H
#define DIALOG_UTILS_H

#include "umlobject.h"
#include "widgetbase.h"
#include "n_stereoattrs.h"

#include <QString>
#include <QInputDialog>

class QGridLayout;
class QLabel;
class UMLWidget;
class QLineEdit;
class KComboBox;

/**
 * Dialog utilities.
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
namespace Dialog_Utils {

QLineEdit* makeLabeledEditField(QGridLayout *layout, int row,
                                QLabel * &label, const QString& labelText,
                                QLineEdit * &editField,
                                const QString& editFieldText = QString(),
                                int columnOffset = 0);

void makeTagEditFields(UMLObject * o, QGridLayout * genLayout,
                       QLabel * pTagLabel[N_STEREOATTRS],
                       QLineEdit *pTagLineEdit[N_STEREOATTRS],
                       int row = 1);

void remakeTagEditFields(const QString &stereoText,
                         UMLObject * o, QGridLayout * genLayout,
                         QLabel * pTagLabel[N_STEREOATTRS],
                         QLineEdit * pTagLineEdit[N_STEREOATTRS],
                         int row = 1);

void updateTagsFromEditFields(UMLObject * o,
                              QLineEdit *pTagLineEdit[N_STEREOATTRS]);

void askNameForWidget(UMLWidget * &targetWidget, const QString& dialogTitle,
                      const QString& dialogPrompt, const QString& defaultName);

bool askName(const QString& title, const QString& prompt, QString& name);

bool askNewName(WidgetBase::WidgetType type, QString &name);
bool askRenameName(WidgetBase::WidgetType type, QString &name);
bool askDefaultNewName(WidgetBase::WidgetType type, QString &name);

bool askNewName(UMLObject::ObjectType type, QString &name);
bool askRenameName(UMLObject::ObjectType type, QString &name);
bool askDefaultNewName(UMLObject::ObjectType type, QString &name);

void insertStereotypesSorted(KComboBox *kcb, const QString& type);

bool askDeleteAssociation();
bool askDeleteDiagram(const QString &name = QString());

int spacingHint();

qreal logicalDpiXForWidget(const QWidget *widget = nullptr);
QSize screenSizeForWidget(QWidget* widget = nullptr);
}

#endif

