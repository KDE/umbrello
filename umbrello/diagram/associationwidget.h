/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIA_ASSOCIATIONWIDGET
#define DIA_ASSOCIATIONWIDGET

#include "umlwidget.h"


namespace Umbrello{

class AssociationWidget: public UMLWidget
{
Q_OBJECT
public:
	enum PathStyle {Orthogonal, Direct};
	
	AssociationWidget(Diagram *diagram, uint id, UMLAssociation *assoc, UMLWidget *start, UMLWidget *end);
	virtual ~AssociationWidget();
	
	virtual void setPath( QPointArray path );
	
	virtual QPointArray areaPoints() const;
	
public slots:
	virtual void umlObjectModified();
	virtual void widgetMoved();
protected:
	virtual void drawShape(QPainter &);
	void calculateSize();
	UMLWidget *m_widgetA, *m_widgetB;
	QPointArray m_path;
	PathStyle m_style;
	
//redefine areapoints
};
}

#endif
