/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "floatingtext.h"

// system includes
#include <kdebug.h>
#include <klocale.h>
#include <qpainter.h>

// local includes
#include "associationwidget.h"
#include "association.h"
#include "umlview.h"
#include "umldoc.h"
#include "classifier.h"
#include "messagewidget.h"
#include "listpopupmenu.h"
#include "operation.h"
#include "inputdialog.h"
#include "dialogs/assocpropdlg.h"
#include "dialogs/selectopdlg.h"

FloatingText::FloatingText(UMLView * view, Text_Role role, QString text, int id)
  : UMLWidget(view, id)
{
	init();
	m_Text = text;
	m_Role = role;
}

void FloatingText::init() {
	// initialize loaded/saved (i.e. persistent) data
	m_PreText = "";
	m_Text = "";
	m_PostText = "";
	//m_SeqNum = "";
	//m_Operation = "";
	m_Role = Uml::tr_Floating;
	m_Type = Uml::wt_Text;
	// initialize non-saved (i.e. volatile) data
	m_pLink = NULL;
	if ( ! m_pView->getDocument()->loading() ) {
		calculateSize();
		setZ( 10 );//make sure always on top.
		update();
	}
}

FloatingText::~FloatingText() {
}

void FloatingText::draw(QPainter & p, int offsetX, int offsetY) {
	int w = width();
	int h = height();
	p.setFont( UMLWidget::getFont() );
	QColor textColor(50, 50, 50);
	p.setPen(textColor);
	p.drawText( offsetX , offsetY,w,h, AlignCenter, getDisplayText() );
	if(m_bSelected)
		drawSelected(&p, offsetX, offsetY);
}

void FloatingText::moveEvent(QMoveEvent * /*m*/) {
	// adjustAssocs(getX(), getY()); // NO, I dont think so. This can create major problems.
}

void FloatingText::resizeEvent(QResizeEvent * /*re*/) {}

void FloatingText::setLinePos(int x, int y) {
	bool xIsValid = (x >= restrictPositionMin && x <= restrictPositionMax);
	bool yIsValid = (y >= restrictPositionMin && y <= restrictPositionMax);
	if (xIsValid && yIsValid) { // fine
		setX(x);
		setY(y);
	} else { // something is broken
		kdDebug() << "FloatingText::setLinePositionRelatively( " << x
			<< " , " << y << " ) - was blocked because at least one value is out of bounds: ["
			<< restrictPositionMin << "..." << restrictPositionMax << "]  "
			<< "origX: " << getX() << ", origY: " << getY()
			<< endl;
		// Let's just leave them at their original values.
	}
}

void FloatingText::setLinePositionRelatively(int newX, int newY, int oldX, int oldY) {
	int myNewX = getX() + (newX-oldX);
	int myNewY = getY() + (newY-oldY);
	bool xIsValid = (myNewX >= restrictPositionMin && myNewX <= restrictPositionMax);
	bool yIsValid = (myNewY >= restrictPositionMin && myNewY <= restrictPositionMax);
	if (xIsValid && yIsValid) { // fine
		bool oldIgnoreSnapToGrid = m_bIgnoreSnapToGrid;
		m_bIgnoreSnapToGrid = true;
		setX(myNewX);
		setY(myNewY);
		m_bIgnoreSnapToGrid = oldIgnoreSnapToGrid;
	} else { // something is broken
		kdDebug() << "FloatingText::setLinePositionRelatively( " << myNewX
			<< " , " << myNewY << " ) - was Blocked because at least one value is out of bounds: ["
			<< restrictPositionMin << "..." << restrictPositionMax << "]\n"
			<< "ToX: " << newX << ", ToY: " << newY
			<< "FromX: " << oldX << ", FromY: " << oldY
			<< "CurrentPointX: " << getX() << ", CurrentPointY: " << getY()
			<< endl;
		// Let's just leave them at their original values.
	}
}

void FloatingText::calculateSize() {
	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int h = fm.lineSpacing();
	int w = fm.width( getDisplayText() );
	setSize( w + 8, h + 4 );//give a small margin
}

