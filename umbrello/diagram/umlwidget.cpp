

#include "umlwidget.h"
#include <qpointarray.h>
#include <kdebug.h>
#include "../dialogs/classpropdlg.h"

#include "../umlobject.h"
//FIXME#include "associationwidget.h"

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
//update
//FIXME
//first update ourselves (width and height)
//then make the canvas make us repaint ourselves in the next update
	//canvas()->setChanged(this->boundingRect());
	//changeChunks();
kdDebug()<<"uml object modified!, update me!!!"<<endl;
}


//FIXME
void UMLWidget::registerAssociation(AssociationWidget *a)
{
	//m_associations.append(a);
}

/*void UMLWidget::editProperties( )
{
kdDebug()<<"widget propeties"<<endl;
	ClassPropDlg *dlg = new ClassPropDlg(0L, m_umlObject, ClassPropDlg::page_gen, true);

	if(dlg->exec()) {
	}
	dlg -> close(true);//wipe from memory
	return;


}*/
void UMLWidget::getConnectingPoint(QPoint &conn1, QPoint &conn2, const QPoint &from)
{
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
