 /*
  *  copyright (C) 2003-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "umlassociationwidget.h"
#include "../../diagram/diagram.h"
#include "../../association.h"


#include <qwmatrix.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qcolor.h>
#include <qpointarray.h>

#include <math.h>

#include <kdebug.h>

namespace {
	const double PI = 3.141516;
}
namespace Umbrello{
class AssociationEnd
{
public:
	enum Side { Start, End };
	AssociationEnd( PathSegment *segment, Side side );
	~AssociationEnd();

	void move( const QPoint &);
	void update( );
	void adjustPosition( );
	void setPoints( const QPointArray &a );
private:
	QPointArray m_points;
	PathSegment *m_segment;
	Side m_side;
	QCanvasPolygon *m_shape;

};





UMLAssociationWidget::UMLAssociationWidget( Diagram *diagram, uint id,
                      DiagramWidget *start, DiagramWidget *end,
					    UMLAssociation *association ) : AssociationWidget(diagram,id,start,end),m_association(association) {
	m_head = 0;
	m_tail = 0;

	createHead();
	createTail();
}


UMLAssociationWidget::~UMLAssociationWidget()
{
	delete m_head;
	delete m_tail;
	hide();
	canvas()->update();
}

void UMLAssociationWidget::moveBy( int dx, int dy)
{
	AssociationWidget::moveBy(dx,dy);
	m_head->adjustPosition( );
	m_tail->adjustPosition( );
}

void UMLAssociationWidget::setPathPoints( const QPointArray &a )
{
	AssociationWidget::setPathPoints( a );
	createHead();
	createTail();
}

void UMLAssociationWidget::moveHotSpotBy( int h, int dx, int dy )
{
	AssociationWidget::moveHotSpotBy( h, dx, dy );
	m_head->adjustPosition( );
	m_tail->adjustPosition( );
}

void UMLAssociationWidget::widgetMoved( )
{
	AssociationWidget::widgetMoved( );
	m_head->adjustPosition( );
	m_tail->adjustPosition( );
}


void UMLAssociationWidget::fillContextMenu(QPopupMenu &menu)
{
	AssociationWidget::fillContextMenu(menu);
}

void UMLAssociationWidget::umlObjectModified( )
{
	kdDebug()<<" update association widget "<<endl;
}

void UMLAssociationWidget::createHead( )
{
//create the shape for the association's head -
// you should "draw" the shape pointing up, with the top at ( 0, 0 ), the shape will be
// rotated and translated automatically to fit the path segment it's attached to
	if(m_head)
		delete m_head;
	m_head = new AssociationEnd( m_segments.first(), AssociationEnd::Start );
	QPointArray a;
	switch( m_association->getAssocType() )
	{
		case Uml::at_Generalization:
		break;

		default: break;
	}

	m_head->setPoints(a);
	m_head->adjustPosition( );
}

void UMLAssociationWidget::createTail( )
{
//create the shape for the association's tail -
// you should "draw" the shape pointing up, with the top at ( 0, 0 ), the shape will be
// rotated and translated automatically to fit the path segment it's attached to
	if(m_tail)
		delete m_tail;
	m_tail = new AssociationEnd( m_segments.last(), AssociationEnd::End );
	QPointArray a;
	switch( m_association->getAssocType() )
	{
		case Uml::at_Generalization:
			a.resize(3);
			a[0] = QPoint(0,0);
			a[1] = QPoint(-5,10);
			a[2] = QPoint(5,10);
		break;
		case Uml::at_Composition:
			a.resize(4);
			a[0] = QPoint(0,0);
			a[1] = QPoint(-7,7);
			a[2] = QPoint(0,14);
			a[3] = QPoint(7,7);
		break;
		case Uml::at_Aggregation:
		case Uml::at_Dependency:
		case Uml::at_Association:
		case Uml::at_Association_Self:
		case Uml::at_Coll_Message:
		case Uml::at_Seq_Message:
		case Uml::at_Coll_Message_Self:
		case Uml::at_Seq_Message_Self:
		case Uml::at_Implementation:
		case Uml::at_Realization:
		case Uml::at_UniAssociation:
		case Uml::at_Anchor:
		case Uml::at_State:
		case Uml::at_Activity:
		case Uml::at_Unknown:
		default:
			kdWarning() << "incorrect or unknown association type" << endl;
			break;
	}
	m_tail->setPoints(a);
	m_tail->adjustPosition( );
}

void UMLAssociationWidget::drawShape(QPainter &p )
{
	AssociationWidget::drawShape(p);
	m_head->update( );
	m_tail->update( );
}


AssociationEnd::AssociationEnd( PathSegment *segment, Side side ):
                m_segment(segment), m_side(side)
{
	m_shape = new QCanvasPolygon( m_segment->canvas() );
	m_shape->setBrush(QBrush(QColor(Qt::blue)));
	kdDebug()<<"segment's z = "<<m_segment->z()<<endl;
	m_shape->setZ( 1000 ); //FIXME
	kdDebug()<<"shape's z = "<<m_shape->z()<<endl;
	m_shape->show();
}

AssociationEnd::~AssociationEnd()
{
	delete m_shape;
}

void AssociationEnd::adjustPosition( )
{
	QPoint p;
	QWMatrix m;
	if( m_side == Start )
	{
		p = m_segment->startPoint();
	}
	else
	{
		p = m_segment->endPoint();
	}
	//m_shape->invalidate();
	//m_shape->update();
	m_shape->hide();
	m_shape->move( p.x(), p.y() );

	double dx = m_segment->endPoint().x() - m_segment->startPoint().x();
	double dy = m_segment->endPoint().y() - m_segment->startPoint().y();
	if( dy == 0 )
		dy = 0.0000000000000000001;
	double degree =  -1 * ( atan( dx/dy ) * 180.0 / PI ) + 180;
	//kdDebug()<<"dx = "<<dx<<" dy = "<<dy<<"degree(1) = "<<degree<<endl;
	if( (m_side == Start && dy > 0) || (m_side == End && dy < 0) )
		degree += 180;
	//kdDebug()<<" degree(2) = "<<degree<<endl<<endl;
	m.rotate( degree );
	m_shape->setPoints( m.map(m_points) );
	m_shape->show();
	m_segment->canvas()->update();
}

void AssociationEnd::setPoints( const QPointArray &a )
{
	m_points = a;
	adjustPosition( );
}

void AssociationEnd::update( )
{
//	m_shape->update( );
}


} //end of namespace Umbrello

#include "umlassociationwidget.moc"
