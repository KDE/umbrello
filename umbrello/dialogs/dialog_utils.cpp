/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "dialog_utils.h"

#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>

namespace Umbrello {

void makeLabeledEditField(QGroupBox *containingBox, QGridLayout *layout, int row,
			  QLabel * &label, QString labelText, 
			  QLineEdit * &editField, QString editFieldText /* = QString::null */)
{
	label = new QLabel(labelText, containingBox);
	layout->addWidget(label, row, 0);
	editField = new QLineEdit(editFieldText, containingBox);
	layout->addWidget(editField, row, 1 );
	label->setBuddy(editField);
}


}

