/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIALOG_PAGE
#define DIALOG_PAGE

#include <qwidget.h>

/** @short A basic interface for all Dialog Pages in Umbrello. */

class DialogPage : public QWidget
{
Q_OBJECT
public:
	DialogPage( QWidget *parent = 0, const char *name = 0, WFlags fl = 0  ) : QWidget( parent, name, fl )
	 { if(!parent) setAutoApply( true ); }
public slots:
/** apply changes to the object being handled*/
	virtual void apply() = 0;
/** reset changes and restore values from object being handled*/
	virtual void cancel() = 0;
	
/** If auto apply is true, the observed object will be modified inmediatly
  * when the user changes something in the dialog */	
	inline void setAutoApply( bool a ) {m_autoApply = a;}
	
//	standAlone

signals:
	void pageModified( );
protected:
	bool m_autoApply;

};



#endif

