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
#include "floatingtext.h"

// system includes
#include <qregexp.h>
#include <qpainter.h>
#include <klocale.h>
#include <kdebug.h>

// local includes
#include "association.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "classifier.h"
#include "listpopupmenu.h"
#include "operation.h"
#include "model_utils.h"
#include "inputdialog.h"
#include "dialogs/assocpropdlg.h"
#include "dialogs/selectopdlg.h"

FloatingText::FloatingText(UMLView * view, Uml::Text_Role role, QString text, Uml::IDType id)
  : UMLWidget(view, id)
{
	init();
	m_Text = text;
	m_Role = role;
	if ( ! UMLApp::app()->getDocument()->loading() ) {
		calculateSize();
		setZ( 10 );//make sure always on top.
		update();
	}
}

void FloatingText::init() {
	// initialize loaded/saved (i.e. persistent) data
	m_PreText = "";
	m_Text = "";
	m_PostText = "";
	m_Role = Uml::tr_Floating;
	m_Type = Uml::wt_Text;
	// initialize non-saved (i.e. volatile) data
	m_pLink = NULL;
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

void FloatingText::resizeEvent(QResizeEvent * /*re*/) {}

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

	case ListPopupMenu::mt_Delete:
		m_pView -> removeWidget(this);
		break;

	case ListPopupMenu::mt_Operation:
		{
			kdDebug() << "FloatingText::slotMenuSelection(mt_Operation) is called."
				  << endl;
			if (m_pLink == NULL) {
				kdDebug() << "FloatingText::slotMenuSelection(mt_Operation): "
					  << "m_pLink is NULL" << endl;
				return;
			}
			UMLClassifier* c = m_pLink->getOperationOwner();
			if (c == NULL) {
				bool ok = false;
				QString opText = KInputDialog::getText(i18n("Name"),
								       i18n("Enter operation name:"),
								       getText(), &ok, m_pView);
				if (ok)
					m_pLink->setCustomOpText(opText);
				return;
			}
			UMLObject* umlObj = UMLApp::app()->getDocument()->createChildObject(c, Uml::ot_Operation);
			if (umlObj) {
				UMLOperation* newOperation = static_cast<UMLOperation*>( umlObj );
				m_pLink->setOperation(newOperation);
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
				if( m_Role == Uml::tr_Floating || m_Role == Uml::tr_Seq_Message ) {
					setFont( font );
				} else if (m_pLink) {
					m_pLink->lwSetFont(font);
				}
			}
		}
		break;

	case ListPopupMenu::mt_Reset_Label_Positions:
		if (m_pLink)
			m_pLink->resetTextPositions();
		break;

	default:
		UMLWidget::slotMenuSelection(sel);
		break;
	}//end switch
}

void FloatingText::handleRename() {
	QString t;
	if( m_Role == Uml::tr_RoleAName || m_Role == Uml::tr_RoleBName ) {
		t = i18n("Enter role name:");
	} else if (m_Role == Uml::tr_MultiA || m_Role == Uml::tr_MultiB) {
		t = i18n("Enter multiplicity:");
		/*
		// NO! shouldnt be allowed
		} else if( m_Role == Uml::tr_ChangeA || m_Role == Uml::tr_ChangeB ) {
		t = i18n("Enter changeability");
		*/
	} else if (m_Role == Uml::tr_Name) {
		t = i18n("Enter association name:");
	} else if (m_Role == Uml::tr_Floating) {
		t = i18n("Enter new text:");
	} else {
		t = i18n("ERROR");
	}
	bool ok = false;
	QString newText = KInputDialog::getText(i18n("Rename"), t, getText(), &ok, m_pView);
	if (!ok)
		return;
	bool valid = isTextValid(newText);
	if (!valid || newText == getText()) {
		if (!valid && m_Role == Uml::tr_Floating)
			m_pView -> removeWidget(this);
		calculateSize();
		update();
		return;
	}
	if (m_pLink && m_Role != Uml::tr_Seq_Message && m_Role != Uml::tr_Seq_Message_Self) {
		m_pLink->setText(this, newText);
	} else {
		setText( newText );
		UMLApp::app()->getDocument()->setModified(true);
	}
	setVisible( true );
	calculateSize();
	update();
}

void FloatingText::setText(const QString &t) {
	if (m_Role == Uml::tr_Seq_Message || m_Role == Uml::tr_Seq_Message_Self) {
		QString seqNum, op;
		m_pLink->getSeqNumAndOp(this, seqNum, op);
		if (seqNum.length() > 0 || op.length() > 0)
			m_Text = seqNum.append(": ").append( op );
		else
			m_Text = t;
	} else
		m_Text = t;
	calculateSize();
	update();
}

