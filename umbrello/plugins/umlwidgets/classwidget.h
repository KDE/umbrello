/***************************************************************************
                                  classwidget.h
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
 
#ifndef UMBRELLO_CLASSWIDGET_H
#define UMBRELLO_CLASSWIDGET_H

#include "umlwidget.h"
#include <qstringlist.h>

class Diagram;
class UMLClass;

namespace Umbrello{


/** Widget to represent a UML Class in a diagram */
class ClassWidget : public UMLWidget
{
Q_OBJECT

public: enum DisplayOptions { ShowPackage = 1, ShowStereotype = 2,
			ShowAtts = 1, ShowOps = 1, ShowType = 2, ShowVisibility = 4, 
			ShowInitialValue = 8, ShowParameterList = 16 };

	/** Constructor
	* @param diagram The diagram the widget belongs to
	* @param id    the unique ID of this widget
	* @param object The UML Class the widget represents
	*/
	ClassWidget(Diagram *diagram, uint id, UMLClass *object);
	
	/** Destructor */
	virtual ~ClassWidget();
	
	virtual void fillContextMenu(QPopupMenu &menu);
	
	inline int  nameDisplayOptions( ) const;
	void setNameDisplayOptions( int );
	inline int  attsDisplayOptions( ) const;
	void setAttsDisplayOptions( int );
	inline int  opsDisplayOptions( ) const;
	void setOpsDisplayOptions( int );
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
	struct AttString { TextFlags flags; QString string; };
	struct OpString  { TextFlags flags; QString string; };
//FIXME - rename this method...
	void calculateSize();
	
	virtual	void drawShape(QPainter &);
	
	int m_nameDisplayOpts;
	int m_attsDisplayOpts;
	int m_opsDisplayOpts;
	
	QString m_stereotype;
	QString m_name;
	QValueList<AttString> m_atts;
	QValueList<OpString>  m_ops;
};

int ClassWidget::nameDisplayOptions( ) const
{ return m_nameDisplayOpts;}
int ClassWidget::attsDisplayOptions( ) const
{ return m_attsDisplayOpts; }
int ClassWidget::opsDisplayOptions( ) const
{ return m_opsDisplayOpts; }

} //end of namespace Umbrello

#endif // UMBRELLO_CLASSWIDGET_H
