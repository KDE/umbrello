

#include "classwidget.h"
#include <qapplication.h>
#include <kdebug.h>
#include <qpainter.h>
#include "../dialogs/classpropdlg.h"
//#include "../dialogs/classpropertiespage.h"
//FIXME#include "classpropertiesbase.h"
#include "../concept.h"
#include "../attribute.h"
#include "../operation.h"
#include <qlist.h>

// unnamed namespace : data for this file only
namespace{
 enum {DisplayPackage = 0x1, DisplayStereotype = 0x2 };
 enum {ShowAtts = 0x1, ShowOps = 0x2, DisplayType = 0x4, DisplayVisibility = 0x8};
 
 
 int max(int a, int b) { return a>b?a:b; };
 
 //vertical and horizontal margins
 int vMargin = 10;
 int hMargin = 10;
 int lineHeight = 1;
}

	

namespace Umbrello{

ClassWidget::ClassWidget(Diagram *diagram, uint id, UMLClass *object):
	UMLWidget(diagram, id, object)
{
kdDebug()<<"creating class..."<<endl;
	m_nameDisplayOpts = 0;
	m_attsDisplayOpts |= ShowAtts | DisplayType;
	m_opsDisplayOpts |=ShowOps;
	
	calculateSize();
}	
	 
ClassWidget::~ClassWidget()
{
	hide();
	canvas()->update();//FIXME do the sam for all other widgets (update)
}	
	
void ClassWidget::umlObjectModified()
{
calculateSize();
update();
canvas()->update();

}

void ClassWidget::drawShape(QPainter &p)
{
//kdDebug()<<"drawing ClassWidget..."<<endl;
	QPen textPen(Qt::black);
	QFont font;
	//if(!useOwnPen !useOwnBrush)
	p.setPen(diagram()->pen());
	p.setBrush(diagram()->brush());
	int currentX,  currentY;
	currentX = x();
	currentY = y();
	
	p.drawRect(currentX, currentY, width(), height());
	currentX +=hMargin;
	currentY +=vMargin;

	p.setPen(textPen);
	font.setBold(true);
	p.setFont(font);
	p.drawText(currentX,currentY,width()-2*hMargin,lineHeight,Qt::AlignCenter,m_name);
	p.setPen(diagram()->pen());
	font.setBold(false);
	p.setFont(font);
	currentY +=lineHeight;
	if(!m_atts.isEmpty())
	{
		currentY += vMargin;
		p.drawLine(currentX - hMargin, currentY, currentX - hMargin + width(), currentY);
		currentY += vMargin;
		currentY += lineHeight;
	}
	
	for(QStringList::Iterator it = m_atts.begin(); it != m_atts.end(); ++it ) 
	{
		p.drawText(currentX,currentY,*it);
		currentY += lineHeight;
	}                              
	if(!m_atts.isEmpty())
	{
		currentY -= lineHeight;
	}
	
	if(!m_ops.isEmpty())
	{
		currentY += vMargin;
		p.drawLine(currentX - hMargin, currentY, currentX - hMargin + width(), currentY);
		currentY += vMargin;
		currentY += lineHeight;
	}
	
	for(QStringList::Iterator it = m_ops.begin(); it != m_ops.end(); ++it ) 
	{
		p.drawText(currentX,currentY,*it);
		currentY += lineHeight;
	}

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

void ClassWidget::calculateSize()
{
UMLClass *obj = dynamic_cast<UMLClass*>(m_umlObject);

//Cache texts	
//kdDebug()<<"calculating size.."<<endl;	
	m_name = "";
	m_atts.clear();
	m_ops.clear();
	
	if( m_nameDisplayOpts & DisplayPackage )
	{
		m_name += obj->getPackage() + "::";
	}
	m_name += obj->getName();
	
	if (m_attsDisplayOpts & ShowAtts )
	{
		QString str;
		QList<UMLAttribute> *atts = obj->getAttList();
		UMLAttribute *att;
		for(att=atts->first();att != 0;att=atts->next()) 
		{
			str = att->getName();
			if( m_attsDisplayOpts & DisplayType )
			{
				str+=" : " + att->getTypeName();
			}
			m_atts.append(str);
			str = "";
		
		}
	
	}
	
	if (m_opsDisplayOpts & ShowOps )
	{
		QString str;
		QList<UMLOperation> *ops = obj->getOpList();
		UMLOperation *op;
		for(op=ops->first();op != 0;op=ops->next()) 
		{
			str = op->getName();
			str += "( )";
			if( m_opsDisplayOpts & DisplayType )
			{
				str+=" : " + op->getReturnType();
			}
			m_ops.append(str);
			str = "";
		
		}
	
	}
	
	
	QFontMetrics fm(qApp->font());
	int maxWidth = 0;
	int currentWidth = 0;
	
	currentWidth = fm.width(m_name);
	maxWidth = max(maxWidth,currentWidth);
	
	
	for(QStringList::Iterator it = m_atts.begin(); it != m_atts.end(); ++it ) 
	{
		currentWidth = fm.width(*it);
		maxWidth = max(maxWidth,currentWidth);
	}
	
	for(QStringList::Iterator it = m_ops.begin(); it != m_ops.end(); ++it ) 
	{
		currentWidth = fm.width(*it);
		maxWidth = max(maxWidth,currentWidth);
	}
	
	m_width = maxWidth + (2*hMargin);
	
	lineHeight = fm.lineSpacing();
//	kdDebug()<<"lineHeight set to "<<lineHeight<<endl;
	
	m_height = 1*lineHeight + (2*vMargin) +
		   ((m_atts.isEmpty())? 0 : (2*vMargin) ) +
		   (m_atts.count() * lineHeight) + 
		   ((m_ops.isEmpty())?0:(2*vMargin) ) +
		   (m_ops.count() * lineHeight ) ;
		   
		   
		   
//	kdDebug()<<"width set to "<<width()<<" and height to "<<height()<<endl;
		   
	
}
	
void ClassWidget::editProperties()
{
kdDebug()<<"class widget properties"<<endl;
	/*ClassPropDlg *dlg = new ClassPropDlg(0L, m_umlObject, ClassPropDlg::page_gen, true);

	if(dlg->exec()) {
	}
	dlg -> close(true);//wipe from memory
	return;*/
	UMLClass *c = dynamic_cast<UMLClass*>(m_umlObject);
	QWidget *w = 0L;
	//ClassPropertiesBase *p = new ClassPropertiesBase( );
	//p->show();	

}
} //end of namespace NewDiagram

#include "classwidget.moc"