void FloatingText::setPreText (const QString &t)
{
	m_PreText = t;
	calculateSize();
	update();
}

void FloatingText::setPostText(const QString &t) {
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
	if (m_Role == Uml::tr_Coll_Message || m_Role == Uml::tr_Coll_Message_Self ||
	    m_Role == Uml::tr_Seq_Message || m_Role == Uml::tr_Seq_Message_Self) {
		showOpDlg();
	} else if (m_Role == Uml::tr_Floating) {
		// double clicking on a text line opens the dialog to change the text
		handleRename();
	} else if (m_pLink) {
		m_pLink->showDialog();
	}
}

void FloatingText::showOpDlg() {
	if (m_pLink == NULL) {
		kdError() << "FloatingText::showOpDlg: m_pLink is NULL" << endl;
		return;
	}
	QString seqNum, opText;
	UMLClassifier* c = m_pLink->getSeqNumAndOp(this, seqNum, opText);
	if (c == NULL) {
		kdError() << "FloatingText::showOpDlg: "
			  << "m_pLink->getSeqNumAndOp() returns a NULL classifier"
			  << endl;
		return;
	}

	SelectOpDlg selectDlg(m_pView, c);
	selectDlg.setSeqNumber( seqNum );
	if (m_pLink->getOperation() == NULL)
		selectDlg.setCustomOp( opText );
	int result = selectDlg.exec();
	if(!result) {
		return;
	}
	seqNum = selectDlg.getSeqNumber();
	opText = selectDlg.getOpText();
	Umbrello::OpDescriptor od;
	Umbrello::Parse_Status st = Umbrello::parseOperation(opText, od, c);
	if (st == Umbrello::PS_OK) {
		UMLClassifierList selfAndAncestors = c->findSuperClassConcepts();
		selfAndAncestors.prepend(c);
		UMLOperation *op = NULL;
		for (UMLClassifier *cl = selfAndAncestors.first(); cl; cl = selfAndAncestors.next()) {
			op = cl->findOperation(od.m_name, od.m_args);
			if (op != NULL)
				break;
		}
		if (op == NULL) {
			// The op does not yet exist. Create a new one.
			UMLObject *o = c->createOperation(od.m_name, NULL, &od.m_args);
			op = static_cast<UMLOperation*>(o);
		}
		if (od.m_pReturnType)
			op->setType(od.m_pReturnType);
		m_pLink->setOperation(op);
		opText = QString::null;
	} else {
		m_pLink->setOperation(NULL);
	}
	m_pLink->setSeqNumAndOp(seqNum, opText);
	setMessageText();
}

void FloatingText::mouseMoveEvent(QMouseEvent* me) {
	if (!m_bMouseDown && me->button() != LeftButton)
		return;
	if (m_Role == Uml::tr_Seq_Message_Self)
		return;
	QPoint newPosition = doMouseMove(me);
	int newX = newPosition.x();
	int newY = newPosition.y();

	//implement specific rules for a sequence diagram
	if (m_Role == Uml::tr_Seq_Message || m_Role == Uml::tr_Seq_Message_Self) {
		m_pLink->constrainTextPos(newX, newY, width(), height(), m_Role);
	}
	m_nOldX = newX;
	m_nOldY = newY;
	setX( newX );
	setY( newY );
	if (m_pLink)
		m_pLink->calculateNameTextSegment();
	m_pView->resizeCanvasToItems();
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
	if(m_Role == Uml::tr_Seq_Message || m_Role == Uml::tr_Seq_Message_Self ||
	   m_Role == Uml::tr_Coll_Message || m_Role == Uml::tr_Coll_Message_Self) {
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

void FloatingText::setLink(LinkWidget * l) {
	m_pLink = l;
}

LinkWidget * FloatingText::getLink() {
	return m_pLink;
}

void FloatingText::setRole(Uml::Text_Role role) {
	m_Role = role;
}

Uml::Text_Role FloatingText::getRole() const {
	return m_Role;
}

bool FloatingText::isTextValid( const QString &text ) {
	int length = text.length();
	if(length < 1)
		return false;
	for(int i=0;i<length;i++)
		if(!text.at(i).isSpace())
			return true;
	return false;
}

void FloatingText::setSelected(bool _select) {
	UMLWidget::setSelected( _select );
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
	bool isDummy = (m_Text.isEmpty() && m_PreText.isEmpty() && m_PostText.isEmpty());
	return !isDummy;
}

void FloatingText::setMessageText() {
	if (m_pLink)
		m_pLink->setMessageText(this);
	setVisible(getText().length() > 0);
	calculateSize();
}

