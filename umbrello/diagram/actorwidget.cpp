

#include "actorwidget.h"
#include <qapplication.h>
#include <kdebug.h>
#include <qpainter.h>

#include "../actor.h"
#include <qlist.h>
#include <kdebug.h>

// unnamed namespace : data for this file only
namespace{
 
 int max(int a, int b) { return a>b?a:b; };
 
 //vertical and horizontal margins
 int vMargin = 10;
 int hMargin = 10;
 int lineHeight = 1;
 int actorHeight = 120;
 int actorWidth = 40;
}

	

namespace Umbrello{

ActorWidget::ActorWidget(Diagram *diagram, uint id, UMLActor *object):
	UMLWidget(diagram, id, object)
{
kdDebug()<<"creating actor..."<<endl;
	
	calculateSize();
}	
	 
ActorWidget::~ActorWidget()
{
	hide();
}	
	
void ActorWidget::umlObjectModified()
{
calculateSize();
update();
canvas()->update();
}

void ActorWidget::drawShape(QPainter &p)
{

	QPen textPen(Qt::black);
	QFont font;
	//if(!useOwnPen !useOwnBrush)
	p.setPen(diagram()->pen());
	p.setBrush(diagram()->brush());
	int currentX,  currentY, hCenter;
	currentX = x();
	currentY = y();
	hCenter = currentX + m_width/2;
	kdDebug()<<"drawing ellipse currentY = "<<currentY<<endl;
	p.drawEllipse(hCenter - actorWidth/2, currentY + vMargin, actorWidth, (actorHeight*100)/300);
	currentY += (actorHeight*100)/300;
	kdDebug()<<"drawing body currentY = "<<currentY<<endl;
	p.drawLine(hCenter, currentY, hCenter , (currentY + ((actorHeight*100)/300)));
	currentY += (actorHeight*100)/300;
	kdDebug()<<"drawing legscurrentY = "<<currentY<<endl;
	p.drawLine(hCenter, currentY, hCenter - actorWidth/2, currentY + (actorHeight*100)/300 );
	p.drawLine(hCenter, currentY, hCenter + actorWidth/2, currentY + (actorHeight*100)/300 );
	
	currentY += (actorHeight*100)/300 ;
	kdDebug()<<"drawing textcurrentY = "<<currentY<<endl;

	p.setPen(textPen);
	p.drawText(x(),currentY, m_width, lineHeight, AlignCenter, m_name);
	

	
	if(isSelected())
	{
	p.setPen(Qt::blue);
	p.setBrush(Qt::blue);
	QRect selectionRect(0,0,4,4);
	selectionRect.moveTopLeft(QPoint(x(),y()));
			p.drawRect(selectionRect);
	selectionRect.moveTopRight(QPoint(x()+width(),y()));
			p.drawRect(selectionRect);
	selectionRect.moveBottomRight(QPoint(x()+width(),y()+height()));
			p.drawRect(selectionRect);
	selectionRect.moveBottomLeft(QPoint(x(),y()+height()));
			p.drawRect(selectionRect);

	}
	                                    
}

void ActorWidget::calculateSize()
{
UMLActor *obj = dynamic_cast<UMLActor*>(m_umlObject);

m_name = obj->getName();

QFontMetrics fm(qApp->font());
int maxWidth = 0;
int currentWidth = 0;
	
currentWidth = fm.width(m_name);
maxWidth = max(maxWidth,currentWidth);	
maxWidth = max(maxWidth, actorWidth);

m_width = maxWidth + (2*hMargin);
	
lineHeight = fm.lineSpacing();

	
m_height = 1*lineHeight + (2*vMargin) + actorHeight;

}
	

	

} //end of namespace NewDiagram

#include "actorwidget.moc"
