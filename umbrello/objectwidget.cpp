/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qpainter.h>

#include <klineeditdlg.h>
#include <klocale.h>
#include <kdebug.h>

#include "seqlinewidget.h"
#include "umlview.h"
#include "umlobject.h"
#include "objectwidget.h"
#include "listpopupmenu.h"

/**
 * The number of pixels margin between the lowest message
 * and the bottom of the vertical line
 */
static const int sequenceLineMargin = 20;

ObjectWidget::ObjectWidget(UMLView * view,  UMLObject *o, UMLWidgetData *pData) : UMLWidget(view, o, pData) {
	m_pLine = 0;
	init();
	int lineLength = ((ObjectWidgetData*)m_pData)->getLineLength();
	if( m_pLine && lineLength != 0 ) {
		m_pLine->setLineLength( lineLength );
	}
}

ObjectWidget::ObjectWidget(UMLView * view, UMLObject *o, int lid) : UMLWidget(view, o, new ObjectWidgetData(view->getOptionState() )) {
	m_pLine = 0;
	((ObjectWidgetData*)m_pData)->m_nLocalID = lid;
	((ObjectWidgetData*)m_pData)->m_InstanceName = "";
	((ObjectWidgetData*)m_pData)->m_bMultipleInstance = false;
	m_pData->setType(wt_Object);
	calculateSize();
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectWidget::ObjectWidget(UMLView * view) : UMLWidget(view, new ObjectWidgetData(view->getOptionState() )) {
	m_pLine = 0;
	((ObjectWidgetData*)m_pData)->m_nLocalID = -1;
	((ObjectWidgetData*)m_pData)->m_InstanceName = "";
	((ObjectWidgetData*)m_pData)->m_bMultipleInstance = false;
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ObjectWidget::init() {
	m_Doc = "";
	m_nOldID = 0;
	messageWidgetList.setAutoDelete(false);
	if( m_pView -> getType() == dt_Sequence ) {
		m_pLine = new SeqLineWidget( m_pView, this );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectWidget::~ObjectWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ObjectWidget::draw(QPainter & p , int offsetX, int offsetY) {
	if( ( (ObjectWidgetData *) m_pData ) -> getDrawAsActor() )
		drawActor( p, offsetX, offsetY );
	else
		drawObject( p, offsetX, offsetY );

	p.setPen(m_pData->getLineColour());
	if(m_bSelected)
		drawSelected(&p, offsetX, offsetY);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ObjectWidget::slotMenuSelection(int sel) {
	QString name = "";
	switch(sel) {
		case ListPopupMenu::mt_Rename_Object:

			bool ok;
			name = KLineEditDlg::getText(i18n("Enter object name:"), ((ObjectWidgetData*)m_pData)->m_InstanceName, &ok, m_pView);
			if(ok) {
				((ObjectWidgetData*)m_pData)->m_InstanceName = name;
				calculateSize();
				moveEvent( 0 );
				update();
			}
			break;

		case ListPopupMenu::mt_Properties:
			m_pView->getDocument() -> showProperties(this);
			calculateSize();
			moveEvent( 0 );
			update();
			break;

		case ListPopupMenu::mt_Up:
			tabUp();
			break;

		case ListPopupMenu::mt_Down:
			tabDown();
			break;

		default:
			UMLWidget::slotMenuSelection(sel);
			break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ObjectWidget::calculateSize() {
	int width, height, textWidth;
	QFontMetrics &fm = getFontMetrics(FT_UNDERLINE);
	int fontHeight  = fm.lineSpacing();
	if( ( (ObjectWidgetData *)m_pData ) -> getDrawAsActor() ) {
		QString t = ((ObjectWidgetData*)m_pData)->m_InstanceName + " : " + m_pObject -> getName();
		textWidth = fm.width( t );
		width = textWidth > A_WIDTH?textWidth:A_WIDTH;
		height = A_HEIGHT + fontHeight + A_MARGIN;
		width += A_MARGIN * 2;
	} else {
		QString t = ((ObjectWidgetData*)m_pData)->m_InstanceName + " : " + m_pObject -> getName();
		textWidth = fm.width(t);
		width = textWidth > O_WIDTH?textWidth:O_WIDTH;
		height = fontHeight + O_MARGIN * 2;
		width += O_MARGIN * 2;
		if(((ObjectWidgetData*)m_pData)->m_bMultipleInstance) {
			width += 10;
			height += 10;
		}
	}//end else drawasactor
	setSize(width, height);
	if( m_pView -> getType() == dt_Sequence ) {
//FIXME not quite sure what this did, but it broke paste
//		setY( (int)y() + (oldHeight - height ) );
	}
	emit sigWidgetMoved(((ObjectWidgetData*)m_pData)->m_nLocalID);//makes any message widgets connected resize themselves
	adjustAssocs( (int)x(), (int)y() );//adjust assoc lines
	moveEvent( 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////

QString ObjectWidget::getDoc() {
	return m_Doc;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ObjectWidget::setMultipleInstance(bool multiple) {
	//make sure only calling this in relation to an object on a collab. diagram
	if(m_pView -> getType() != dt_Collaboration)
		return;
	((ObjectWidgetData*)m_pData)->m_bMultipleInstance = multiple;
	calculateSize();
	update();
}

/** Activate the object after serializing it from a QDataStream */

bool ObjectWidget::activate(IDChangeLog* ChangeLog /*= 0*/) {
	bool status = UMLWidget::activate(ChangeLog);
	if(status) {
		calculateSize();
	}
	moveEvent( 0 );
	return status;
}

void ObjectWidget::synchronizeData() {
	UMLWidget::synchronizeData();
	if( m_pLine )
		((ObjectWidgetData*)m_pData)-> setLineLength( m_pLine -> getLineLength() );
}

void ObjectWidget::moveEvent(QMoveEvent */*m*/) {
	emit sigWidgetMoved( m_pData->getId() );
	if( m_pLine )
		m_pLine -> setStartPoint( (int)x() + width() / 2, (int)y() + height() );
}

void ObjectWidget::slotColorChanged(int /*viewID*/) {
	m_pData->getFillColour() = m_pView->getFillColour();
	m_pData->getLineColour() = m_pView->getLineColor();

	if( m_pLine)
		m_pLine -> setPen( QPen( m_pData->getLineColour(), 0, DashLine ) );
}

void ObjectWidget::cleanup() {

	UMLWidget::cleanup();
	if( m_pLine ) {
		m_pLine -> cleanup();
		delete m_pLine;
	}
}


void ObjectWidget::drawObject(QPainter & p, int offsetX, int offsetY) {

	QFont font = p.font();
	bool wasUnderlined = font.underline();
	font.setUnderline( true );
	p.setFont( font );

	p.setPen(m_pData->getLineColour());
	if(m_pData->getUseFillColor())
		p.setBrush(m_pData->getFillColour());
	else
		p.setBrush(m_pView -> viewport() -> backgroundColor());
	int w = width();
	int h= height();

	QString t = ((ObjectWidgetData*)m_pData)->m_InstanceName + " : " + m_pObject -> getName();
	if(((ObjectWidgetData*)m_pData)->m_bMultipleInstance) {
		p.drawRect(offsetX + 10, offsetY + 10, w - 10, h - 10);
		p.drawRect(offsetX + 5, offsetY + 5, w - 10, h - 10);
		p.drawRect(offsetX, offsetY, w - 10, h - 10);
		p.setPen(QPen(black));

		p.drawText(offsetX + O_MARGIN, offsetY + O_MARGIN, w - O_MARGIN * 2 - 10, h - O_MARGIN * 2 - 10, AlignCenter, t);
	} else {
		p.drawRect(offsetX, offsetY, w, h);
		p.setPen(QPen(black));
		p.drawText(offsetX + O_MARGIN, offsetY + O_MARGIN, w - O_MARGIN * 2, h - O_MARGIN * 2, AlignCenter, t);
	}

	font.setUnderline( wasUnderlined );
	p.setFont( font );
}

void ObjectWidget::drawActor(QPainter & p, int offsetX, int offsetY) {
	QFontMetrics &fm = getFontMetrics(FT_UNDERLINE);

	p.setPen(m_pData->getLineColour());
	if(m_pData->getUseFillColor())
		p.setBrush(m_pData->getFillColour());
	int w = width();
	int textStartY = A_HEIGHT + A_MARGIN;
	int fontHeight  = fm.lineSpacing();

	int middleX = w / 2;
	int thirdY = A_HEIGHT / 3;

	//draw actor
	p.drawEllipse(offsetX + middleX - A_WIDTH / 2, offsetY,  A_WIDTH, thirdY);//head
	p.drawLine(offsetX + middleX, offsetY + thirdY, offsetX + middleX, offsetY + thirdY * 2);//body
	p.drawLine(offsetX + middleX, offsetY + 2 * thirdY, offsetX + middleX - A_WIDTH / 2, offsetY + A_HEIGHT);//left leg

	p.drawLine(offsetX + middleX, offsetY +  2 * thirdY, offsetX + middleX + A_WIDTH / 2, offsetY + A_HEIGHT);//right leg
	p.drawLine(offsetX + middleX - A_WIDTH / 2, offsetY + thirdY + thirdY / 2, offsetX + middleX + A_WIDTH / 2, offsetY + thirdY + thirdY / 2);//arms
	//draw text
	p.setPen(QPen(black));
	QString t = ((ObjectWidgetData*)m_pData)->m_InstanceName + " : " + m_pObject -> getName();
	p.drawText(offsetX + A_MARGIN, offsetY + textStartY, w - A_MARGIN * 2, fontHeight, AlignCenter, t);
}

void ObjectWidget::mouseMoveEvent(QMouseEvent* me) {
	if( m_bMouseDown || me->button() == LeftButton ) {
		QPoint newPosition = doMouseMove(me);
		int newX = newPosition.x();
		int newY = newPosition.y();

		//implement rule for sequence diagram
		if( m_pView -> getType() == dt_Sequence ) {
			newY = (int)this -> y();
		}
		m_nOldX = newX;
		m_nOldY = newY;
		setX( newX );
		setY( newY );
		adjustAssocs(newX, newY);
		m_pView->resizeCanvasToItems();
		moveEvent(0);
	}
}

void ObjectWidget::tabUp() {
	int newY = (int)y() - height();
	newY = ( newY + height() ) < 80?80 - height():newY;
	setY( newY );
	moveEvent( 0 );
	adjustAssocs( (int)x(), newY);
}

void ObjectWidget::tabDown() {
	int newY = (int)y() + height();
	setY( newY );
	moveEvent( 0 );
	adjustAssocs( (int)x(), newY);
}

bool ObjectWidget::canTabUp() {
	return (int)y() >= 80;
}

void ObjectWidget::setShowDestruction( bool bShow ) {
	( ( ObjectWidgetData *)m_pData ) -> setShowDestruction( bShow );
	if( m_pLine )
		m_pLine -> setupDestructionBox();
}

int ObjectWidget::getEndLineY() {
	int y = (int)this -> y() + height();
	if( m_pLine)
		y += m_pLine -> getLineLength();
	if( ( ( ObjectWidgetData *)m_pData ) -> getShowDestruction() )
		y += 10;
	return y;
}

void ObjectWidget::messageAdded(MessageWidget* message) {
	messageWidgetList.append(message);
}

void ObjectWidget::messageRemoved(MessageWidget* message) {
	messageWidgetList.remove(message);
}

void ObjectWidget::slotMessageMoved() {
	QPtrListIterator<MessageWidget> iterator(messageWidgetList);
	MessageWidget* message;
	int lowestMessage = 0;
	while ( (message = iterator.current()) != 0 ) {
		++iterator;
		int messageHeight = (int)message->y() + message->height();
		if (lowestMessage < messageHeight) {
			lowestMessage = messageHeight;
		}
	}
	m_pLine->setEndOfLine(lowestMessage + sequenceLineMargin);
}

#include "objectwidget.moc"
