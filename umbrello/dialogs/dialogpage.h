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

#include <qobject.h>

/** @short A basic interface for all Dialog Pages in Umbrello. */

class DialogPage : public QObject
{
Q_OBJECT
public:
	DialogPage( QObject *parent = 0, const char *name = 0 ) : QObject( parent, name ) {}
public slots:
/** apply changes to the object being handled*/
	virtual void apply() = 0;
/** reset changes and restore values from object being handled*/	
	virtual void cancel() = 0;
	
	
signals:
/** Signals whether the page is ready to apply changes and be dismissed,
    ie, if it contains valid data */
	void pageOk( bool );

};

#endif

