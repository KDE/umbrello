/***************************************************************************
                          classdisplayoptionspage.h
                             -------------------
    copyright            : (C) 2003 Luis De la Parra
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef CLASS_DISPLAY_OPTIONS_PAGE
#define CLASS_DISPLAY_OPTIONS_PAGE

#include "classdisplayoptionsbase.h"
#include "../../../dialogs/dialogpage.h"


namespace Umbrello {

class ClassWidget;

/** @short A Page to control the display options of a ClassWidget
 * 
 * @description A Page to control the display options of a ClassWidget.
 * If the parent widget is null, the page will default to autoApply, but you can
 * change this at any time. 
 * @see also DialogPage
 *
*/
class ClassDisplayOptionsPage : public  ClassDisplayOptionsBase, public DialogPage
{
Q_OBJECT
public:
	/** Constructor.
	 * @param c The ClassWidget to control
	 * @param parent The widget parent, normally a UmbrelloDialog or null
	 * @param name   The name of the page
	 */
	ClassDisplayOptionsPage(ClassWidget *c, QWidget *parent = 0, const char *name = 0 );
	
	/** Destructor */
	virtual ~ClassDisplayOptionsPage( );
	
public slots:
	/** apply changes to the object being observed*/
	virtual void apply();
	/** reset changes and restore values from observed object*/	
	virtual void cancel();
	/** Inform the page that a field has been modified. Do not use (internal) */
	virtual void pageContentsModified();
	/** Load the widget data from the observed object. */
	virtual void loadData();
protected:
	/** Apply changes made in the page to the object being observed */
	virtual void saveData();
	
	ClassWidget *m_widget;
};

}

#endif
