/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// own header file
#include "objectwidget.h"
// system includes
#include <qpainter.h>
#include <qvalidator.h>
#include <klocale.h>
#include <kdebug.h>
// local includes
#include "inputdialog.h"
#include "seqlinewidget.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "umlobject.h"
#include "listpopupmenu.h"

/**
 * The number of pixels margin between the lowest message
 * and the bottom of the vertical line
 */
static const int sequenceLineMargin = 20;

ObjectWidget::ObjectWidget(UMLView * view, UMLObject *o, int lid) : UMLWidget(view, o) {
	init();
	if( lid >= 0 )
		m_nLocalID = lid;
	//calculateSize();  Doing this during loadFromXMI() gives futile updates.
	//                  Instead, it is done afterwards by UMLWidget::activate()
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ObjectWidget::init() {
	UMLWidget::setBaseType(wt_Object);
	m_Doc = "";
	m_nLocalID = -1;
	m_InstanceName = "";
	m_bMultipleInstance = false;
	m_bDrawAsActor = false;
	m_bShowDestruction = false;
	messageWidgetList.setAutoDelete(false);
	if( m_pView != NULL && m_pView -> getType() == dt_Sequence ) {
		m_pLine = new SeqLineWidget( m_pView, this );
	} else {
		m_pLine = NULL;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectWidget::~ObjectWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ObjectWidget::draw(QPainter & p , int offsetX, int offsetY) {
	if ( m_bDrawAsActor )
		drawActor( p, offsetX, offsetY );
	else
		drawObject( p, offsetX, offsetY );

	UMLWidget::draw(p, offsetX, offsetY);
	if(m_bSelected)
		drawSelected(&p, offsetX, offsetY);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ObjectWidget::slotMenuSelection(int sel) {
	QString name = "";
	switch(sel) {
		case ListPopupMenu::mt_Rename_Object:
		{
			bool ok;
			QRegExpValidator* validator = new QRegExpValidator(QRegExp(".*"), 0);
			name = KInputDialog::getText
				(i18n("Rename Object"),
				 i18n("Enter object name:"),
				 m_InstanceName,
				 &ok,
				 m_pView,
#if KDE_IS_VERSION(3,1,90)
				 "renameobject",
#endif
				 validator);
			if (ok) {
				m_InstanceName = name;
				calculateSize();
				moveEvent( 0 );
				update();
				UMLApp::app()->getDocument()->setModified(true);
			}
			delete validator;
			break;
		}
		case ListPopupMenu::mt_Properties:
			UMLApp::app()->getDocument() -> showProperties(this);
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
	if ( m_bDrawAsActor ) {
		QString t = m_InstanceName + " : " + m_pObject -> getName();
		textWidth = fm.width( t );
		width = textWidth > A_WIDTH?textWidth:A_WIDTH;
		height = A_HEIGHT + fontHeight + A_MARGIN;
		width += A_MARGIN * 2;
	} else {
		QString t = m_InstanceName + " : " + m_pObject -> getName();
		textWidth = fm.width(t);
		width = textWidth > O_WIDTH?textWidth:O_WIDTH;
		height = fontHeight + O_MARGIN * 2;
		width += O_MARGIN * 2;
		if (m_bMultipleInstance) {
			width += 10;
			height += 10;
		}
	}//end else drawasactor
	setSize(width, height);
	adjustAssocs( getX(), getY() );//adjust assoc lines
	moveEvent( 0 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////

QString ObjectWidget::getDoc() const {
	return m_Doc;
}

void ObjectWidget::setDrawAsActor( bool drawAsActor ) {
	m_bDrawAsActor = drawAsActor;
	calculateSize();
}

void ObjectWidget::setMultipleInstance(bool multiple) {
	//make sure only calling this in relation to an object on a collab. diagram
	if(m_pView -> getType() != dt_Collaboration)
		return;
	m_bMultipleInstance = multiple;
	calculateSize();
	update();
}

bool ObjectWidget::activate(IDChangeLog* ChangeLog /*= 0*/) {
	bool status = UMLWidget::activate(ChangeLog);
	if (status) {
		calculateSize();
	}
	moveEvent(0);
	return status;
}

void ObjectWidget::setX( int x ) {
	UMLWidget::setX(x);
	moveEvent(0);
}

void ObjectWidget::setY( int y ) {
	UMLWidget::setY(y);
	moveEvent(0);
}

void ObjectWidget::moveEvent(QMoveEvent */*m*/) {
	emit sigWidgetMoved( m_nLocalID );
	if( m_pLine )
		m_pLine -> setStartPoint( getX() + width() / 2, getY() + height() );
}

void ObjectWidget::slotColorChanged(int /*viewID*/) {
	UMLWidget::getFillColour() = m_pView->getFillColor();
	UMLWidget::getLineColour() = m_pView->getLineColor();

	if( m_pLine)
		m_pLine -> setPen( QPen( UMLWidget::getLineColour(), UMLWidget::getLineWidth(), DashLine ) );
}

void ObjectWidget::cleanup() {

	UMLWidget::cleanup();
	if( m_pLine ) {
		m_pLine -> cleanup();
		delete m_pLine;
	}
}


void ObjectWidget::drawObject(QPainter & p, int offsetX, int offsetY) {

	QFont oldFont = p.font();
	QFont font = UMLWidget::getFont();
	font.setUnderline( true );
	p.setFont( font );

	UMLWidget::draw(p, offsetX, offsetY);
	if(UMLWidget::getUseFillColour())
		p.setBrush(UMLWidget::getFillColour());
	else
		p.setBrush(m_pView -> viewport() -> backgroundColor());
	int w = width();
	int h= height();

	QString t = m_InstanceName + " : " + m_pObject -> getName();
	if ( m_bMultipleInstance ) {
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

	p.setFont( oldFont );
}

void ObjectWidget::drawActor(QPainter & p, int offsetX, int offsetY) {
	QFontMetrics &fm = getFontMetrics(FT_UNDERLINE);

	UMLWidget::draw(p, offsetX, offsetY);
	if ( UMLWidget::getUseFillColour() )
		p.setBrush( UMLWidget::getFillColour() );
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
	QString t = m_InstanceName + " : " + m_pObject -> getName();
	p.drawText(offsetX + A_MARGIN, offsetY + textStartY, w - A_MARGIN * 2, fontHeight, AlignCenter, t);
}

void ObjectWidget::mouseMoveEvent(QMouseEvent* me) {
	if( m_bMouseDown || me->button() == LeftButton ) {
		QPoint newPosition = doMouseMove(me);
		int newX = newPosition.x();
		int newY = newPosition.y();

		//implement rule for sequence diagram
		if( m_pView -> getType() == dt_Sequence ) {
			newY = this -> getY();
		}
		m_nOldX = newX;
		m_nOldY = newY;
		setX( newX );
		setY( newY );
		adjustAssocs(newX, newY);
		m_pView->resizeCanvasToItems();
	}
}

void ObjectWidget::tabUp() {
	int newY = getY() - height();
	if (newY < topMargin())
		newY = topMargin();
	setY( newY );
	moveEvent( 0 );
	adjustAssocs( getX(), newY);
}

void ObjectWidget::tabDown() {
	int newY = getY() + height();
	setY( newY );
	moveEvent( 0 );
	adjustAssocs( getX(), newY);
}

int ObjectWidget::topMargin() {
	return 80 - height();
}

bool ObjectWidget::canTabUp() {
	int y = getY();
	//kdDebug() << "ObjectWidget::canTabUp: y is " << y << endl;
	return (y > topMargin());
}

void ObjectWidget::setShowDestruction( bool bShow ) {
	m_bShowDestruction = bShow;
	if( m_pLine )
		m_pLine -> setupDestructionBox();
}

int ObjectWidget::getEndLineY() {
	int y = this -> getY() + getHeight();
	if( m_pLine)
		y += m_pLine -> getLineLength();
	if ( m_bShowDestruction )
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
	MessageWidgetListIt iterator(messageWidgetList);
	MessageWidget* message;
	int lowestMessage = 0;
	while ( (message = iterator.current()) != 0 ) {
		++iterator;
		int messageHeight = message->getY() + message->getHeight();
		if (lowestMessage < messageHeight) {
			lowestMessage = messageHeight;
		}
	}
	m_pLine->setEndOfLine(lowestMessage + sequenceLineMargin);
}

bool ObjectWidget::messageOverlap(int y, MessageWidget* messageWidget) {
	MessageWidgetListIt iterator(messageWidgetList);
	MessageWidget* message;
	while ( (message = iterator.current()) != 0 ) {
		++iterator;
		int messageHeight = message->getY() + message->getHeight();
		if (y >= message->getY() && y <= messageHeight && message != messageWidget) {
			return true;
		}
	}
	return false;
}

void ObjectWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement objectElement = qDoc.createElement( "objectwidget" );
	UMLWidget::saveToXMI( qDoc, objectElement );
	objectElement.setAttribute( "instancename", m_InstanceName );
	objectElement.setAttribute( "drawasactor", m_bDrawAsActor );
	objectElement.setAttribute( "multipleinstance", m_bMultipleInstance );
	objectElement.setAttribute( "localid", m_nLocalID );
	objectElement.setAttribute( "decon", m_bShowDestruction );
	qElement.appendChild( objectElement );
}

bool ObjectWidget::loadFromXMI( QDomElement & qElement ) {
	if( !UMLWidget::loadFromXMI( qElement ) )
		return false;
	m_InstanceName = qElement.attribute( "instancename", "" );
	QString draw = qElement.attribute( "drawasactor", "0" );
	QString multi = qElement.attribute( "multipleinstance", "0" );
	QString localid = qElement.attribute( "localid", "0" );
	QString decon = qElement.attribute( "decon", "0" );

	m_bDrawAsActor = (bool)draw.toInt();
	m_bMultipleInstance = (bool)multi.toInt();
	m_nLocalID = localid.toInt();
	m_bShowDestruction = (bool)decon.toInt();
	return true;

}

#include "objectwidget.moc"
