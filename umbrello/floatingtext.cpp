/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "associationwidget.h"
#include "umlview.h"
#include "umldoc.h"
#include "classifier.h"
#include "floatingtext.h"
#include "messagewidget.h"
#include "listpopupmenu.h"
#include "operation.h"

#include "dialogs/assocpropdlg.h"
#include "dialogs/selectopdlg.h"

#include <kdebug.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <qpainter.h>

FloatingText::FloatingText(UMLView * view, Text_Role role, QString text) : UMLWidget(view)
{
	init();
	m_Text = text;
	m_Role = role;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
FloatingText::FloatingText(UMLView * view) : UMLWidget(view)
{
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::init() {
	// initialize loaded/saved (i.e. persistent) data
	m_PreText = "";
	m_Text = "";
	m_PostText = "";
	m_SeqNum = "";
	m_Operation = "";
	m_Role = Uml::tr_Floating;
	m_Type = Uml::wt_Text;
	// initialize non-saved (i.e. volatile) data
	m_pAssoc = 0;
	m_pMessage = 0;
	if ( ! m_pView->getDocument()->loading() ) {
		calculateSize();
		setZ( 10 );//make sure always on top.
		update();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
FloatingText::~FloatingText() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::moveEvent(QMoveEvent * /*m*/) {
	adjustAssocs(getX(), getY());
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::resizeEvent(QResizeEvent * /*re*/) {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::setLinePos(int x, int y) {
	setX(x);
	setY(y);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::setLinePositionRelatively(int newX, int newY, int oldX, int oldY) {
	setX( getX() + (newX-oldX) );
	setY( getY() + (newY-oldY) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::setPositionFromMessage() {
	if (!m_pMessage) {
		return;
	}
	calculateSize();
	setLinePos(m_pMessage->getX() + 5, m_pMessage->getY() - height());
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::calculateSize() {
	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int h  = fm.lineSpacing();
	int w = fm.width( getDisplayText() );
	setSize( w + 8, h + 4 );//give a small margin
	adjustAssocs( getX(), getY() );//adjust assoc lines
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::slotMenuSelection(int sel) {
	QString t, newText;
	bool ok;
	int result;
	UMLClassifier * c = (UMLClassifier *)getUMLObject();
	bool done = false;
	QFont font;
	KLineEditDlg * dlg = 0;

	if( m_Role == tr_RoleAName || m_Role == tr_RoleBName ) {
		t = i18n("Enter role name");
	} else if (m_Role == tr_MultiA || m_Role == tr_MultiB) {
		t = i18n("Enter multiplicity");
		/*
		// NO! shouldnt be allowed
		} else if( m_Role == tr_ChangeA || m_Role == tr_ChangeB ) {
		t = i18n("Enter changeability");
		*/
	} else if (m_Role == tr_Name) {
		t = i18n("Enter association name");
	} else if (m_Role == tr_Floating) {
		t = i18n("Enter new text");
	} else {
		t = i18n("ERROR");
	}

	switch(sel) {
	case ListPopupMenu::mt_Properties:
		mouseDoubleClickEvent((QMouseEvent *)0);
		done = true;
		break;

	case ListPopupMenu::mt_Delete_Association:
		if(!m_pAssoc) {
			return;
		}
		m_pView->removeAssoc(m_pAssoc);
		done = true;
		break;

	case ListPopupMenu::mt_Delete:
		m_pView -> removeWidget(this);
		done = true;
		break;


	case ListPopupMenu::mt_Delete_Message:
		if (!m_pMessage) {
			if(m_pView -> getType() != dt_Collaboration)
				return;
			//here to delete m_pAssoc./m_pMessage on collab diagram.
			if(!m_pAssoc) {
				kdDebug() << "Error in floating text:no m_pAssoc set." << endl;
				return;
			}
			m_pView->removeAssoc(m_pAssoc);
			return;
		}
		//here to delete this from a seq. diagram.
		m_pMessage -> slotMenuSelection(ListPopupMenu::mt_Delete);
		//m_pMessage will delete this
		done = true;
		break;

	case ListPopupMenu::mt_Operation:
	{
		UMLOperation* newOperation = dynamic_cast<UMLOperation*>( m_pView->getDocument()->createUMLObject(c, ListPopupMenu::convert_MT_OT((ListPopupMenu::Menu_Type)sel)) );
		if (newOperation) {
			setOperation( newOperation->toString(st_NoSigNoScope) );
			setVisible(getText().length() > 0);
			calculateSize();
			setPositionFromMessage(); //force it to display
		}
		done = true;
	}
	break;

	case ListPopupMenu::mt_Sequence_Number:
		t = KLineEditDlg::getText(i18n("Enter sequence number:"), getSeqNum(), &ok, (QWidget*)m_pView);
		if(ok) {
			setSeqNum( t );
			calculateSize();
			setVisible( true );
		}
		done = true;
		break;

	case ListPopupMenu::mt_Select_Operation:
		showOpDlg();
		done = true;
		break;

	case ListPopupMenu::mt_Rename:
		dlg = new KLineEditDlg(t, getText(), m_pView);
		result = dlg ->exec();
		newText = dlg -> text();
		delete dlg;
		if(result && newText != getText() && isTextValid(newText) ) {
			if(m_pAssoc) {
				switch (m_Role) {
				case tr_Name:
					m_pAssoc->setName(newText);
					break;
				case tr_RoleAName:
					m_pAssoc->setRoleNameA(newText);
					break;
				case tr_RoleBName:
					m_pAssoc->setRoleNameB(newText);
					break;
					/*
					  NO! shouldnt be allowed
					  case tr_ChangeA:
					  m_pAssoc->setChangeWidgetA(newText);
					  break;
					  case tr_ChangeB:
					  m_pAssoc->setChangeWidgetB(newText);
					  break;
					*/
				case tr_MultiA:
					m_pAssoc->setMultiA(newText);
					break;
				case tr_MultiB:
					m_pAssoc->setMultiB(newText);
					break;
				default:
					break;
				}//end switch
				setVisible( true );
			} else {
				setText( newText );
				setVisible( true );
			}
		}
		if(!isTextValid(newText))
			if(m_Role  == tr_Floating)
				m_pView -> removeWidget(this);
		done = true;
		calculateSize();
		update();
		break;

	case ListPopupMenu::mt_Change_Font:
		font = getFont();
		if( KFontDialog::getFont( font, false, m_pView ) ) {
			if(m_Role  == tr_Floating || m_Role  == tr_Seq_Message ) {
				setFont( font );
			} else if (m_pAssoc) {
				m_pAssoc->changeFont(font);
			}
		}
		done = true;
		break;

	case ListPopupMenu::mt_Reset_Label_Positions:
		if (m_pAssoc) {
			m_pAssoc->resetTextPositions();
		}
		break;
	default:
		break;
	}//end switch
	if(!done)
		UMLWidget::slotMenuSelection(sel);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::setText(QString t) {
	m_Text = t;
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::setPreText (QString t)
{
	m_PreText = t;
	calculateSize();
	update();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::setPostText(QString t) {
	m_PostText = t;
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::startMenu(AssociationWidget * a, QPoint p) {
	m_pAssoc = a;
	startPopupMenu(p);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::changeTextDlg() {
	KLineEditDlg dlg(i18n("Enter new text:"), getText(), m_pView);
	int result = dlg.exec();
	QString newText = dlg.text();

	if(result && newText !=  getText() && isTextValid(newText)) {
		setText( newText );
		setVisible( ( getText().length() > 0 ) );
		calculateSize();
		update();
	}
	if(!isTextValid(newText))
		hide();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::mouseDoubleClickEvent(QMouseEvent * /* me*/) {
	if(m_pView -> getCurrentCursor() != WorkToolBar::tbb_Arrow)
		return;
	if(m_pAssoc) {
		if(m_Role  == tr_Coll_Message || m_Role  == tr_Coll_Message_Self)
		{
			showOpDlg();
		}
		else
		{
			AssocPropDlg dlg(static_cast<QWidget*>(m_pView), m_pAssoc );
			int result = dlg.exec();
			QString rnA = dlg.getRoleAName(), rnB = dlg.getRoleBName(),
				 ma = dlg.getMultiA(), mb = dlg.getMultiB();
			if(result) {
				m_pAssoc -> setRoleNameA(rnA);
				m_pAssoc -> setRoleNameB(rnB);
				m_pAssoc -> setMultiA(ma);
				m_pAssoc -> setMultiB(mb);
			}
		} //end if m_Role
	}//end if m_pAssoc
	else if(m_Role  == tr_Seq_Message || m_Role  == tr_Seq_Message_Self) {
		//if on a seq. diagram m_pAssoc won't be set
		//but we still need to show the op. dialog
		showOpDlg();
	} else if (m_Role == tr_Floating) {
		// double clicking on a text line opens the dialog to change the text
		slotMenuSelection(ListPopupMenu::mt_Rename);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::showOpDlg() {
	UMLClassifier* c = (UMLClassifier*)getUMLObject();
	SelectOpDlg selectDlg((QWidget*)m_pView, c);
	selectDlg.setSeqNumber( getSeqNum() );
	selectDlg.setCustomOp( getOperation() );
	int result = selectDlg.exec();
	if(!result) {
		return;
	}

	setSeqNum( selectDlg.getSeqNumber() );
	setOperation( selectDlg.getOpText() );
	setVisible(getText().length() > 0);
	calculateSize();
	setPositionFromMessage(); //force it to display
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::mouseMoveEvent(QMouseEvent* me) {
	if( m_bMouseDown || me->button() == LeftButton ) {
		QPoint newPosition = doMouseMove(me);
		int newX = newPosition.x();
		int newY = newPosition.y();

		//implement specific rules for a sequence diagram
		if( m_Role  == tr_Seq_Message || m_Role  == tr_Seq_Message_Self) {
			newX = m_pMessage->getX() + 5;
			int minHeight = m_pMessage->getMinHeight();
			newY = newY < minHeight ? minHeight : newY;

			int maxHeight = m_pMessage->getMaxHeight() - height() - 5;
			newY = newY < maxHeight ? newY : maxHeight;
			m_pMessage->setX( newX - 5 );

			m_pMessage->setY( newY + height() );
		}
		m_nOldX = newX;
		m_nOldY = newY;
		setX( newX );
		setY( newY );
		if(m_pAssoc)
			m_pAssoc->calculateNameTextSegment();
		m_pView->resizeCanvasToItems();
		moveEvent(0);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString FloatingText::getPreText() const {
	return m_PreText;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString FloatingText::getPostText() const {
	return m_PostText;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString FloatingText::getText() const {
	//test to make sure not just the ":" between the seq number
	//and the actual m_pMessage
	// hmm. this section looks like it could have been avoided by using pre-, post- text
	// instead of storing in the main body of the text -b.t.
	if(m_Role  == tr_Seq_Message || m_Role  == tr_Seq_Message_Self ||
	   m_Role  == tr_Coll_Message || m_Role  == tr_Coll_Message_Self) {
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

void FloatingText::synchronizeData() {
	if( m_Role  == tr_Coll_Message || m_Role  == tr_Seq_Message ) {
		setText( getSeqNum().append(": ").append( getOperation() ) );
	}
	UMLWidget::synchronizeData();
}

void FloatingText::setMessage(MessageWidget* m) {
	m_pMessage = m;
	setPositionFromMessage();
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

void FloatingText::setSelected(bool _select) {
	if( m_nOldID == -10 )//used to stop a recursive call
	{
		m_nOldID = 0;
		return;
	}
	UMLWidget::setSelected( _select );
	if( !m_pMessage )
		return;
	if( m_bSelected && m_pMessage -> getSelected() )
		return;
	if( !m_bSelected && !m_pMessage -> getSelected() )
		return;

	m_nOldID = -10;
	m_pView -> setSelected( m_pMessage, 0 );
	m_pMessage -> setSelected( m_bSelected );
}

bool FloatingText::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement textElement = qDoc.createElement( "UML:FloatingTextWidget" );
	bool status = UMLWidget::saveToXMI( qDoc, textElement );
	textElement.setAttribute( "text", m_Text );
	textElement.setAttribute( "pretext", m_PreText );
	textElement.setAttribute( "posttext", m_PostText );
	textElement.setAttribute( "operation", m_Operation );
	textElement.setAttribute( "seqnum", m_SeqNum );
	textElement.setAttribute( "role", m_Role );
	qElement.appendChild( textElement );
	return status;
}

bool FloatingText::loadFromXMI( QDomElement & qElement ) {
	if( !UMLWidget::loadFromXMI( qElement ) )
		return false;
	m_Text = qElement.attribute( "text", "" );
	m_PreText = qElement.attribute( "pretext", "" );
	m_PostText = qElement.attribute( "posttext", "" );
	m_Operation  = qElement.attribute( "operation", "" );
	m_SeqNum = qElement.attribute( "seqnum", "" );
	QString role = qElement.attribute( "role", "" );

	if( !role.isEmpty() )
		m_Role = (Uml::Text_Role)role.toInt();

	return true;
}

