/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIALOG_UTILS_H
#define DIALOG_UTILS_H

#include <qstring.h>

class QGroupBox;
class QGridLayout;
class QLabel;
class QLineEdit;

namespace Dialog_Utils {

	void makeLabeledEditField(QGroupBox *containingBox, QGridLayout *layout, int row,
				  QLabel * &label, QString labelText, 
				  QLineEdit * &editField, QString editFieldText = QString::null);

}

#endif

