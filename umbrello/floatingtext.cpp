/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "associationwidget.h"
#include "concept.h"
#include "floatingtext.h"
#include "floatingtextdata.h"
#include "messagewidget.h"
#include "operation.h"
#include "dialogs/assocpropdlg.h"
#include "dialogs/selectopdlg.h"
#include <kdebug.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <qpainter.h>

FloatingText::FloatingText(UMLView * view, UMLWidgetData* pData) : UMLWidget(view, pData) {
	init();
}

FloatingText::FloatingText(UMLView * view, Text_Role role, QString text)
  : UMLWidget(view, new FloatingTextData()) {
	((FloatingTextData*)m_pData)->m_Text = text;
	setRole( role );
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
FloatingText::FloatingText(UMLView * view) : UMLWidget(view, new FloatingTextData()) {
	setRole( tr_Floating );
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::init() {
	m_pAssoc = 0;
	m_pMessage = 0;
	((FloatingTextData*)m_pData)-> m_Type = wt_Text;
	calculateSize();
	setZ( 10 );//make sure always on top.
	update();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
FloatingText::~FloatingText() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::draw(QPainter & p, int offsetX, int offsetY) {
	int w = width();
	int h = height();
	p.setFont( m_pData -> getFont() );
	QColor textColor(50, 50, 50);
	p.setPen(textColor);
	p.drawText( offsetX , offsetY,w,h, AlignCenter, getText() );
	if(m_bSelected)
		drawSelected(&p, offsetX, offsetY);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::moveEvent(QMoveEvent * /*m*/) {
	int x = (int)this -> x();
	int y =  (int)this -> y();
	adjustAssocs(x, y);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::resizeEvent(QResizeEvent * /*re*/) {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::setLinePos(int x, int y) {
	setX( x );
	setY( y );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::setPositionFromMessage() {
	if (!m_pMessage) {
		return;
	}
	calculateSize();
	setLinePos((int)m_pMessage->x()+5, (int)m_pMessage->y()-height());
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::calculateSize() {
	QFontMetrics fm = QFontMetrics( m_pData -> getFont() );
	int h  = fm.lineSpacing();
	int w = fm.width( getText() );
	setSize( w + 8, h + 4 );//give a small margin
	adjustAssocs( (int)x(), (int)y() );//adjust assoc lines
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::slotMenuSelection(int sel) {
	QString t, newText;
	bool ok;
	int result;
	UMLConcept * c = (UMLConcept *)getUMLObject();
	bool done = false;
	QFont font;
	KLineEditDlg * dlg = 0;

	if( getRole() == tr_RoleName) {
		t = i18n("Enter role name:");
	} else if (getRole() == tr_MultiA || getRole() == tr_MultiB) {
		t = i18n("Enter multiplicity:");
	} else if (getRole() == tr_Floating) {
		t = i18n("Enter new text:");
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
			if(!m_pMessage)
				if(m_pView -> getType() != dt_Collaboration)
					return;
				else {
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
					switch(getRole() ) {
						case tr_RoleName:
							m_pAssoc->setRole(newText);
							break;
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
				if(getRole()  == tr_Floating)
					m_pView -> removeWidget(this);
			done = true;
			calculateSize();
			update();
			break;

		case ListPopupMenu::mt_Change_Font:
			font = getFont();
			if( KFontDialog::getFont( font, false, m_pView ) ) {
				if(getRole()  == tr_Floating || getRole()  == tr_Seq_Message ) {
					setFont( font );
				} else if (m_pAssoc) {
					m_pAssoc->changeFont(font);
				}
			}
			done = true;
			break;
		default:
			break;
	}//end switch
	if(!done)
		UMLWidget::slotMenuSelection(sel);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::setText(QString t) {
	if( ! m_pData )
		return;

	((FloatingTextData*)m_pData)->m_Text= t;
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
	if(m_pView -> m_CurrentCursor != WorkToolBar::tbb_Arrow)
		return;
	if(m_pAssoc) {
		if(getRole()  == tr_Coll_Message || getRole()  == tr_Coll_Message_Self) {

			showOpDlg();
		} else {
			AssocPropDlg dlg(static_cast<QWidget*>(m_pView), m_pAssoc);
			int result = dlg.exec();
			QString rn = dlg.getRoleName(), ma = dlg.getMultiA(), mb = dlg.getMultiB();
			if(result) {
				m_pAssoc -> setRole(rn);
				m_pAssoc -> setMultiA(ma);
				m_pAssoc -> setMultiB(mb);

			}
		}//end if m_Role
	}//end if m_pAssoc
	else if(getRole()  == tr_Seq_Message || getRole()  == tr_Seq_Message_Self) {
		//if on a seq. diagram m_pAssoc won't be set
		//but we still need to show the op. dialog
		showOpDlg();
	} else if (getRole() == tr_Floating) {
		// double clicking on a text line opens the dialog to change the text
		slotMenuSelection(ListPopupMenu::mt_Rename);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingText::showOpDlg() {
	UMLConcept* c = (UMLConcept*)getUMLObject();
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
		if( getRole()  == tr_Seq_Message || getRole()  == tr_Seq_Message_Self) {
			newX = (int)m_pMessage->x() + 5;
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
		if(m_pAssoc) {
			m_pAssoc->calculateRoleTextSegment();
		}
		moveEvent(0);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString FloatingText::getText() {
	//test to make sure not just the ":" between the seq number
	//and the actual m_pMessage
	QString text = "";
	if( !m_pData )
		return text;
	text = ( ( FloatingTextData *)m_pData ) -> m_Text;
	if(getRole()  == tr_Seq_Message || getRole()  == tr_Seq_Message_Self ||
	        getRole()  == tr_Coll_Message || getRole()  == tr_Coll_Message_Self) {
		if( text.length() <= 1 || text == ": " )
			return "";
	}
	return text;
}

bool FloatingText::activate( IDChangeLog* ChangeLog /*= 0 */) {
	bool status = UMLWidget::activate( ChangeLog );
	calculateSize();
	update();
	return status;
}

void FloatingText::synchronizeData() {
	if( getRole()  == tr_Coll_Message || getRole()  == tr_Seq_Message ) {
		setText( getSeqNum().append(": ").append( getOperation() ) );
	}
	UMLWidget::synchronizeData();
}

void FloatingText::setMessage(MessageWidget* m) {
	m_pMessage = m;
	setPositionFromMessage();
}

void FloatingText::setRole(Text_Role Role) {
	dynamic_cast<FloatingTextData *>( m_pData ) -> setRole( Role );
}

Uml::Text_Role FloatingText::getRole() {
	return dynamic_cast<FloatingTextData *>( m_pData ) -> getRole();
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
	if( !m_pData )
		return;
	( ( FloatingTextData *)m_pData ) -> setSeqNum( sn );
	update();
}

QString FloatingText::getSeqNum() {
	if( !m_pData )
		return "";
	return ( ( FloatingTextData *)m_pData ) -> getSeqNum();
}

void FloatingText::setOperation(QString op) {
	if( !m_pData )
		return;
	( ( FloatingTextData *)m_pData ) ->setOperation( op );
}

QString FloatingText::getOperation() {
	if( !m_pData )

		return "";
	return ( ( FloatingTextData *)m_pData ) -> getOperation();
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



