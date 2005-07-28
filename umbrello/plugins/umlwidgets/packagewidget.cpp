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

#include "packagewidget.h"
#include "../../uml.h"
#include "../../package.h"
#include "../../dialogs/umbrellodialog.h"
#include "../../dialogs/packagepropertiespage.h"
//#include "../../dialogs/umlobjectassociationspage.h"
//#include "../../dialogs/classdisplayoptionspage.h"
#include "../../dialogs/widgetcolorspage.h"

#include <qapplication.h>
#include <klocale.h>
#include <qpainter.h>
#include <q3popupmenu.h>
#include <q3vbox.h>
#include <q3ptrlist.h>

#include <kdebug.h>


namespace Umbrello {

int PackageWidget::lineHeight = 1;

PackageWidget::PackageWidget(Diagram *diagram, uint id, UMLPackage *object):
	UMLWidget(diagram, id, object)
{
	m_nameDisplayOpts = ShowPackage | ShowStereotype;
	calculateSize();
}

PackageWidget::~PackageWidget()
{
	hide();
	canvas()->update();
}


void PackageWidget::umlObjectModified()
{
	invalidate(); //canvas()->setChanged( boundingRec() );
	calculateSize();
	update();
	canvas()->update();
}

void PackageWidget::fillContextMenu(Q3PopupMenu &menu)
{
	UMLWidget::fillContextMenu(menu);
}

void PackageWidget::drawShape(QPainter &p)
{
	QPen textPen(Qt::black);
	QFont font;
	QPen drawPen = (m_useOwnPen ? pen() : diagram()->pen());
	QBrush drawBrush = ( m_useOwnBrush ? brush() : diagram()->brush() );

	int currentX,  currentY;
	currentX = (int) x();
	currentY = (int) y();



	p.setPen(drawPen);
	p.setBrush(drawBrush);
	p.drawRect(currentX, currentY, width() / 3 , tabHeight);
	currentY += tabHeight;
	p.drawRect(currentX, currentY, width() , height() - tabHeight );
	currentY += vMargin;

	p.setPen(textPen);
	font.setBold(true);
	p.setFont(font);
	if( (m_nameDisplayOpts & ShowStereotype) && !(m_stereotype.isEmpty()))
	{
		p.drawText( currentX, currentY, width(), lineHeight, Qt::AlignCenter,m_stereotype );
		currentY += lineHeight;
	}

	p.drawText(currentX, currentY, width(), lineHeight, Qt::AlignCenter,m_name);

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

void PackageWidget::calculateSize()
{
	UMLPackage *obj = dynamic_cast<UMLPackage*>(m_umlObject);

	m_stereotype = "";
	m_name = "";
	if( (m_nameDisplayOpts & ShowStereotype) && !(obj->getStereotype().isEmpty()))
	{
		m_stereotype = "<<" + obj->getStereotype() + ">>";
	}
	if( m_nameDisplayOpts & ShowPackage )
	{
		m_name += obj->getPackage() + "::";
	}
	m_name += obj->getName();

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

	uint width = maxWidth + (2 * hMargin);

	lineHeight = fm.lineSpacing();

	uint height = ( 2*vMargin) + tabHeight +
		( ((m_nameDisplayOpts & ShowStereotype) && (!m_stereotype.isEmpty()))? lineHeight : 0 )  + //stereotype
		lineHeight; //name

	setSize(width,height);
}

void PackageWidget::editProperties()
{

	UmbrelloDialog dialog;

	dialog.addPage(new PackagePropertiesPage( static_cast<UMLPackage*>(m_umlObject),0L),i18n("General"));
	/* Packages cannot have associations:
	dialog.addPage(new UMLObjectAssociationsPage( static_cast<UMLPackage*>(m_umlObject),0L),i18n("Associations"));
	 */
	//dialog.addPage(new PackageDisplayOptionsPage( this,0L), i18n("Display Options"));
	dialog.addPage(new WidgetColorsPage( this,  0L), i18n("Colors"));

	dialog.exec();
}

} //end of namespace Umbrello

#include "packagewidget.moc"
