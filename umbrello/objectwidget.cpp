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

// own header file
#include "objectwidget.h"

// system includes
#include <qpainter.h>
#include <qvalidator.h>
#include <qevent.h>
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>

// local includes
#include "objectwidgetcontroller.h"
#include "seqlinewidget.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "umlobject.h"
#include "listpopupmenu.h"
#include "docwindow.h"
#include "dialogs/classpropdlg.h"
#include "umlscene.h"

/**
 * The number of pixels margin between the lowest message
 * and the bottom of the vertical line
 */
static const int sequenceLineMargin = 20;

ObjectWidget::ObjectWidget(UMLScene * scene, UMLObject *o, Uml::IDType lid)
        : NewUMLRectWidget(scene, o) {
    init();
    if( lid != Uml::id_None )
        m_nLocalID = lid;
    //updateComponentSize();
    //                  Doing this during loadFromXMI() gives futile updates.
    //                  Instead, it is done afterwards by NewUMLRectWidget::activate()
}

void ObjectWidget::init() {
    NewUMLRectWidget::setBaseType(Uml::wt_Object);
    m_nLocalID = Uml::id_None;
    m_InstanceName = "";
    m_bMultipleInstance = false;
    m_bDrawAsActor = false;
    m_bShowDestruction = false;
    if( umlScene() != NULL && umlScene()->getType() == Uml::dt_Sequence ) {
        m_pLine = new SeqLineWidget( umlScene(), this );

        //Sets specific widget controller for sequence diagrams
        // delete m_widgetController;
        // m_widgetController = 0;// [PORT] new ObjectWidgetController(this);
    } else {
        m_pLine = NULL;
    }
}

ObjectWidget::~ObjectWidget() {}

void ObjectWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	QPainter &p = *painter;
	qreal offsetX = 0, offsetY = 0;

    if ( m_bDrawAsActor )
        drawActor( p, offsetX, offsetY );
    else
        drawObject( p, offsetX, offsetY );

    setPenFromSettings(p);
    if(isSelected())
        drawSelected(&p, offsetX, offsetY);
}

void ObjectWidget::slotMenuSelection(QAction* action) {
    ListPopupMenu::Menu_Type sel = m_pMenu->getMenuType(action);
    switch(sel) {
    case ListPopupMenu::mt_Rename_Object:
        {
            bool ok;
            QRegExpValidator* validator = new QRegExpValidator(QRegExp(".*"), 0);
            QString name = KInputDialog::getText
                   (i18n("Rename Object"),
                    i18n("Enter object name:"),
                    m_InstanceName,
                    &ok,
                    umlScene()->activeView(),
                    validator);
            if (ok) {
                m_InstanceName = name;
                updateComponentSize();
                moveEvent( 0 );
                update();
                UMLApp::app()->getDocument()->setModified(true);
            }
            delete validator;
            break;
        }
    case ListPopupMenu::mt_Properties:
        showProperties();
        updateComponentSize();
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
        NewUMLRectWidget::slotMenuSelection(action);
        break;
    }
}

QSizeF ObjectWidget::calculateSize() {
    int width, height;
    const QFontMetrics &fm = getFontMetrics(FT_UNDERLINE);
    const int fontHeight  = fm.lineSpacing();
    const QString t = m_InstanceName + " : " + umlObject()->getName();
    const int textWidth = fm.width(t);
    if ( m_bDrawAsActor ) {
        width = textWidth > A_WIDTH?textWidth:A_WIDTH;
        height = A_HEIGHT + fontHeight + A_MARGIN;
        width += A_MARGIN * 2;
    } else {
        width = textWidth > O_WIDTH?textWidth:O_WIDTH;
        height = fontHeight + O_MARGIN * 2;
        width += O_MARGIN * 2;
        if (m_bMultipleInstance) {
            width += 10;
            height += 10;
        }
    }//end else drawasactor

    return QSizeF(width, height);
}

void ObjectWidget::setDrawAsActor( bool drawAsActor ) {
    m_bDrawAsActor = drawAsActor;
    updateComponentSize();
}

void ObjectWidget::setMultipleInstance(bool multiple) {
    //make sure only calling this in relation to an object on a collab. diagram
    if(umlScene()->getType() != Uml::dt_Collaboration)
        return;
    m_bMultipleInstance = multiple;
    updateComponentSize();
    update();
}

