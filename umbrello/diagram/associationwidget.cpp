



#include "associationwidget.h"
#include "../association.h"

#include <qpainter.h>
#include <qrect.h>
#include <kdebug.h>



namespace Umbrello{

AssociationWidget::AssociationWidget(Diagram *diagram, uint id, UMLAssociation *assoc,
				 UMLWidget *start, UMLWidget *end):
		UMLWidget(diagram, id, assoc ),m_widgetA(start),m_widgetB(end), m_path(2)
{
	connect(m_widgetA,SIGNAL(moved()),this,SLOT(widgetMoved()));
	connect(m_widgetB,SIGNAL(moved()),this,SLOT(widgetMoved()));
	connect(m_widgetA,SIGNAL(destroyed()),this,SLOT(deleteLater()));
	connect(m_widgetB,SIGNAL(destroyed()),this,SLOT(deleteLater()));
	
//FIXME  for now justr connect to the center of the widget. we should call classWidget->getconnectingpoint to get
// an appropiate connecting point from the classwidget, depending on where we are comming from	
	QPoint startPoint = QPoint(m_widgetA->x(), m_widgetA->y()) + QPoint(m_widgetA->width() / 2, m_widgetA->height() / 2 );
	QPoint endPoint = QPoint(m_widgetB->x(),m_widgetB->y()) + QPoint(m_widgetB->width() / 2, m_widgetB->height() / 2 );
		
	moveAbs( startPoint.x(), startPoint.y() );
	
	m_path[0] = startPoint;
	m_path[1] = endPoint;
	
	//FIXME!!!!
//m_width = 10;
//m_height = 10;
/*	start->registerAssociation(this);
	end->registerAssociation(this);
	QPoint aproxEnd = end->pos();
	QPoint aproxStart = start->pos();
	
	QPoint p1, p2;
	start->getConnectingPoint(p1,p2,aproxEnd);
	m_path.append(p1);
	m_path.append(p2);
	end->getConnectingPoint(p1,p2,aproxStart);
	m_path.append(p2);
	m_path.append(p1);*/
}
AssociationWidget::~AssociationWidget()
{
	hide();
	canvas()->update();
}

void AssociationWidget::setPath( QPointArray path )
{
	QPoint start = m_path[0];
	QPoint end = m_path[ m_path.size() -1 ];
	
	//m_path.clear();
	m_path.resize( path.size() + 2 );
	m_path[0] = start;
	for(int i = 0; i < path.size() ; i++)
	{
		m_path[i+1] = path[i];
	}
	
	m_path[ m_path.count() -1 ] = end;
}
	
QPointArray AssociationWidget::areaPoints() const
{

//Since calculating the areaPoints for a multi-segment path is not so easy, we 
// just calculate the bounding "polygon" for now (in the worst case == bounding rectangle)
// and add a margin of 10 to be on the safe side.
// Implementing this the right way would mean calculating the area points for each segment (easy)
// and then t
QPoint topLeft( m_path[0] ), topRight( m_path[0] ), bottomRight( m_path[0] ), bottomLeft( m_path[0] );
for( int i = 0; i< m_path.count() ; i++)
{
	if( m_path[i].x() < topLeft.x() && m_path[i].y() < topLeft.y() )
		topLeft = m_path[i];
	if( m_path[i].x() > topRight.x() && m_path[i].y() < topRight.y() )
		topRight = m_path[i];
	if( m_path[i].x() > bottomRight.x() && m_path[i].y() > bottomRight.y() )
		bottomRight = m_path[i];
	if( m_path[i].x() < bottomLeft.x() && m_path[i].y() > bottomLeft.y() )
		bottomLeft = m_path[i];
}
// //FIXME
    QPointArray pa(4);   
    pa[0] = topLeft + QPoint( -10, -10 );
    pa[1] = topRight + QPoint( 10, -10 );
    pa[2] = bottomRight + QPoint( 10, 10 );
    pa[3] = bottomLeft + QPoint( 10, -10 );
    ////////////////////////////////////////////////7
//     kdDebug()<<"area points : "<<endl;
//    for( int i = 0; i< m_path.count() ; i++)
//    	kdDebug()<<"m_path["<<i<<"] = "<<m_path[i].x()<<","<<m_path[i].y()<<endl;
///////////////////////////////
    return pa;

}
	

void AssociationWidget::umlObjectModified()
{
kdDebug()<<"UMLAssociation was modifed. update!"<<endl;

}


void AssociationWidget::widgetMoved()
{
//FIXME we connect now to the widget's center. we just call widget::getConnectingPoint() here, and have the widget assign us a new point
	const QObject *sender = QObject::sender();
	if( sender == m_widgetA )
	{
		m_path[0] = QPoint(m_widgetA->x(), m_widgetA->y()) + QPoint(m_widgetA->width() / 2, m_widgetA->height() / 2 );
	}
	else if( sender == m_widgetB )
	{
		m_path[ m_path.count() -1 ] = QPoint(m_widgetB->x(), m_widgetB->y()) + QPoint(m_widgetB->width() / 2, m_widgetB->height() / 2 );
	}
	update();
	canvas()->update();
}

void AssociationWidget::drawShape(QPainter &p)
{
	int pointCount = m_path.count();
	for( int i = 0; i < pointCount -1 ; i++ )
	{
		p.drawLine( m_path[i], m_path[i+1]);
	}
	if(isSelected())
	{
		p.setPen(Qt::blue);
		p.setBrush(Qt::blue);
		QRect selectionRect(0,0,4,4);
		for( int i = 0; i< m_path.count() ; i++)
		{
			selectionRect.moveCenter( m_path[i] );
			p.drawRect(selectionRect);
		}
	}
}

}

#include "associationwidget.moc"
