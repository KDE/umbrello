



#include "associationwidget.h"
#include "../association.h"

#include <kdebug.h>



namespace Umbrello{

AssociationWidget::AssociationWidget(Diagram *diagram, uint id, UMLAssociation *assoc,
				 UMLWidget *start, UMLWidget *end):
		UMLWidget(diagram, id, assoc ),m_widgetA(start),m_widgetB(end)
{
	connect(m_widgetA,SIGNAL(moved()),this,SLOT(widgetMoved()));
	connect(m_widgetB,SIGNAL(moved()),this,SLOT(widgetMoved()));
	connect(m_widgetA,SIGNAL(destroyed()),this,SLOT(deleteLater()));
	connect(m_widgetB,SIGNAL(destroyed()),this,SLOT(deleteLater()));
	
	//FIXME!!!!
m_width = 10;
m_height = 10;
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
}
	
QPointArray AssociationWidget::areaPoints() const
{
//FIXME
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
	

void AssociationWidget::umlObjectModified()
{
kdDebug()<<"UMLAssociation was modifed. update!"<<endl;

}


void AssociationWidget::widgetMoved()
{
kdDebug()<<"widget moved, update!"<<endl;
}

void AssociationWidget::drawShape(QPainter &p)
{
kdDebug()<<"paint association!!"<<endl;
	
	/*p.drawLine(p1,p2);
	p.drawLine(p2,p3);
	p.drawLine(p3,p4);*/
}

}

#include "associationwidget.moc"
