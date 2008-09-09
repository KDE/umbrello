/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DIALOG_UTILS_H
#define DIALOG_UTILS_H

#include <QtCore/QString>

class QGroupBox;
class QGridLayout;
class QLabel;
class UMLWidget;
class KLineEdit;

/**
 * Dialog utilities.
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
namespace Dialog_Utils {

KLineEdit* makeLabeledEditField(QGroupBox *containingBox, QGridLayout *layout, int row,
                                QLabel * &label, const QString& labelText,
                                KLineEdit * &editField,
                                const QString& editFieldText = QString());

void askNameForWidget(UMLWidget * &targetWidget, const QString& dialogTitle,
                      const QString& dialogPrompt, const QString& defaultName);


}

#endif

