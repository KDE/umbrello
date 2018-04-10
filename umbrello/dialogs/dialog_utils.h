/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef DIALOG_UTILS_H
#define DIALOG_UTILS_H

#include <QString>

#if QT_VERSION < 0x050000
#include <kinputdialog.h>
#else
#include <QInputDialog>
#endif

class QGridLayout;
class QLabel;
class UMLWidget;
class KLineEdit;
class KComboBox;

/**
 * Dialog utilities.
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
namespace Dialog_Utils {

KLineEdit* makeLabeledEditField(QGridLayout *layout, int row,
                                QLabel * &label, const QString& labelText,
                                KLineEdit * &editField,
                                const QString& editFieldText = QString());

void askNameForWidget(UMLWidget * &targetWidget, const QString& dialogTitle,
                      const QString& dialogPrompt, const QString& defaultName);

bool askName(const QString& title, const QString& prompt, QString& name);

void insertStereotypesSorted(KComboBox *kcb, const QString& type);

bool askDeleteAssociation();
bool askDeleteDiagram(const QString &name = QString());
}

#endif