void FloatingText::slotMenuSelection(int sel) {
	switch(sel) {
	case ListPopupMenu::mt_Properties:
		mouseDoubleClickEvent((QMouseEvent *)0);
		break;

	case ListPopupMenu::mt_Delete_Association:
	{
		AssociationWidget *a = dynamic_cast<AssociationWidget*>(m_pLink);
		if (a)
			m_pView->removeAssoc(a);
		kdDebug() << "FloatingText::slotMenuSelection(mt_Delete_Association)"
			  << " is called." << endl;
		break;
	}

	case ListPopupMenu::mt_Delete:
		m_pView -> removeWidget(this);
		break;

	case ListPopupMenu::mt_Delete_Message:
	{
		if (m_pLink == NULL)
			return;
		AssociationWidget *a = dynamic_cast<AssociationWidget*>(m_pLink);
		if (a) {
			if(m_pView -> getType() != dt_Collaboration)
				return;
			//here to delete assoc. on collab diagram.
			m_pView->removeAssoc( a );
		} else {
			//here to delete this from a seq. diagram.
			MessageWidget *m = static_cast<MessageWidget*>(m_pLink);
			m_pView->removeWidget( m );
		}
		break;
	}

	case ListPopupMenu::mt_Operation:
		{
			kdDebug() << "FloatingText::slotMenuSelection(mt_Operation) is called."
				  << endl;
			if (m_pLink == NULL) {
				kdDebug() << "FloatingText::slotMenuSelection(mt_Operation): "
					  << "m_pLink is NULL" << endl;
				return;
			}
			UMLClassifier* c;
			AssociationWidget *a = dynamic_cast<AssociationWidget*>(m_pLink);
			if (a)  {
				if (a->getAssocType() == at_Coll_Message)
					c = (UMLClassifier*)( a->getWidget(B)->getUMLObject() );
				else
					c = (UMLClassifier*)( a->getWidget(A)->getUMLObject() );
			} else {
				c = (UMLClassifier*)getUMLObject();
			}
			UMLObject* umlObj = m_pView->getDocument()->createChildObject(c, Uml::ot_Operation);
			UMLOperation* newOperation = static_cast<UMLOperation*>( umlObj );
			if (a) {
				a->setName( newOperation->toString(st_SigNoScope) );
			} else {
				MessageWidget *m = static_cast<MessageWidget*>(m_pLink);
				m->setOperation( newOperation->toString(st_SigNoScope) );
				setMessageText();
			}
		}
		break;

	case ListPopupMenu::mt_Select_Operation:
		showOpDlg();
		break;

	case ListPopupMenu::mt_Rename:
		handleRename();
		break;

	case ListPopupMenu::mt_Change_Font:
		{
			QFont font = getFont();
			if( KFontDialog::getFont( font, false, m_pView ) ) {
				if( m_Role == tr_Floating || m_Role == tr_Seq_Message ) {
					setFont( font );
				} else {
					AssociationWidget *a = dynamic_cast<AssociationWidget*>(m_pLink);
					if (a)
						a->setFont(font);
				}
			}
		}
		break;

	case ListPopupMenu::mt_Reset_Label_Positions:
		{
			AssociationWidget *a = dynamic_cast<AssociationWidget*>(m_pLink);
			if (a)
				a->resetTextPositions();
		}
		break;

	default:
		UMLWidget::slotMenuSelection(sel);
		break;
	}//end switch
}

void FloatingText::handleRename() {
	QString t;
	UMLDoc *doc = m_pView->getDocument();

	if( m_Role == tr_RoleAName || m_Role == tr_RoleBName ) {
		t = i18n("Enter role name:");
	} else if (m_Role == tr_MultiA || m_Role == tr_MultiB) {
		t = i18n("Enter multiplicity:");
		/*
		// NO! shouldnt be allowed
		} else if( m_Role == tr_ChangeA || m_Role == tr_ChangeB ) {
		t = i18n("Enter changeability");
		*/
	} else if (m_Role == tr_Name) {
		t = i18n("Enter association name:");
	} else if (m_Role == tr_Floating) {
		t = i18n("Enter new text:");
	} else {
		t = i18n("ERROR");
	}
	bool ok = false;
	QString newText = KInputDialog::getText(i18n("Rename"), t, getText(), &ok, m_pView);
	if (!ok)  {
		return;
	}
	bool valid = isTextValid(newText);
	if (!valid || newText == getText()) {
		if (!valid && m_Role == tr_Floating)
			m_pView -> removeWidget(this);
		calculateSize();
		update();
		return;
	}
	AssociationWidget *a = dynamic_cast<AssociationWidget*>(m_pLink);
	if (a) {
		a->setText( newText, m_Role );
	} else {
		setText( newText );
		doc -> setModified(true);
	}
	setVisible( true );
	calculateSize();
	update();
}

void FloatingText::setText(QString t) {
	if (m_Role == tr_Seq_Message || m_Role == tr_Seq_Message_Self) {
		MessageWidget *m = static_cast<MessageWidget*>(m_pLink);
		QString seqNum = m->getSequenceNumber();
		QString op = m->getOperation();
		if (seqNum.length() > 0 || op.length() > 0)
			m_Text = seqNum.append(": ").append( op );
		else
			m_Text = t;
	} else
		m_Text = t;
	calculateSize();
	update();
}