bool ObjectWidget::activate(IDChangeLog* ChangeLog /*= 0*/) {
    if (! NewUMLRectWidget::activate(ChangeLog))
        return false;
    if (m_bShowDestruction && m_pLine)
        m_pLine->setupDestructionBox();
    moveEvent(0);
    return true;
}

void ObjectWidget::setX(qreal x) {
    NewUMLRectWidget::setX(x);
    moveEvent(0);
}

void ObjectWidget::setY(qreal y) {
    NewUMLRectWidget::setY(y);
    moveEvent(0);
}

void ObjectWidget::moveEvent(QMoveEvent *m) {
    Q_UNUSED(m);
    // [PORT] emit sigWidgetMoved( m_nLocalID );
    if (m_pLine) {
        const qreal x = getX();    // for debugging: gdb has a problem evaluating getX() etc
        const qreal w = getWidth();
        const qreal y = getY();
        const qreal h = getHeight();
        m_pLine->setStartPoint(x + w / 2, y + h);
    }
}

void ObjectWidget::slotColorChanged(Uml::IDType /*viewID*/) {
    NewUMLRectWidget::setFillColour( umlScene()->getFillColor() );
    NewUMLRectWidget::setLineColor( umlScene()->getLineColor() );

    if( m_pLine)
        m_pLine->setPen( QPen( NewUMLRectWidget::getLineColor(), NewUMLRectWidget::getLineWidth(), Qt::DashLine ) );
}

void ObjectWidget::cleanup() {

    NewUMLRectWidget::cleanup();
    if( m_pLine ) {
        m_pLine->cleanup();
        delete m_pLine;
    }
}

void ObjectWidget::showProperties() {
    DocWindow *docwindow = UMLApp::app()->getDocWindow();
    docwindow->updateDocumentation(false);
    ClassPropDlg *dlg = new ClassPropDlg((QWidget*)UMLApp::app(), this);
    if (dlg->exec()) {
        docwindow->showDocumentation(this, true);
        UMLApp::app()->getDocument()->setModified(true);
    }
    dlg->close();
}

void ObjectWidget::drawObject(QPainter & p, int offsetX, int offsetY) {

    QFont oldFont = p.font();
    QFont font = NewUMLRectWidget::getFont();
    font.setUnderline( true );
    p.setFont( font );

    setPenFromSettings(p);
    if(NewUMLRectWidget::getUseFillColour())
        p.setBrush(NewUMLRectWidget::getFillColour());
    else {
        // [PORT]
        // p.setBrush( umlScene()->viewport()->palette().color(QPalette::Background) );
    }
    const int w = getWidth();
    const int h = getHeight();

    const QString t = m_InstanceName + " : " + umlObject()->getName();
    int multiInstOfst = 0;
    if ( m_bMultipleInstance ) {
        p.drawRect(offsetX + 10, offsetY + 10, w - 10, h - 10);
        p.drawRect(offsetX + 5, offsetY + 5, w - 10, h - 10);
        multiInstOfst = 10;
    }
    p.drawRect(offsetX, offsetY, w - multiInstOfst, h - multiInstOfst);
    p.setPen(QPen(Qt::black));
    p.drawText(offsetX + O_MARGIN, offsetY + O_MARGIN,
               w - O_MARGIN * 2 - multiInstOfst, h - O_MARGIN * 2 - multiInstOfst,
               Qt::AlignCenter, t);

    p.setFont( oldFont );
}

void ObjectWidget::drawActor(QPainter & p, int offsetX, int offsetY) {
    const QFontMetrics &fm = getFontMetrics(FT_UNDERLINE);

    setPenFromSettings(p);
    if ( NewUMLRectWidget::getUseFillColour() )
        p.setBrush( NewUMLRectWidget::getFillColour() );
    const int w = getWidth();
    const int textStartY = A_HEIGHT + A_MARGIN;
    const int fontHeight  = fm.lineSpacing();

    const int middleX = offsetX + w / 2;
    const int thirdH = A_HEIGHT / 3;

    //draw actor
    p.drawEllipse(middleX - A_WIDTH / 2, offsetY,  A_WIDTH, thirdH);//head
    p.drawLine(middleX, offsetY + thirdH, middleX, offsetY + thirdH * 2);//body
    p.drawLine(middleX, offsetY + 2 * thirdH,
               middleX - A_WIDTH / 2, offsetY + A_HEIGHT);//left leg
    p.drawLine(middleX, offsetY +  2 * thirdH,
               middleX + A_WIDTH / 2, offsetY + A_HEIGHT);//right leg
    p.drawLine(middleX - A_WIDTH / 2, offsetY + thirdH + thirdH / 2,
               middleX + A_WIDTH / 2, offsetY + thirdH + thirdH / 2);//arms
    //draw text
    p.setPen(QPen(Qt::black));
    QString t = m_InstanceName + " : " + umlObject()->getName();
    p.drawText(offsetX + A_MARGIN, offsetY + textStartY,
               w - A_MARGIN * 2, fontHeight, Qt::AlignCenter, t);
}

