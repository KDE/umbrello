/***************************************************************************
                                  packagewidget.h
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
 
#ifndef UMBRELLO_PACKAGEWIDGET_H
#define UMBRELLO_PACKAGEWIDGET_H

#include "umlwidget.h"
#include <qstringlist.h>

class Diagram;
class UMLPackage;

namespace Umbrello{


/** Widget to represent a UML Package in a diagram */
class PackageWidget : public UMLWidget
{
Q_OBJECT

public: enum DisplayOptions { ShowPackage = 1, ShowStereotype = 2 };

	/** Constructor
	* @param diagram The diagram the widget belongs to
	* @param id    the unique ID of this widget
	* @param object The UML Package the widget represents
	*/
	PackageWidget(Diagram *diagram, uint id, UMLPackage *object);
	
	/** Destructor */
	virtual ~PackageWidget();
	
	virtual void fillContextMenu(QPopupMenu &menu);
	
	
public slots:
	/** Reimplemented from UMLWidget. re-calculate internal data and repaint the widget*/
	virtual void umlObjectModified();
	/** Show a dialog to edit the properties of the UML Class the widget represents, and of
	* the widget itself
	*/
	virtual void editProperties();
	
protected:
	enum TextFlags { EmptyFlag = 0x0, Bold = 0x1, Italics = 0x2, Underline = 0x4 };
//FIXME - rename this method...
	void calculateSize();
	
	virtual	void drawShape(QPainter &);
	
	int m_nameDisplayOpts;

	QString m_stereotype;
	QString m_name;

	static const int vMargin = 10;
	static const int hMargin = 10;
	static const int tabHeight = 15;
	static int lineHeight;
};

} //end of namespace Umbrello

#endif // UMBRELLO_PACKAGEWIDGET_H
