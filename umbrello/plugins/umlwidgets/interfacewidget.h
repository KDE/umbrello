/***************************************************************************
                                  interfacewidget.h
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
 
#ifndef UMBRELLO_INTERFACEWIDGET_H
#define UMBRELLO_INTERFACEWIDGET_H

#include "umlwidget.h"
#include <qstringlist.h>

class Diagram;
class UMLInterface;

namespace Umbrello{


/** Widget to represent a UML Interface in a diagram */
class InterfaceWidget : public UMLWidget
{
Q_OBJECT

public: enum DisplayOptions { ShowPackage = 1, ShowStereotype = 2,
			ShowOps = 1, ShowType = 2, ShowVisibility = 4, 
			ShowParameterList = 16 };

	/** Constructor
	* @param diagram The diagram the widget belongs to
	* @param id    the unique ID of this widget
	* @param object The UML Interface the widget represents
	*/
	InterfaceWidget(Diagram *diagram, uint id, UMLInterface *object);
	
	/** Destructor */
	virtual ~InterfaceWidget();
	
	virtual void fillContextMenu(QPopupMenu &menu);
	
	
public slots:
	/** Reimplemented from UMLWidget. re-calculate internal data and repaint the widget*/
	virtual void umlObjectModified();
	/** Show a dialog to edit the properties of the UML Class the widget represents, and of
	* the widget itself
	*/
	virtual void editProperties();
	
	void refactor( );
	
protected:
	enum TextFlags { EmptyFlag = 0x0, Bold = 0x1, Italics = 0x2, Underline = 0x4 };
	struct OpString  { TextFlags flags; QString string; };
//FIXME - rename this method...
	void calculateSize();
	
	virtual	void drawShape(QPainter &);
	
	int m_nameDisplayOpts;
	int m_opsDisplayOpts;
	
	QString m_stereotype;
	QString m_name;
	QValueList<OpString>  m_ops;

	static const int vMargin = 10;
	static const int hMargin = 10;
	static int lineHeight;
};

} //end of namespace Umbrello

#endif // UMBRELLO_INTERFACEWIDGET_H
