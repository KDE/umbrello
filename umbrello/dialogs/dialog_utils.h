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

/**
 * Dialog utilities.
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
namespace Umbrello {

	/**
	 * Create a labeled text lineedit widget.
	 *
	 * @param containingBox		The containing QGroupBox.
	 * @param layout		The QGridLayout to use.
	 * @param row			The row number within the QGridLayout.
	 * @param label			The QLabel object allocated (return value)
	 * @param labelText		The label text.
	 * @param editField		The QLineEdit object allocated (return value)
	 * @param editFieldText		Initialization text in the editField (optional.)
	 */
	void makeLabeledEditField(QGroupBox *containingBox, QGridLayout *layout, int row,
				  QLabel * &label, QString labelText, 
				  QLineEdit * &editField, QString editFieldText = QString::null);

}

#endif

