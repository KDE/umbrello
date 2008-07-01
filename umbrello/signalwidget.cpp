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
#include "signalwidget.h"

// qt includes
#include <qevent.h>
#include <QPolygon>

// kde includes
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>

// app includes
#include "uml.h"
#include "umlnamespace.h"
#include "umldoc.h"
#include "uniqueid.h"
#include "docwindow.h"
#include "umlwidget.h"
#include "umlview.h"
#include "floatingtextwidget.h"
#include "linkwidget.h"

// #include "dialogs/signaldialog.h"
#include "listpopupmenu.h"
#include "umlscene.h"

SignalWidget::SignalWidget(UMLScene * scene, SignalType signalType, Uml::IDType id)
        : NewUMLRectWidget(scene, id)
{
    NewUMLRectWidget::setBaseType(Uml::wt_Signal);
    m_SignalType = signalType;
    updateComponentSize();
    m_pName = NULL;
    if (signalType == SignalWidget::Time) {
        m_pName = new FloatingTextWidget(scene, Uml::tr_Floating,"");
        scene->setupNewWidget(m_pName);
        m_pName->setX(0);
        m_pName->setY(0);
    }
}

SignalWidget::~SignalWidget() {}

void SignalWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
	QPainter &p = *painter;
	qreal offsetX = 0, offsetY = 0;

    setPenFromSettings(p);
    const qreal w = getWidth();
    const qreal h = getHeight();
    QPolygon a;
    switch (m_SignalType)
    {
    case Send :
        if(NewUMLRectWidget::getUseFillColour())
            p.setBrush(NewUMLRectWidget::getFillColour());
        {

            a.setPoints( 5, offsetX           ,offsetY,
                            offsetX + (w*2)/3 ,offsetY,
                            offsetX + w       ,(h/2)+offsetY,
                            offsetX + (w*2)/3 ,h+offsetY,
                            offsetX           ,h+offsetY );
            p.drawPolygon( a );
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const qreal fontHeight  = fm.lineSpacing();
            qreal textStartY = (h / 2) - (fontHeight / 2);

            p.setPen(Qt::black);
            QFont font = NewUMLRectWidget::getFont();
            font.setBold( false );
            p.setFont( font );
            p.drawText(offsetX + SIGNAL_MARGIN, offsetY + textStartY,
                           w - SIGNAL_MARGIN * 2, fontHeight,
                           Qt::AlignCenter, getName());
            setPenFromSettings(p);
        }
        break;
    case Accept :
        if(NewUMLRectWidget::getUseFillColour())
            p.setBrush(NewUMLRectWidget::getFillColour());
        {
            a.setPoints( 5, offsetX ,      offsetY,
                            offsetX + w/3, (h/2)+offsetY,
                            offsetX ,      h+offsetY,
                            offsetX + w,   h+offsetY,
                            offsetX + w,   offsetY );

            p.drawPolygon( a );
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            const qreal fontHeight  = fm.lineSpacing();
            qreal textStartY = (h / 2) - (fontHeight / 2);

            p.setPen(Qt::black);
            QFont font = NewUMLRectWidget::getFont();
            font.setBold( false );
            p.setFont( font );
            p.drawText(offsetX + SIGNAL_MARGIN, offsetY + textStartY,
                           w - SIGNAL_MARGIN * 2 + (w/3), fontHeight,
                           Qt::AlignCenter, getName());
            setPenFromSettings(p);
        }
        break;
    case Time :
        if(NewUMLRectWidget::getUseFillColour())
            p.setBrush(NewUMLRectWidget::getFillColour());
        {
            a.setPoints( 4, offsetX ,    offsetY,
                            offsetX + w, offsetY+h,
                            offsetX ,    offsetY+h,
                            offsetX + w, offsetY);

            p.drawPolygon( a );
            //const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
            //const qreal fontHeight  = fm.lineSpacing();
            //qreal textStartY = (h / 2) - (fontHeight / 2);
            p.setPen(Qt::black);
            QFont font = NewUMLRectWidget::getFont();
            font.setBold( false );
            p.setFont( font );

            setPenFromSettings(p);
        }
        if (m_pName->getX() == 0 && m_pName->getY() == 0) {
            //the floating text has not been linked with the signal
            m_pName->setX(offsetX + w/2 - m_pName->getWidth()/2);
            m_pName->setY(offsetY + h);
        }
        m_pName->setVisible( ( m_pName->getText().length() > 0 ) );
        m_pName->updateComponentSize();

        break;
    default:
        uWarning() << "Unknown signal type:" << m_SignalType;
        break;
    }
    if(isSelected())
        drawSelected(&p, offsetX, offsetY);
}


