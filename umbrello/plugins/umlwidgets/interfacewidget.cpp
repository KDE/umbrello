  /***************************************************************************
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

#include "interfacewidget.h"
#include "../../uml.h"
#include "../../interface.h"
#include "../../dialogs/umbrellodialog.h"
#include "../../dialogs/interfacepropertiespage.h"
#include "../../dialogs/classifieroperationspage.h"
#include "../../dialogs/umlobjectassociationspage.h"
//#include "../../dialogs/classdisplayoptionspage.h"
#include "../../dialogs/widgetcolorspage.h"
#include "../../operation.h"
#include "../../attribute.h"

#include <qapplication.h>
#include <klocale.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qvbox.h>
#include <qptrlist.h>

#include <kdebug.h>


namespace Umbrello {

int InterfaceWidget::lineHeight = 1;

InterfaceWidget::InterfaceWidget(Diagram *diagram, uint id, UMLInterface *object):
	UMLWidget(diagram, id, object)
{
	m_nameDisplayOpts = ShowPackage | ShowStereotype;
	m_opsDisplayOpts  = ShowOps | ShowParameterList;

	calculateSize();
}

InterfaceWidget::~InterfaceWidget()
{
	hide();
	canvas()->update();
}

void InterfaceWidget::refactor( )
{
	UMLApp::app()->refactor(static_cast<UMLInterface*>(m_umlObject));
}

void InterfaceWidget::umlObjectModified()
{
	invalidate(); //canvas()->setChanged( boundingRec() );
	calculateSize();
	update();
	canvas()->update();
}

void InterfaceWidget::fillContextMenu(QPopupMenu &menu)
{
	UMLWidget::fillContextMenu(menu);
	menu.insertItem(i18n("Refactor"),this, SLOT(refactor()));
}

void InterfaceWidget::drawShape(QPainter &p)
{
	QPen textPen(Qt::black);
	QFont font;
	QPen drawPen = (m_useOwnPen ? pen() : diagram()->pen());
	QBrush drawBrush = ( m_useOwnBrush ? brush() : diagram()->brush() );

	int currentX,  currentY, limitX, limitY;
	int internalWidth;
	currentX = (int) x();
	currentY = (int) y();
	limitX = currentX + width() - ( 1 * hMargin );
	limitY = currentY + height() - ( 1 * vMargin );
	internalWidth = width() - ( 2 * hMargin );

	p.setPen(drawPen);
	p.setBrush(drawBrush);
	p.drawRect(currentX, currentY, width(), height());
	currentX +=hMargin;
	currentY +=vMargin;

	p.setPen(textPen);
	font.setBold(true);
	p.setFont(font);
	if( (m_nameDisplayOpts & ShowStereotype) && !(m_stereotype.isEmpty()))
	{
		p.drawText( currentX, currentY, internalWidth, lineHeight, Qt::AlignCenter,m_stereotype );
		currentY += lineHeight;
	}
	//package information is already in m_name
	p.drawText(currentX, currentY, internalWidth, lineHeight, Qt::AlignCenter,m_name);
	currentY += lineHeight;
	font.setBold(false);
	p.setFont(font);
	if( m_opsDisplayOpts & ShowOps )
	{
		p.setPen(drawPen);
		currentY += vMargin;
		p.drawLine((int)x(),currentY,(int) x() + width() -1, currentY);
		currentY += vMargin;
		p.setPen(textPen);

		QValueList<OpString>::Iterator end(m_ops.end());
		for(QValueList<OpString>::Iterator it(m_ops.begin()); it != end; ++it )
		{
			font.setUnderline( ((*it).flags & Underline) );
			font.setItalic( ((*it).flags & Italics) );
			p.setFont(font);
			p.drawText(currentX, currentY, internalWidth, lineHeight,Qt::AlignVCenter | Qt::AlignLeft, (*it).string );
			currentY += lineHeight;
		}
		//currentY += vMargin;
	}

	if(isSelected())
	{
	p.setPen(Qt::blue);
	p.setBrush(Qt::blue);
	QRect selectionRect(0,0,4,4);
	selectionRect.moveTopLeft(QPoint((int)x(),(int) y()));
			p.drawRect(selectionRect);
	selectionRect.moveTopRight(QPoint((int) x()+width(),(int) y()));
			p.drawRect(selectionRect);
	selectionRect.moveBottomRight(QPoint((int) x()+width(),(int) y()+height()));
			p.drawRect(selectionRect);
	selectionRect.moveBottomLeft(QPoint((int) x(),(int) y()+height()));
			p.drawRect(selectionRect);

	}
	if( isShowHotSpots( ) )
		drawHotSpots( p );

}

void InterfaceWidget::calculateSize()
{
	UMLInterface *obj = dynamic_cast<UMLInterface*>(m_umlObject);

	m_stereotype = "";
	m_name = "";
	m_ops.clear();
	if( (m_nameDisplayOpts & ShowStereotype) && !(obj->getStereotype().isEmpty()))
	{
		m_stereotype = "<<" + obj->getStereotype() + ">>";
	}
	if( m_nameDisplayOpts & ShowPackage )
	{
		m_name += obj->getPackage() + "::";
	}
	m_name += obj->getName();

	if (m_opsDisplayOpts & ShowOps )
	{

		QPtrList<UMLOperation> *ops = obj->getFilteredOperationsList(); //FIXME probably want getOpsList()
		UMLOperation *op;
		for( op=ops->first();op != 0;op=ops->next() )
		{
			OpString opString;
			//why is this needed??
			opString.string = "";
			opString.flags = EmptyFlag;
			switch(op->getScope( )) //FIXME Visibility!
			{
				case Uml::Public:
					opString.string = "+ ";
					break;
				case Uml::Protected:
					opString.string = "# ";
					break;
				case Uml::Private:
					opString.string = "- ";
					break;
			}
			opString.string += op->getName() + "( ";
			QPtrList<UMLAttribute> *params = op->getParmList();
			for( UMLAttribute *last = params->last(), *param = params->first();
				param &&  (m_opsDisplayOpts & ShowParameterList);
				param = params->next() )
			{
				opString.string += param->getName() + " : " + param->getTypeName();
				if(!(param->getInitialValue().isEmpty()))
				{
					opString.string += " = " + param->getInitialValue();
				}
				if( param != last )
				{
					opString.string += ", ";
				}
			}
			opString.string += " )";
			opString.string+=" : " + op->getReturnType();
			if( op->getAbstract() )
			{
				opString.flags = (TextFlags)(opString.flags|Italics);
			}
			if( op->getStatic() )
			{
				opString.flags = (TextFlags)(opString.flags|Underline);
			}
			m_ops.append(opString);
		}
	}

	QFont font;  // for now use application font

	int maxWidth = 0;
/*	int currentWidth = 0;  FIXME unused variable */

	// we calculate the screen space needed using bold, underline and italics
	// this may give a slightly larger area but it's easier than testing
	// each element
	font.setBold(true);
	font.setUnderline(true);
	font.setItalic(true);
	QFontMetrics fm(font);

	maxWidth = kMax(maxWidth,fm.width(m_stereotype));
	maxWidth = kMax(maxWidth, fm.width(m_name));

	QValueList<OpString>::Iterator end(m_ops.end());
	for(QValueList<OpString>::Iterator it(m_ops.begin()); it != end; ++it )
	{
		maxWidth = kMax(maxWidth,fm.width((*it).string));
	}

	uint width = maxWidth + (2 * hMargin);

	lineHeight = fm.lineSpacing();

	uint height = vMargin +
		( ((m_nameDisplayOpts & ShowStereotype) && (!m_stereotype.isEmpty()))? lineHeight : 0 )  + //stereotype
		lineHeight + //name
		vMargin +
		(( m_opsDisplayOpts & ShowOps ) ? 1 * vMargin : 0 ) +
		(m_ops.count() * lineHeight ) +
		(( (m_opsDisplayOpts & ShowOps) && (!m_ops.isEmpty())) ? 1 * vMargin : 0 );

	setSize(width,height);
}

void InterfaceWidget::editProperties()
{

	UmbrelloDialog dialog;

	dialog.addPage(new InterfacePropertiesPage( static_cast<UMLInterface*>(m_umlObject),0L),i18n("General"));
	dialog.addPage(new ClassifierOperationsPage( static_cast<UMLInterface*>(m_umlObject), diagram()->document(),0L),i18n("Operations"));
	dialog.addPage(new UMLObjectAssociationsPage( static_cast<UMLInterface*>(m_umlObject),0L),i18n("Associations"));
	//dialog.addPage(new InterfaceDisplayOptionsPage( this,0L), i18n("Display Options"));
	dialog.addPage(new WidgetColorsPage( this,  0L), i18n("Colors"));

	dialog.exec();

}

} //end of namespace Umbrello

#include "interfacewidget.moc"