void FloatingText::setPreText (QString t)
{
	m_PreText = t;
	calculateSize();
	update();
}

void FloatingText::setPostText(QString t) {
	m_PostText = t;
	calculateSize();
	update();
}

void FloatingText::changeTextDlg() {
	bool ok = false;
	QString newText = KInputDialog::getText(i18n("Change Text"), i18n("Enter new text:"), getText(), &ok, m_pView);

	if(ok && newText != getText() && isTextValid(newText)) {
		setText( newText );
		setVisible( ( getText().length() > 0 ) );
		calculateSize();
		update();
	}
	if(!isTextValid(newText))
		hide();
}

void FloatingText::mouseDoubleClickEvent(QMouseEvent * /* me*/) {
	if(m_pView -> getCurrentCursor() != WorkToolBar::tbb_Arrow)
		return;
	if (m_Role == tr_Coll_Message || m_Role == tr_Coll_Message_Self ||
	    m_Role == tr_Seq_Message || m_Role == tr_Seq_Message_Self) {
		showOpDlg();
	} else if (m_Role == tr_Floating) {
		// double clicking on a text line opens the dialog to change the text
		handleRename();
	} else {
		AssociationWidget *a = dynamic_cast<AssociationWidget*>(m_pLink);
		if (a)
			a->showDialog();
	}
}

void FloatingText::showOpDlg() {
	QString seqNum, op;
	MessageWidget *m = dynamic_cast<MessageWidget*>(m_pLink);
	AssociationWidget *a = dynamic_cast<AssociationWidget*>(m_pLink);
	UMLClassifier* c = (UMLClassifier*)getUMLObject();
	if (m) {
		seqNum = m->getSequenceNumber();
		op = m->getOperation();
	} else if (a)  {
		seqNum = a->getMulti(A);
		op = a->getName();
		c = (UMLClassifier*)( a->getWidget(B)->getUMLObject() );
	}

	SelectOpDlg selectDlg((QWidget*)m_pView, c);
	selectDlg.setSeqNumber( seqNum );
	selectDlg.setCustomOp( op );
	int result = selectDlg.exec();
	if(!result) {
		return;
	}
	seqNum = selectDlg.getSeqNumber();
	op = selectDlg.getOpText();
	QString displayText = seqNum + ": " + op;
	if (m) {
		m->setSequenceNumber( seqNum );
		m->setOperation( op );
	} else if (a) {
		a->setName(op);
		a->setMulti(seqNum, A);
	}
	setMessageText();
}

void FloatingText::mouseMoveEvent(QMouseEvent* me) {
	if( m_bMouseDown || me->button() == LeftButton ) {
		QPoint newPosition = doMouseMove(me);
		int newX = newPosition.x();
		int newY = newPosition.y();

		//implement specific rules for a sequence diagram
		if( m_Role == tr_Seq_Message || m_Role == tr_Seq_Message_Self) {
			MessageWidget *m = static_cast<MessageWidget*>(m_pLink);
			m->updateMessagePos(getHeight(), newX, newY);
		}
		m_nOldX = newX;
		m_nOldY = newY;
		setX( newX );
		setY( newY );
		AssociationWidget *a = dynamic_cast<AssociationWidget*>(m_pLink);
		if (a)
			a->calculateNameTextSegment();
		m_pView->resizeCanvasToItems();
		moveEvent(0);
	}
}

QString FloatingText::getPreText() const {
	return m_PreText;
}

QString FloatingText::getPostText() const {
	return m_PostText;
}

QString FloatingText::getText() const {
	//test to make sure not just the ":" between the seq number
	//and the actual message widget
	// hmm. this section looks like it could have been avoided by using pre-, post- text
	// instead of storing in the main body of the text -b.t.
	if(m_Role == tr_Seq_Message || m_Role == tr_Seq_Message_Self ||
	   m_Role == tr_Coll_Message || m_Role == tr_Coll_Message_Self) {
		if( m_Text.length() <= 1 || m_Text == ": " )
			return "";
	}
	return m_Text;
}

QString FloatingText::getDisplayText() const
{
	QString displayText = m_Text;
	displayText.prepend(m_PreText);
	displayText.append(m_PostText);
	return displayText;
}

bool FloatingText::activate( IDChangeLog* ChangeLog /*= 0 */) {
	bool status = UMLWidget::activate( ChangeLog );
	calculateSize();
	update();
	return status;
}

