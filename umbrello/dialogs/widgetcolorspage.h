/***************************************************************************
                          widgetcolorspage.h
                             -------------------
    copyright            : (C) 2003 Luis De la Parra
  Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef WIDGET_COLORS_PAGE_H
#define WIDGET_COLORS_PAGE_H
 
 #include "widgetcolorsbase.h"
 #include "dialogpage.h"
 
 
 namespace Umbrello{
 
 class DiagramElement;
 
 class WidgetColorsPage : public WidgetColorsBase, public DialogPage
 {
 Q_OBJECT
 public:
 	WidgetColorsPage( DiagramElement *e, QWidget *parent = 0, const char *name = 0 );
	virtual ~WidgetColorsPage();
public slots:
	/** apply changes to the object being observed*/
	virtual void apply();
	/** reset changes and restore values from observed object*/	
	virtual void cancel();
	/** Inform the page that a field has been modified. Do not use (internal) */
	virtual void pageContentsModified();
	/** Load the widget data from the UMLObject. */
	virtual void loadData();

signals:
	void pageModified( );

protected:
	/** Apply changes made in the page to the UMLClass being observed */
	virtual void saveData();

	DiagramElement *m_element;
 };
 
 }
 
 
 
 #endif
