 /*
  *  copyright (C) 2002-2004
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

// own header
#include "notewidget.h"
//qt includes
#include <qpointarray.h>
#include <qpainter.h>
#include <qtextedit.h>
#include <qframe.h>
// kde includes
#include <kdebug.h>
#include <kcursor.h>
#include <kcolordialog.h>
// app includes
#include "dialogs/notedialog.h"
#include "umldoc.h"
#include "umlview.h"
#include "uml.h"
#include "listpopupmenu.h"

#define NOTEMARGIN 10

NoteWidget::NoteWidget(UMLView * view, Uml::IDType id) : UMLWidget(view, id) {
	init();
	setSize(100,80);
	m_pEditor = new QTextEdit(view);
	m_pEditor->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	m_pEditor->setHScrollBarMode(QScrollView::AlwaysOff);
	m_pEditor->setVScrollBarMode(QScrollView::AlwaysOff);
	m_pEditor->setShown(true);
	setEditorGeometry();
	connect(m_pView, SIGNAL(contentsMoving(int, int)),
		this, SLOT(slotViewScrolled(int, int)));
}

void NoteWidget::init() {
	UMLWidget::setBaseType(Uml::wt_Note);
	m_bLinkDocumentation = false;
	m_pEditor = NULL;
}

NoteWidget::~NoteWidget() {
	delete m_pEditor;
}

void NoteWidget::slotViewScrolled(int x, int y) {
	setEditorGeometry(x, y);
}

void NoteWidget::setEditorGeometry(int dx /*=0*/, int dy /*=0*/) {
	const QRect editorGeometry( UMLWidget::getX() - dx + 6,
				    UMLWidget::getY() - dy + 10,
				    UMLWidget::getWidth() - 16,
				    UMLWidget::getHeight() - 16);
	m_pEditor->setGeometry( editorGeometry );
	m_pEditor->setShown(true);
}

void NoteWidget::setX( int x ) {
	UMLWidget::setX(x);
	setEditorGeometry();
}

void NoteWidget::setY( int y ) {
	UMLWidget::setY(y);
	setEditorGeometry();
}

QString NoteWidget::getDoc() const {
	return m_pEditor->text();
}

void NoteWidget::setDoc(const QString &newText) {
	m_pEditor->setText(newText);
}

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
	UMLWidget::draw(p, offsetX, offsetY);
	if ( UMLWidget::getUseFillColour() ) {
		QBrush brush( UMLWidget::getFillColour() );
		p.setBrush(brush);
		p.drawPolygon(poly);
		m_pEditor->setPaper(brush);
	} else
		p.drawPolyline(poly);
	p.drawLine(offsetX + w - margin, offsetY, offsetX + w - margin, offsetY + margin);
	p.drawLine(offsetX + w - margin, offsetY + margin, offsetX + w, offsetY + margin);
	if(m_bSelected) {
		drawSelected(&p, offsetX, offsetY,  true);
	}

	drawText( p, offsetX, offsetY );
}

void NoteWidget::mouseMoveEvent(QMouseEvent *me) {
	if(!m_bResizing) {
		UMLWidget::mouseMoveEvent(me);
		return;
	}
	if( !m_bMouseDown )
		return;
	int newX = me->x();
	int newY = me->y();
	if (! m_bIgnoreSnapToGrid) {
		newX = m_pView->snappedX( newX );
		newY = m_pView->snappedY( newY );
	}
	int newW = m_nOldW + newX - m_nOldX - m_nPressOffsetX;
	int newH = m_nOldH + newY - m_nOldY - m_nPressOffsetY;
	newW = newW < 50?50:newW;
	newH = newH < 50?50:newH;
	setSize( newW, newH );
	setEditorGeometry();
	adjustAssocs( getX(), getY() );
}

void NoteWidget::mousePressEvent(QMouseEvent *me) {
	UMLWidget::mousePressEvent(me);
	m_nOldW = getWidth();
	m_nOldH = getHeight();
	int m = 10;
	//bottomRight
	if( m_nOldX + m_nPressOffsetX >= getX() + m_nOldW - m &&
	    m_nOldY + m_nPressOffsetY >= getY() + m_nOldH - m &&
	    me -> button() == LeftButton) {
		m_bResizing = true;
		m_pView -> setCursor(KCursor::sizeFDiagCursor());
	}
}

void NoteWidget::slotMenuSelection(int sel) {
	NoteDialog * dlg = 0;
	UMLDoc *doc = UMLApp::app()->getDocument();
	switch(sel) {
		case ListPopupMenu::mt_Link_Docs:
			m_pView->updateNoteWidgets();
			doc -> setModified(true);
			break;

		case ListPopupMenu::mt_Rename:
			m_pView -> updateDocumentation( false );
			dlg = new NoteDialog( m_pView, this );
			if( dlg -> exec() ) {
				m_pView -> showDocumentation( this, true );
				doc -> setModified(true);
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

void NoteWidget::mouseReleaseEvent( QMouseEvent * me ) {
	UMLWidget::mouseReleaseEvent( me );
	if ( m_bResizing ) {
		m_bResizing = false;
		m_pView -> setCursor( KCursor::arrowCursor() );
		UMLApp::app()->getDocument()->setModified(true);
	}
}

void NoteWidget::mouseDoubleClickEvent( QMouseEvent * me ) {
	if( me -> button() != LeftButton )
		return;
	slotMenuSelection( ListPopupMenu::mt_Rename );
}

void NoteWidget::drawText(QPainter &, int, int) {
	m_pEditor->setText( getDoc() );
	m_pEditor->setShown(true);
}

void NoteWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement noteElement = qDoc.createElement( "notewidget" );
	UMLWidget::saveToXMI( qDoc, noteElement );
	noteElement.setAttribute( "text", m_pEditor->text() );
	qElement.appendChild( noteElement );
}

bool NoteWidget::loadFromXMI( QDomElement & qElement ) {
	if( !UMLWidget::loadFromXMI( qElement ) )
		return false;
	m_pEditor->setText( qElement.attribute("text", ""));
	return true;
}


#include "notewidget.moc"

