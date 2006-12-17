/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DIALOG_UTILS_H
#define DIALOG_UTILS_H

#include <qstring.h>

class QGroupBox;
class QGridLayout;
class QLabel;
class QLineEdit;
class UMLWidget;

/**
 * Dialog utilities.
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
namespace Dialog_Utils {

/**
 * Create a labeled text lineedit widget.
 *
 * @param containingBox         The containing QGroupBox.
 * @param layout                The QGridLayout to use.
 * @param row                   The row number within the QGridLayout.
 * @param label                 The QLabel object allocated (return value)
 * @param labelText             The label text.
 * @param editField             The QLineEdit object allocated (return value)
 * @param editFieldText         Initialization text in the editField (optional.)
 * @return a pointer to the QLineEdit so you can setFocus() if necessary
 */
QLineEdit* makeLabeledEditField(QGroupBox *containingBox, QGridLayout *layout, int row,
                                QLabel * &label, const QString& labelText,
                                QLineEdit * &editField,
                                const QString& editFieldText = QString::null);

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
                      const QString& dialogPrompt, const QString& defaultName);

}

#endif