void SignalWidget::setX(qreal newX) {
    m_oldX = getX();
    NewUMLRectWidget::setX(newX);
}

void SignalWidget::setY(qreal newY) {
    m_oldY = getY();
    NewUMLRectWidget::setY(newY);
}

QSizeF SignalWidget::calculateSize() {
        qreal width = SIGNAL_WIDTH, height = SIGNAL_HEIGHT;
        const QFontMetrics &fm = getFontMetrics(FT_BOLD);
        const qreal fontHeight  = fm.lineSpacing();
        qreal textWidth = fm.width(getName());

        if (m_SignalType == Accept)
             textWidth = qreal((float)textWidth * 1.3f);
        height  = fontHeight;
        if (m_SignalType != Time)
        {
              width   = textWidth > SIGNAL_WIDTH?textWidth:SIGNAL_WIDTH;
              height  = height > SIGNAL_HEIGHT?height:SIGNAL_HEIGHT;
        }
        width  += SIGNAL_MARGIN * 2;
        height += SIGNAL_MARGIN * 2;

    return QSizeF(width, height);
}

void SignalWidget::setName(const QString &strName) {
    m_Text = strName;
    updateComponentSize();
    if (getSignalType() == SignalWidget::Time) {
        m_pName->setText(m_Text);
    }
}

QString SignalWidget::getName() const {
    return m_Text;
}

SignalWidget::SignalType SignalWidget::getSignalType() const {
    return m_SignalType;
}

void SignalWidget::setSignalType( SignalType signalType ) {
    m_SignalType = signalType;
}

void SignalWidget::slotMenuSelection(QAction* action) {
    bool ok = false;
    QString name = m_Text;

    ListPopupMenu::Menu_Type sel = m_pMenu->getMenuType(action);
    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        name = KInputDialog::getText( i18n("Enter signal name"), i18n("Enter the signal name :"), m_Text, &ok );
        if( ok && name.length() > 0 )
            setName(name);
        break;

    default:
        NewUMLRectWidget::slotMenuSelection(action);
    }
}


void SignalWidget::showProperties() {}

void SignalWidget::mouseMoveEvent(QGraphicsSceneMouseEvent* me) {
    NewUMLRectWidget::mouseMoveEvent(me);
    qreal diffX = m_oldX - getX();
    qreal diffY = m_oldY - getY();
    if (m_pName!=NULL) {
        m_pName->setX(m_pName->getX() - diffX);
        m_pName->setY(m_pName->getY() - diffY);
    }
}

void SignalWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement signalElement = qDoc.createElement( "signalwidget" );
    NewUMLRectWidget::saveToXMI( qDoc, signalElement );
    signalElement.setAttribute( "signalname", m_Text );
    signalElement.setAttribute( "documentation", documentation() );
    signalElement.setAttribute( "signaltype", m_SignalType );
    if (m_pName && !m_pName->getText().isEmpty()) {
        signalElement.setAttribute( "textid", ID2STR(m_pName->getID()) );
        m_pName->saveToXMI( qDoc, signalElement );
    }
    qElement.appendChild( signalElement );
}

bool SignalWidget::loadFromXMI( QDomElement & qElement ) {
    if( !NewUMLRectWidget::loadFromXMI( qElement ) )
        return false;
    m_Text = qElement.attribute( "signalname", "" );
    setDocumentation(qElement.attribute( "documentation", "" ));
    QString type = qElement.attribute( "signaltype", "" );
    QString textid = qElement.attribute( "textid", "-1" );
    Uml::IDType textId = STR2ID(textid);

    setSignalType((SignalType)type.toInt());
    if (getSignalType() == Time) {

        if (textId != Uml::id_None) {
            NewUMLRectWidget *flotext = umlScene()->findWidget( textId );
            if (flotext != NULL) {
            // This only happens when loading files produced by
            // umbrello-1.3-beta2.
                m_pName = static_cast<FloatingTextWidget*>(flotext);
                return true;
            }
        } else {
            // no textid stored->get unique new one
            textId = UniqueID::gen();
        }
    }
     //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    if ( !element.isNull() ) {
        QString tag = element.tagName();
        if (tag == "floatingtext") {
            m_pName = new FloatingTextWidget( umlScene(),Uml::tr_Floating,m_Text, textId );
            if( ! m_pName->loadFromXMI(element) ) {
                // Most likely cause: The FloatingTextWidget is empty.
                delete m_pName;
                m_pName = NULL;
            }
        } else {
            uError() << "unknown tag " << tag << endl;
        }
    }
   return true;
}


#include "signalwidget.moc"