// God this is bad. Why not use the assocList in the
// parent umlwidget class? At the very least, we should
// keep them synced.
// Also, there seems to be some issues with setting id to the association
// XMI id. There CAN be more than one floating text widget with this id
// but a findWidget will come up with only the first one.
void FloatingText::setLink(LinkWidget * l) {
	AssociationWidget *a = dynamic_cast<AssociationWidget*>(m_pLink);
	if (a) {
		// remove pre-existing association from our umlwidget assoc list
		removeAssoc(a);
	}
	m_pLink = l;
	if (l == NULL)
		return;
	a = dynamic_cast<AssociationWidget*>(m_pLink);
	if (a != NULL) {
		UMLAssociation *umla = a->getAssociation();
		if (umla != NULL) // *always* sync id to association id.
			setID( umla->getID() );
		UMLWidget::addAssoc(a);
	} else { // Uml::wt_Message
		MessageWidget *m = static_cast<MessageWidget*>(m_pLink);
		if (getID() > 0) {
			kdDebug() << "FloatingText::setLink: overriding own id "
				  << getID() << " with new value " << m->getID()
				  << endl;
		}
		setID( m->getID() );
		m->setTextPosition();
	}
}

LinkWidget * FloatingText::getLink() {
	return m_pLink;
}

void FloatingText::setRole(Text_Role role) {
	m_Role = role;
}

Uml::Text_Role FloatingText::getRole() const {
	return m_Role;
}

bool FloatingText::isTextValid(QString text) {
	int length = text.length();
	if(length < 1)
		return false;
	for(int i=0;i<length;i++)
		if(!text.at(i).isSpace())
			return true;
	return false;
}

/*
void FloatingText::setSeqNum(QString sn)
{
	m_SeqNum = sn;
	update();
}

QString FloatingText::getSeqNum() const {
	return m_SeqNum;
}

void FloatingText::setOperation(QString op) {
	m_Operation = op;
}

QString FloatingText::getOperation() const {
	return m_Operation;
}
*/

void FloatingText::setSelected(bool _select) {
	if( m_nOldID == -10 )//used to stop a recursive call
	{
		m_nOldID = 0;
		return;
	}
	UMLWidget::setSelected( _select );
	if (m_Role != tr_Seq_Message && m_Role != tr_Seq_Message_Self)
		return;
	MessageWidget *m = static_cast<MessageWidget*>(m_pLink);
	if (m_bSelected && m->getSelected())
		return;
	if (!m_bSelected && !m->getSelected())
		return;

	m_nOldID = -10;
	m_pView -> setSelected( m, NULL );
	m -> setSelected( m_bSelected );
}

void FloatingText::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement textElement = qDoc.createElement( "floatingtext" );
	UMLWidget::saveToXMI( qDoc, textElement );
	textElement.setAttribute( "text", m_Text );
	textElement.setAttribute( "pretext", m_PreText );
	textElement.setAttribute( "posttext", m_PostText );

	/* No need to save these - the messagewidget already did it.
	m_Operation  = qElement.attribute( "operation", "" );
	m_SeqNum = qElement.attribute( "seqnum", "" );
	 */

	textElement.setAttribute( "role", m_Role );
	qElement.appendChild( textElement );
}

bool FloatingText::loadFromXMI( QDomElement & qElement ) {
	if( !UMLWidget::loadFromXMI( qElement ) )
		return false;

	QString role = qElement.attribute( "role", "" );
	if( !role.isEmpty() )
		m_Role = (Uml::Text_Role)role.toInt();

	m_PreText = qElement.attribute( "pretext", "" );
	m_PostText = qElement.attribute( "posttext", "" );
	m_Text = qElement.attribute( "text", "" );
	// If all texts are empty then this is a useless widget.
	// In that case we return false.
	// CAVEAT: The caller should not interpret the false return value
	//  as an indication of failure since previous umbrello versions
	//  saved lots of these empty FloatingTexts.
	bool isDummy = (m_Text == "" && m_PreText == "" && m_PostText == "");
	return !isDummy;
}

void FloatingText::setMessageText() {
	QString displayText;
	MessageWidget *m  = dynamic_cast<MessageWidget*>(m_pLink);
	AssociationWidget *a = dynamic_cast<AssociationWidget*>(m_pLink);
	if (m) {
		QString seqNum = m->getSequenceNumber();
		QString op = m->getOperation();
		displayText = seqNum + ": " + op;
		setText( displayText );
		m->setTextPosition();
	} else if (a) {
		AssociationWidget *a  = static_cast<AssociationWidget*>(m_pLink);
		displayText = a->getName();
		setText( displayText );
	}

	setVisible(getText().length() > 0);
	calculateSize();
}

