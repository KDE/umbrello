 /***************************************************************************
                               umlwidget.cpp
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

#include "umlwidget.h"
#include "associationwidget.h"
#include "../umlobject.h"


#include <qpointarray.h>
#include <kdebug.h>


namespace Umbrello{

UMLWidget::UMLWidget( Diagram *diagram, uint id, UMLObject *object) :
	DiagramElement(diagram, id), m_umlObject(object)
{
	connect(object, SIGNAL(modified()),this, SLOT(umlObjectModified()));
	connect(object, SIGNAL(destroyed()),this, SLOT(deleteLater()));
}

UMLWidget::~UMLWidget()
{
	hide();
	canvas()->update();
}


QPointArray UMLWidget::areaPoints() const
{
    QPointArray pa(4);
    int pw = (pen().width()+1)/2;
    if ( pw < 1 ) pw = 1;
    if ( pen() == NoPen ) pw = 0;
    pa[0] = QPoint((int)x()-pw,(int)y()-pw);
    pa[1] = pa[0] + QPoint(m_width + pw*2, 0);
    pa[2] = pa[1] + QPoint(0, m_height + pw*2);
    pa[3] = pa[0] + QPoint(0, m_height + pw*2);
    return pa;
}

void UMLWidget::umlObjectModified()
{
	kdWarning()<<"UMLWidget::umlObjectModified() called! "<<endl
		<<"This method should be reimplemented in subclassed to update the widget"<<endl;
}

void UMLWidget::execDefaultAction()
{
	//call virtual editProperties
	kdDebug()<<"UMLWidget::execDefaultAction() called"<<endl;
	editProperties();
}

//FIXME
void UMLWidget::registerAssociation(AssociationWidget *a)
{
	m_associations.append(a);
}


QPoint UMLWidget::getConnectingPoint( AssociationWidget *a, const QPoint &from)
{
kdDebug()<<"UMLWidget::getConnectingPoint(QPoint &conn1, QPoint &conn2, const QPoint &from) not implemented"<<endl;
/*	//for now just allow the assoc. to connect in the direction its comming
	if(from.y() > y())
	{
		if(from.x()<x())
		{
		conn1.setX(x());
		conn1.setY(y() + height() / 2);
		conn2 = conn1 - QPoint(10,0);
		
		}
		else if(from.x()>x())
		{
		conn1.setX(x() + width());
		conn1.setY(y() + height() / 2);
		conn2 = conn1 + QPoint(10,0);
		}
		else
		{
		conn1.setX(x() + width() / 2);
		conn1.setY( y());
		conn2 = conn1 - QPoint(0,10);
		}
		
	}
	else
	{
	if(from.x()<x())
		{
		conn1.setX(x());
		conn1.setY(y() + height() / 2);
		conn2 = conn1 - QPoint(10,0);
		
		}
		else if(from.x()>x())
		{
		conn1.setX(x() + width());
		conn1.setY(y() + height() / 2);
		conn2 = conn1 + QPoint(10,0);
		}
		else
		{
		conn1.setX(x() + width() / 2);
		conn1.setY( y() + height());
		conn2 = conn1 + QPoint(0,10);
		}
		
	}*/
}

}

#include "umlwidget.moc"