void ObjectWidget::tabUp() {
    int newY = getY() - getHeight();
    if (newY < topMargin())
        newY = topMargin();
    setY( newY );
    moveEvent( 0 );
    adjustAssocs( getX(), newY);
}

void ObjectWidget::tabDown() {
    int newY = getY() + getHeight();
    setY( newY );
    moveEvent( 0 );
    adjustAssocs( getX(), newY);
}

int ObjectWidget::topMargin() {
    return 80 - getHeight();
}

bool ObjectWidget::canTabUp() {
    int y = getY();
    return (y > topMargin());
}

void ObjectWidget::setShowDestruction( bool bShow ) {
    m_bShowDestruction = bShow;
    if( m_pLine )
        m_pLine->setupDestructionBox();
}

void ObjectWidget::setEndLine(int yPosition) {
    m_pLine->setEndOfLine(yPosition);
}

int ObjectWidget::getEndLineY() {
    int y = this->getY() + getHeight();
    if( m_pLine)
        y += m_pLine->getLineLength();
    if ( m_bShowDestruction )
        y += 10;
    return y;
}

void ObjectWidget::messageAdded(MessageWidget* message) {
    if ( messageWidgetList.count(message) ) {
        uError() << message->getName() << ": duplicate entry !" << endl;
        return ;
    }
    messageWidgetList.append(message);
}

void ObjectWidget::messageRemoved(MessageWidget* message) {
    if ( messageWidgetList.removeAll(message) == false ) {
        uError() << message->getName() << ": missing entry !"
                  << endl;
        return ;
    }
}

void ObjectWidget::slotMessageMoved() {

    int lowestMessage = 0;
    foreach ( MessageWidget* message, messageWidgetList ) {
        int messageHeight = message->getY() + message->getHeight();
        if (lowestMessage < messageHeight) {
            lowestMessage = messageHeight;
        }
    }
    m_pLine->setEndOfLine(lowestMessage + sequenceLineMargin);
}

bool ObjectWidget::messageOverlap(int y, MessageWidget* messageWidget) {

    foreach ( MessageWidget* message , messageWidgetList ) {
        const int msgY = message->getY();
        const int msgHeight = msgY + message->getHeight();
        if (y >= msgY && y <= msgHeight && message != messageWidget) {
            return true;
        }
    }
    return false;
}

SeqLineWidget *ObjectWidget::getSeqLine() {
    return m_pLine;
}

void ObjectWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement objectElement = qDoc.createElement( "objectwidget" );
    NewUMLRectWidget::saveToXMI( qDoc, objectElement );
    objectElement.setAttribute( "instancename", m_InstanceName );
    objectElement.setAttribute( "drawasactor", m_bDrawAsActor );
    objectElement.setAttribute( "multipleinstance", m_bMultipleInstance );
    objectElement.setAttribute( "localid", ID2STR(m_nLocalID) );
    objectElement.setAttribute( "decon", m_bShowDestruction );
    qElement.appendChild( objectElement );
}

bool ObjectWidget::loadFromXMI( QDomElement & qElement ) {
    if( !NewUMLRectWidget::loadFromXMI( qElement ) )
        return false;
    m_InstanceName = qElement.attribute( "instancename", "" );
    QString draw = qElement.attribute( "drawasactor", "0" );
    QString multi = qElement.attribute( "multipleinstance", "0" );
    QString localid = qElement.attribute( "localid", "0" );
    QString decon = qElement.attribute( "decon", "0" );

    m_bDrawAsActor = (bool)draw.toInt();
    m_bMultipleInstance = (bool)multi.toInt();
    m_nLocalID = STR2ID(localid);
    m_bShowDestruction = (bool)decon.toInt();
    return true;

}

#include "objectwidget.moc"
