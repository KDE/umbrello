/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NOTEDIALOG_H
#define NOTEDIALOG_H
//qt includes
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qtextedit.h>
//kde includes
#include <kdialogbase.h>
//app includes
#include "../notewidget.h"

/**
 * @author Paul Hensgen
 */

class NoteDialog : public KDialogBase {
public:
	/**
	 *	Constructs an NoteDialog.
	 */
	NoteDialog( QWidget * parent, NoteWidget * pNote );

	/**
	 *	Standard deconstructor.
	 */
	~NoteDialog();

public slots:
	void slotOk();
private:
	//GUI widgets
	QGroupBox * m_pDocGB;
	QTextEdit * m_pDocTE;

	/**
	 *	Note widget to show documentation for.
	 */
	NoteWidget * m_pNoteWidget;
};

#endif
