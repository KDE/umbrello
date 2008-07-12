/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "floatingtextwidget.h"

// system includes
#include <QtCore/QRegExp>
#include <QtGui/QPainter>
#include <QtCore/QEvent>
#include <QtGui/QValidator>
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>

// local includes
#include "floatingtextwidgetcontroller.h"
#include "association.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "classifier.h"
#include "listpopupmenu.h"
#include "operation.h"
#include "model_utils.h"
#include "object_factory.h"
#include "messagewidget.h"
#include "dialogs/assocpropdlg.h"
#include "dialogs/selectopdlg.h"
#include "cmds.h"
#include "umlscene.h"

qreal FloatingTextWidget::restrictPositionMin = 0;
qreal FloatingTextWidget::restrictPositionMax = 3000;


FloatingTextWidget::FloatingTextWidget(UMLScene * scene, Uml::Text_Role role,
                                       const QString& text, Uml::IDType id)
    : NewUMLRectWidget(scene, id)//, new FloatingTextWidgetController(this))
{
    init();
    m_Text = text;
    m_Role = role;
    if ( ! UMLApp::app()->getDocument()->loading() ) {
        updateComponentSize();
        setZ( 10 );//make sure always on top.
        update();
    }
}

void FloatingTextWidget::init()
{
    // initialize loaded/saved (i.e. persistent) data
    m_PreText = "";
    m_Text = "";
    m_PostText = "";
    m_Role = Uml::tr_Floating;
    m_baseType = Uml::wt_Text;
    // initialize non-saved (i.e. volatile) data
    m_pLink = NULL;
    setResizable(false);
}

FloatingTextWidget::~FloatingTextWidget()
{
}

void FloatingTextWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
	QPainter &p = *painter;
	qreal offsetX = 0, offsetY = 0;

    qreal w = getWidth();
    qreal h = getHeight();
    p.setFont( NewUMLRectWidget::getFont() );
    QColor textColor(50, 50, 50);
    p.setPen(textColor);
    p.drawText( offsetX , offsetY,w,h, Qt::AlignCenter, getDisplayText() );
    if(isSelected())
        drawSelected(&p, offsetX, offsetY);
}

void FloatingTextWidget::resizeEvent(QResizeEvent * re)
{
    Q_UNUSED(re);
}

QSizeF FloatingTextWidget::calculateSize()
{
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    qreal h = fm.lineSpacing();
    qreal w = fm.width( getDisplayText() );
    return QSizeF(w + 8, h + 4);  // give a small margin
}

void FloatingTextWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::Menu_Type sel = m_pMenu->getMenuType(action);
    switch(sel) {
    case ListPopupMenu::mt_Properties:
        showProperties();
        break;

    case ListPopupMenu::mt_Delete:
        umlScene()->removeWidget(this);
        break;

    case ListPopupMenu::mt_Operation:
        {
            if (m_pLink == NULL) {
                uDebug() << "mt_Operation: " << "m_pLink is NULL";
                return;
            }
            UMLClassifier* c = m_pLink->getOperationOwner();
            if (c == NULL) {
                bool ok = false;
                QString opText = KInputDialog::getText(i18nc("operation name", "Name"),
                                                       i18n("Enter operation name:"),
                                                       getText(), &ok, umlScene()->activeView());
                if (ok)
                    m_pLink->setCustomOpText(opText);
                return;
            }
            UMLClassifierListItem* umlObj = Object_Factory::createChildObject(c, Uml::ot_Operation);
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
            if( KFontDialog::getFont( font, false, umlScene()->activeView() ) ) {
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
        NewUMLRectWidget::slotMenuSelection(action);
        break;
    }//end switch
}

void FloatingTextWidget::handleRename()
{
    QRegExpValidator v(QRegExp(".*"), 0);
    QString t;
    if( m_Role == Uml::tr_RoleAName || m_Role == Uml::tr_RoleBName ) {
        t = i18n("Enter role name:");
    } else if (m_Role == Uml::tr_MultiA || m_Role == Uml::tr_MultiB) {
        t = i18n("Enter multiplicity:");
        /*
        // NO! shouldn't be allowed
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
    QString newText = KInputDialog::getText(i18n("Rename"), t, getText(), &ok,
                                            umlScene()->activeView(), &v);
    if (!ok || newText == getText())
        return;

    UMLApp::app()->executeCommand(new Uml::CmdHandleRename(this,newText));
}

void FloatingTextWidget::changeName(const QString& newText)
{
    if (m_pLink && !isTextValid(newText)) {
        AssociationWidget *assoc = dynamic_cast<AssociationWidget*>(m_pLink);
        if (assoc) {
            switch (m_Role) {
              case Uml::tr_MultiA:
                assoc->setMulti(QString(), Uml::A);
                break;
              case Uml::tr_MultiB:
                assoc->setMulti(QString(), Uml::B);
                break;
              case Uml::tr_RoleAName:
                assoc->setRoleName(QString(), Uml::A);
                break;
              case Uml::tr_RoleBName:
                assoc->setRoleName(QString(), Uml::B);
                break;
              case Uml::tr_ChangeA:
                assoc->setChangeability(Uml::chg_Changeable, Uml::A);
                break;
              case Uml::tr_ChangeB:
                assoc->setChangeability(Uml::chg_Changeable, Uml::B);
                break;
              default:
                assoc->setName(QString());
                break;
            }
        } else {
            MessageWidget *msg = dynamic_cast<MessageWidget*>(m_pLink);
            if (msg) {
                msg->setName(QString());
                umlScene()->removeWidget(this);
            }
        }
        return;
    }
    if (m_pLink && m_Role != Uml::tr_Seq_Message && m_Role != Uml::tr_Seq_Message_Self) {
        m_pLink->setText(this, newText);
    } else {
        setText( newText );
        UMLApp::app()->getDocument()->setModified(true);
    }
    setVisible( true );
    updateComponentSize();
    update();
}

void FloatingTextWidget::setTextcmd(const QString &t)
{
    UMLApp::app()->executeCommand(new Uml::CmdSetTxt(this,t));
}

void FloatingTextWidget::setText(const QString &t)
{
    if (m_Role == Uml::tr_Seq_Message || m_Role == Uml::tr_Seq_Message_Self) {
        QString seqNum, op;
        m_pLink->getSeqNumAndOp(seqNum, op);
        if (seqNum.length() > 0 || op.length() > 0) {
            if (! umlScene()->getShowOpSig())
                op.replace( QRegExp("\\(.*\\)"), "()" );
            m_Text = seqNum.append(": ").append( op );
        } else
            m_Text = t;
    } else
        m_Text = t;
    updateComponentSize();
    update();
}

void FloatingTextWidget::setPreText (const QString &t)
{
    m_PreText = t;
    updateComponentSize();
    update();
}

void FloatingTextWidget::setPostText(const QString &t)
{
    m_PostText = t;
    updateComponentSize();
    update();
}

void FloatingTextWidget::changeTextDlg()
{
    bool ok = false;
    QString newText = KInputDialog::getText(i18n("Change Text"), i18n("Enter new text:"), getText(), &ok, umlScene()->activeView());

    if(ok && newText != getText() && isTextValid(newText)) {
        setText( newText );
        setVisible( ( getText().length() > 0 ) );
        updateComponentSize();
        update();
    }
    if(!isTextValid(newText))
        hide();
}

void FloatingTextWidget::showOpDlg()
{
    if (m_pLink == NULL) {
        uError() << "m_pLink is NULL";
        return;
    }
    QString seqNum, opText;
    UMLClassifier* c = m_pLink->getSeqNumAndOp(seqNum, opText);
    if (c == NULL) {
        uError() << "m_pLink->getSeqNumAndOp() returns a NULL classifier";
        return;
    }

    SelectOpDlg selectDlg(umlScene()->activeView(), c);
    selectDlg.setSeqNumber( seqNum );
    if (m_pLink->getOperation() == NULL) {
        selectDlg.setCustomOp( opText );
    } else {
        selectDlg.setClassOp( opText );
    }
    int result = selectDlg.exec();
    if(!result) {
        return;
    }
    seqNum = selectDlg.getSeqNumber();
    opText = selectDlg.getOpText();
    if (selectDlg.isClassOp()) {
        Model_Utils::OpDescriptor od;
        Model_Utils::Parse_Status st = Model_Utils::parseOperation(opText, od, c);
        if (st == Model_Utils::PS_OK) {
            UMLClassifierList selfAndAncestors = c->findSuperClassConcepts();
            selfAndAncestors.prepend(c);
            UMLOperation *op = NULL;
            foreach (UMLClassifier *cl , selfAndAncestors) {
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
            opText = QString();
        } else {
            m_pLink->setOperation(NULL);
        }
    } else {
        m_pLink->setOperation(NULL);
    }
    m_pLink->setSeqNumAndOp(seqNum, opText);
    setMessageText();
}

QString FloatingTextWidget::getPreText() const
{
    return m_PreText;
}

QString FloatingTextWidget::getPostText() const
{
    return m_PostText;
}

QString FloatingTextWidget::getText() const
{
    //test to make sure not just the ":" between the seq number
    //and the actual message widget
    // hmm. this section looks like it could have been avoided by using pre-, post- text
    // instead of storing in the main body of the text -b.t.
    if(m_Role == Uml::tr_Seq_Message || m_Role == Uml::tr_Seq_Message_Self ||
            m_Role == Uml::tr_Coll_Message || m_Role == Uml::tr_Coll_Message_Self) {
        if( m_Text.length() <= 1 || m_Text == ": " )
            return QString();
    }
    return m_Text;
}

QString FloatingTextWidget::getDisplayText() const
{
    QString displayText = m_Text;
    displayText.prepend(m_PreText);
    displayText.append(m_PostText);
    return displayText;
}

bool FloatingTextWidget::activate( IDChangeLog* ChangeLog /*= 0 */)
{
    if (! NewUMLRectWidget::activate(ChangeLog))
        return false;
    update();
    return true;
}

void FloatingTextWidget::setLink(LinkWidget * l)
{
    m_pLink = l;
}

LinkWidget * FloatingTextWidget::getLink()
{
    return m_pLink;
}

void FloatingTextWidget::setRole(Uml::Text_Role role)
{
    m_Role = role;
}

Uml::Text_Role FloatingTextWidget::getRole() const
{
    return m_Role;
}

bool FloatingTextWidget::isTextValid( const QString &text )
{
    int length = text.length();
    if(length < 1)
        return false;
    for(int i=0;i<length;i++)
        if(!text.at(i).isSpace())
            return true;
    return false;
}

void FloatingTextWidget::showProperties()
{
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

void FloatingTextWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement textElement = qDoc.createElement( "floatingtext" );
    NewUMLRectWidget::saveToXMI( qDoc, textElement );
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

bool FloatingTextWidget::loadFromXMI( QDomElement & qElement )
{
    if( !NewUMLRectWidget::loadFromXMI( qElement ) )
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

void FloatingTextWidget::setMessageText()
{
    if (m_pLink)
        m_pLink->setMessageText(this);
    setVisible(getText().length() > 0);
    updateComponentSize();
}


#include "floatingtextwidget.moc"

