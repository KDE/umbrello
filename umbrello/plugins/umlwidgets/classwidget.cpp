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

#include "classwidget.h"
#include "../../uml.h"
#include "../../class.h"
#include "../../dialogs/umbrellodialog.h"
#include "../../dialogs/classpropertiespage.h"
#include "../../dialogs/classattributespage.h"
#include "../../dialogs/classifieroperationspage.h"
#include "../../dialogs/umlobjectassociationspage.h"
#include "../../dialogs/classtemplatespage.h"
#include "dialogs/classdisplayoptionspage.h"
#include "../../dialogs/widgetcolorspage.h"
#include "../../attribute.h"
#include "../../operation.h"

#include <qapplication.h>
#include <klocale.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qvbox.h>
#include <qptrlist.h>

#include <kdebug.h>


namespace Umbrello {

int ClassWidget::lineHeight = 1;

ClassWidget::ClassWidget(Diagram *diagram, uint id, UMLClass *object):
	UMLWidget(diagram, id, object)
{
	m_nameDisplayOpts = ShowPackage | ShowStereotype;
	m_attsDisplayOpts = ShowAtts | ShowType;
	m_opsDisplayOpts  = ShowOps | ShowParameterList;

	calculateSize();
}

ClassWidget::~ClassWidget()
{
	hide();
	canvas()->update();
}

void ClassWidget::refactor( )
{
	UMLApp::app()->refactor(static_cast<UMLClass*>(m_umlObject));
}

void ClassWidget::umlObjectModified()
{
	invalidate(); //canvas()->setChanged( boundingRec() );
	calculateSize();
	update();
	canvas()->update();
}

void ClassWidget::fillContextMenu(QPopupMenu &menu)
{
	UMLWidget::fillContextMenu(menu);
	menu.insertItem(i18n("Refactor"),this, SLOT(refactor()));
}

void ClassWidget::drawShape(QPainter &p)
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
	if( m_attsDisplayOpts & ShowAtts )
	{
		p.setPen(drawPen);
		currentY += vMargin;
		p.drawLine( (int)x(),currentY, (int)x() + width() -1, currentY );
		if( !m_atts.isEmpty() )
		{
			currentY += vMargin;
		}
		p.setPen(textPen);

		QValueList<AttString>::Iterator atts_end(m_atts.end());
		for(QValueList<AttString>::Iterator it = m_atts.begin(); it != atts_end; ++it )
		{
			font.setUnderline( ((*it).flags & Underline) );
			p.setFont(font);
			p.drawText(currentX, currentY, internalWidth, lineHeight, Qt::AlignVCenter | Qt::AlignLeft, (*it).string );
			currentY += lineHeight;
		}
		//currentY += vMargin;
	}
	if( m_opsDisplayOpts & ShowOps )
	{
		p.setPen(drawPen);
		currentY += vMargin;
		p.drawLine((int)x(),currentY,(int) x() + width() -1, currentY);
		currentY += vMargin;
		p.setPen(textPen);

		QValueList<OpString>::Iterator ops_end(m_ops.end());
		for(QValueList<OpString>::Iterator it = m_ops.begin(); it != ops_end; ++it )
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

void ClassWidget::calculateSize()
{
	UMLClass *obj = dynamic_cast<UMLClass*>(m_umlObject);

	m_stereotype = "";
	m_name = "";
	m_atts.clear();
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

	if (m_attsDisplayOpts & ShowAtts )
	{
		QPtrList<UMLAttribute> *atts = obj->getFilteredAttributeList();
		UMLAttribute *att;
		for( att=atts->first(); att != 0 ;att=atts->next() )
		{
			AttString attString;
			//why??
			attString.string = "";
			attString.flags = EmptyFlag;
			switch(att->getScope( )) //FIXME Visibility!
			{
				case Uml::Public:
					attString.string = "+ ";
					break;
				case Uml::Protected:
					attString.string = "# ";
					break;
				case Uml::Private:
					attString.string = "- ";
					break;
			}
			attString.string += att->getName();
			if( m_attsDisplayOpts & ShowType )
			{
				attString.string+= " : " + att->getTypeName();
			}
			if( m_attsDisplayOpts & ShowInitialValue )
			{
				attString.string+= " = " + att->getInitialValue();
			}
			if( att->getStatic() )
			{
				attString.flags = Underline;
			}
			m_atts.append(attString);
		}
	}
	if (m_opsDisplayOpts & ShowOps )
	{

		QPtrList<UMLOperation> *ops = obj->getFilteredOperationsList();//FIXME probably want getOpsList()
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

	// we calculate the screen space needed using bold, underline and italics
	// this may give a slightly larger area but it's easier than testing
	// each element
	font.setBold(true);
	font.setUnderline(true);
	font.setItalic(true);
	QFontMetrics fm(font);

	maxWidth = kMax(maxWidth,fm.width(m_stereotype));
	maxWidth = kMax(maxWidth, fm.width(m_name));

	QValueList<AttString>::Iterator atts_end(m_atts.end());
	for(QValueList<AttString>::Iterator it(m_atts.begin()); it != atts_end; ++it )
	{
		maxWidth = kMax(maxWidth,fm.width((*it).string));
	}

	QValueList<OpString>::Iterator ops_end(m_ops.end());
	for(QValueList<OpString>::Iterator it(m_ops.begin()); it != ops_end; ++it )
	{
		maxWidth = kMax(maxWidth,fm.width((*it).string));
	}

	uint width = maxWidth + (2 * hMargin);

	lineHeight = fm.lineSpacing();

	uint height = vMargin +
		( ((m_nameDisplayOpts & ShowStereotype) && (!m_stereotype.isEmpty()))? lineHeight : 0 )  + //stereotype
		lineHeight + //name
		vMargin +
		(( m_attsDisplayOpts & ShowAtts ) ? 1 * vMargin : 0 ) +
		(m_atts.count() * lineHeight) +
		(( (m_attsDisplayOpts & ShowAtts) && (!m_atts.isEmpty())) ? 1 * vMargin : 0 ) +
		(( m_opsDisplayOpts & ShowOps ) ? 1 * vMargin : 0 ) +
		(m_ops.count() * lineHeight ) +
		(( (m_opsDisplayOpts & ShowOps) && (!m_ops.isEmpty())) ? 1 * vMargin : 0 );

	setSize(width,height);

}

void ClassWidget::editProperties()
{

	UmbrelloDialog dialog;

	dialog.addPage(new ClassPropertiesPage( static_cast<UMLClass*>(m_umlObject),0L),i18n("General"));
	dialog.addPage(new ClassAttributesPage( static_cast<UMLClass*>(m_umlObject), diagram()->document(),0L),i18n("Attributes"));
	dialog.addPage(new ClassifierOperationsPage( static_cast<UMLClass*>(m_umlObject), diagram()->document(),0L),i18n("Operations"));
	dialog.addPage(new UMLObjectAssociationsPage( static_cast<UMLClass*>(m_umlObject),0L),i18n("Associations"));
	dialog.addPage(new ClassTemplatesPage( static_cast<UMLClass*>(m_umlObject), diagram()->document(),0L),i18n("Templates"));
	dialog.addPage(new ClassDisplayOptionsPage( this,0L), i18n("Display Options"));
	dialog.addPage(new WidgetColorsPage( this,  0L), i18n("Colors"));

	dialog.exec();
}

void ClassWidget::setNameDisplayOptions( int o )
{
	m_nameDisplayOpts = o;
	invalidate(); //canvas()->setChanged( boundingRec() );
	calculateSize();
	update();
	canvas()->update();
}
void ClassWidget::setAttsDisplayOptions( int o )
{
	m_attsDisplayOpts = o;
	invalidate(); //canvas()->setChanged( boundingRec() );
	calculateSize();
	update();
	canvas()->update();
}
void ClassWidget::setOpsDisplayOptions( int o )
{
	m_opsDisplayOpts = o;
	invalidate(); //canvas()->setChanged( boundingRec() );
	calculateSize();
	update();
	canvas()->update();
}


} //end of namespace Umbrello

#include "classwidget.moc"
