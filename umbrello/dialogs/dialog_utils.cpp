#include "dialog_utils.h"

#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>

namespace Dialog_Utils {

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

