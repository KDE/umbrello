/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//qt includes
#include <qpointarray.h>
//kde includes
#include <kcursor.h>
#include <kcolordialog.h>
//app includes
#include "dialogs/notedialog.h"
#include "umlview.h"
#include "notewidget.h"

NoteWidget::NoteWidget(UMLView * view, UMLWidgetData* pData)  : UMLWidget(view, pData) {
	init();
}

NoteWidget::NoteWidget(UMLView * view, int id)  : UMLWidget(view, id, new NoteWidgetData(view->getOptionState() )) {
	init();
	setSize(100,80);
	m_pData->setType(wt_Note);
	((NoteWidgetData*)m_pData)->m_bLinkDocumentation = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NoteWidget::NoteWidget(UMLView * view) : UMLWidget(view, new NoteWidgetData(view->getOptionState() )) {
	init();
	setSize(100,80);
	m_pData->setType(wt_Note);
	((NoteWidgetData*)m_pData)->m_bLinkDocumentation = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NoteWidget::init() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NoteWidget::~NoteWidget() {

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NoteWidget::draw(QPainter & p, int offsetX, int offsetY) {
	int margin = 10;
	int w = width()-1;

	int h= height()-1;
	QPointArray poly(6);
	poly.setPoint(0, offsetX, offsetY);
	poly.setPoint(1, offsetX, offsetY + h);
	poly.setPoint(2, offsetX + w, offsetY + h);
	poly.setPoint(3, offsetX + w, offsetY + margin);
	poly.setPoint(4, offsetX + w - margin, offsetY);
	poly.setPoint(5, offsetX, offsetY);
	p.setPen(m_pData->getLineColour());
	if(m_pData->getUseFillColor()) {
		p.setBrush(m_pData->getFillColour());
		p.drawPolygon(poly);
	} else
		p.drawPolyline(poly);
	p.drawLine(offsetX + w - margin, offsetY, offsetX + w - margin, offsetY + margin);
	p.drawLine(offsetX + w - margin, offsetY + margin, offsetX + w, offsetY + margin);
	if(m_bSelected)
		drawSelected(&p, offsetX, offsetY);

	drawText( p, offsetX, offsetY );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoteWidget::mouseMoveEvent(QMouseEvent *me) {
	if(!m_bResizing) {
		UMLWidget::mouseMoveEvent(me);
		return;
	}
	if( !m_bMouseDown )
		return;
	int newW = m_nOldW + me->x()- m_nOldX - m_nPressOffsetX;
	int newH = m_nOldH + me->y()- m_nOldY - m_nPressOffsetY;
	newW = newW < 50?50:newW;
	newH = newH < 50?50:newH;
	setSize( newW, newH );
	adjustAssocs( (int)x(), (int)y() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NoteWidget::mousePressEvent(QMouseEvent *me) {
	UMLWidget::mousePressEvent(me);
	int w = width();
	int h = height();
	m_nOldW=w;
	m_nOldH=h;
	int m=6;
	//bottomRight
	if( (m_nOldX + m_nPressOffsetX ) >= ((int)x() + width() - m) && ( m_nOldY + m_nPressOffsetY ) >= ( (int)y() + height() - m) && me -> button() == LeftButton) {
		m_bResizing = true;
		m_pView -> setCursor(KCursor::sizeFDiagCursor());
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NoteWidget::slotMenuSelection(int sel) {
	NoteDialog * dlg = 0;
	switch(sel) {
		case ListPopupMenu::mt_Link_Docs:
			m_pView->updateNoteWidgets();
			break;

		case ListPopupMenu::mt_Rename:
			m_pView -> updateDocumentation( false );
			dlg = new NoteDialog( m_pView, this );
			if( dlg -> exec() ) {
				m_pView -> showDocumentation( this, true );
				update();
			}
			delete dlg;
			break;

		default:
			UMLWidget::slotMenuSelection(sel);
			break;
	}
}

bool NoteWidget::activate ( IDChangeLog* ChangeLog /*= 0*/ ) {
	bool status = UMLWidget::activate( ChangeLog );
	return status;
}

void NoteWidget::synchronizeData() {
	UMLWidget::synchronizeData();
}

void NoteWidget::mouseReleaseEvent( QMouseEvent * me ) {
	UMLWidget::mouseReleaseEvent( me );
	m_bResizing = false;
	m_pView -> setCursor( KCursor::arrowCursor() );
}

void NoteWidget::mouseDoubleClickEvent( QMouseEvent * me ) {
	if( me -> button() != LeftButton )
		return;
	slotMenuSelection( ListPopupMenu::mt_Rename );
}

void NoteWidget::drawText(QPainter & p, int offsetX, int offsetY) {
	/*
	Implement word wrap for text as follows:
	wrap at width on whole words.
	if word is wider than width then clip word
	if reach height exit and don't print anymore
	start new line on \n character
	*/
	p.setPen( black );
	QFont font = m_pData -> getFont();
	p.setFont( font );
	QFontMetrics fm = QFontMetrics( font );
	int fontHeight  = fm.lineSpacing();
	QString text = getDoc();
	if( text.length() == 0 )
		return;
	uint i = 0;
	QString word = "";
	int margin = fm.width( "W" );
	int textY = fontHeight / 2;
	int textX = margin;
	int width = this -> width() - margin * 2;
	int height = this -> height() - fontHeight;
	QChar returnChar('\n');
	while( i <= text.length() ) {
		QChar c = text[ i++ ];
		if( c == returnChar ) {
			if( word.length() > 0 ) {
				int textWidth = fm.width( word );
				if( ( textX + textWidth ) > width )//wrap word
				{
					textWidth = textWidth < width ? textWidth: width;
					textX = margin;
					textY += fontHeight;
					if( textY > height )
						return;
					p.drawText( offsetX + textX, offsetY + textY , textWidth, fontHeight, AlignLeft, word );
				}//end if
				else
					p.drawText( offsetX + textX, offsetY + textY , textWidth, fontHeight, AlignLeft, word );
				textX = textWidth;
			}//end if
			textY += fontHeight;
			word = "";
		} else if( c.isSpace() ) {
			if( word.length() > 0 ) {
				int textWidth = fm.width( word );
				if( ( textX + textWidth ) > width )//wrap word
				{
					textWidth = textWidth < width ? textWidth: width;
					if( textX != margin )
						textY += fontHeight;
					textX = margin;
					if( textY > height )
						return;
					p.drawText( offsetX + textX, offsetY + textY , textWidth, fontHeight, AlignLeft, word );
				}//end if
				else
					p.drawText( offsetX + textX, offsetY + textY , textWidth, fontHeight, AlignLeft, word );
				textX += textWidth;
			}//end if
			textX += fm.width( " " );
			word = "";
		} else {
			word += c;
		}
	}//end while
	if( word.length() > 0 ) {
		int textWidth = fm.width( word );
		if( ( textWidth + textX ) > width )//wrap word
		{
			textX = margin;
			textY += fontHeight;
			if( textY > height )
				return;
			p.drawText( offsetX + textX, offsetY + textY , textWidth, fontHeight, AlignLeft, word );
		}//end if
		else
			p.drawText( offsetX + textX, offsetY + textY , textWidth, fontHeight, AlignLeft, word );
	}//end if
}


