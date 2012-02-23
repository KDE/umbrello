/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "pinwidget.h"

// qt includes
#include <QtGui/QPainter>

// kde includes
#include <klocale.h>
#include <kinputdialog.h>

// app includes
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "docwindow.h"
#include "umlscene.h"
#include "uniqueid.h"
#include "listpopupmenu.h"
#include "floatingtextwidget.h"

//Added by qt3to4:
#include <QMouseEvent>
#include <QPolygon>

PinWidget::PinWidget(UMLScene * scene, UMLWidget* a, Uml::IDType id)
  : UMLWidget(scene, WidgetBase::wt_Pin, id)
{
    init();
    setMinimumSize(10,10);
    setMaximumSize(10,10);
    setSize(10,10);
    m_pOw = a;
    int y = getY();
    m_nY = y;
    y = y < getMinY() ? getMinY() : y;
    m_nY = y;

    m_pName = new FloatingTextWidget(scene, Uml::TextRole::Floating, "");
    scene->setupNewWidget(m_pName);
    m_pName->setX(0);
    m_pName->setY(0);
    this->activate();
}

PinWidget::~PinWidget()
{
}

void PinWidget::init()
{
    m_ignoreSnapToGrid = true;
    m_ignoreSnapComponentSizeToGrid = true;
    m_resizable =  false ;
    m_pOw = NULL;
    m_nY = 0;
    setVisible(true);
}

void PinWidget::paint(QPainter & p, int offsetX, int offsetY)
{
    int w = 10;
    int h = 10;
    int width_Activity = m_pOw->getWidth();
    int height_Activity = m_pOw->getHeight();
    int y = 0;
    int x = m_pOw->getX() + (width_Activity/2);

    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    int cas = 0;

    if ( (offsetY + height_Activity/2) <= m_pOw->getY() + height_Activity){
        y = m_pOw->getY()-5;
        if (m_pName->getX() == 0 && m_pName->getY() == 0) {
            //the floating text has not been linked with the signal
            m_pName->setX(x + 5 - m_Text.length()/2);
            m_pName->setY(y -fontHeight);
            cas = 1;
        }


    } else if((offsetY + height_Activity/2) > m_pOw->getY() + height_Activity){
       y = (m_pOw->getY() + height_Activity)-5;
        if (m_pName->getX() == 0 && m_pName->getY() == 0) {
            //the floating text has not been linked with the signal
            m_pName->setX(x + 5 - m_Text.length()/2);
            m_pName->setY(y + fontHeight);
            cas = 2;
        }
    }

    if (offsetX + width_Activity/4 <= m_pOw->getX() + width_Activity/2
         && (offsetY > m_pOw->getY() +5 && offsetY < m_pOw->getY() + height_Activity - 5) ){
        x = m_pOw->getX() -5;
        y = m_pOw->getY() + (height_Activity/2) -5;
        if (m_pName->getX() == 0 && m_pName->getY() == 0) {
            m_pName->setX(x - m_Text.length());
            m_pName->setY(y - fontHeight);
            cas = 3;
        }
    } else if (offsetX + width_Activity/4 > m_pOw->getX() + width_Activity/2
         && (offsetY > m_pOw->getY() +5 && offsetY < m_pOw->getY() + height_Activity - 5) ){
        x = m_pOw->getX() + width_Activity -5;
        y = m_pOw->getY() + (height_Activity/2) -5;
        if (m_pName->getX() == 0 && m_pName->getY() == 0) {
            //the floating text has not been linked with the signal
            m_pName->setX(x + 10);
            m_pName->setY(y - fontHeight);
            cas = 4;
        }
    }

    m_oldX = getX();
    setX(x);
    m_oldY = getY();
    setY(y);

//test if y isn't above the object
//     if ( y <= m_pOw[Uml::A]->getY() + height_Activity-5 && x == m_pOw[Uml::A]->getX() + (width_Activity/2) ) {
//         y = m_pOw[Uml::A]->getY() + height_Activity + 15;
//     }
//     if (y + h >= m_pOw[Uml::A]->getEndLineY()) {
//         y = m_pOw[Uml::A]->getEndLineY() - h;
//     }

    setPenFromSettings(p);
    if ( UMLWidget::useFillColor() ) {
        p.setBrush( UMLWidget::fillColor() );
    }
    p.drawRect(x,y,w, h);
    //make sure it's always above the other
    setZ(20);
    setPenFromSettings(p);
    m_pName->setVisible(( m_pName->text().length() > 0 ));
    m_pName->updateComponentSize();
    if(m_selected)
         drawSelected(&p, offsetX, offsetY);
}

void PinWidget::setName(const QString &strName)
{
    m_Text = strName;
    updateComponentSize();
    m_pName->setText(m_Text);
}

int PinWidget::getMinY()
{
    if (!m_pOw) {
        return 0;
    }
    int heightA = m_pOw->getY() + m_pOw->getHeight();
    return heightA;
}

void PinWidget::mouseMoveEvent(QMouseEvent* me)
{
    UMLWidget::mouseMoveEvent(me);
    int diffX = m_oldX - getX();
    int diffY = m_oldY - getY();
    if (m_pName!=NULL && !( m_pName->text() ).isEmpty()) {
        m_pName->setX(m_pName->getX() - diffX);
        m_pName->setY(m_pName->getY() - diffY);
    }
}

void PinWidget::slotMenuSelection(QAction* action)
{
    bool ok = false;
    QString name = m_Text;

    ListPopupMenu::MenuType sel = m_pMenu->getMenuType(action);
    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        name = KInputDialog::getText( i18n("Enter Pin Name"), i18n("Enter the pin name :"), m_Text, &ok );
        if( ok )
            setName(name);
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

void PinWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement PinElement = qDoc.createElement( "pinwidget" );
    PinElement.setAttribute( "widgetaid", ID2STR(m_pOw->id()) );
    UMLWidget::saveToXMI( qDoc, PinElement );
    if (m_pName && !m_pName->text().isEmpty()) {
        PinElement.setAttribute( "textid", ID2STR(m_pName->id()) );
        m_pName -> saveToXMI( qDoc, PinElement );
    }
    qElement.appendChild( PinElement );
}


bool PinWidget::loadFromXMI( QDomElement & qElement )
{
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    QString widgetaid = qElement.attribute( "widgetaid", "-1" );

    Uml::IDType aId = STR2ID(widgetaid);

    UMLWidget *pWA = m_scene -> findWidget( aId );
    if (pWA == NULL) {
        uDebug() << "role A object " << ID2STR(aId) << " not found";
        return false;
    }

    m_pOw = pWA;

    QString textid = qElement.attribute( "textid", "-1" );
    Uml::IDType textId = STR2ID(textid);
    if (textId != Uml::id_None) {
        UMLWidget *flotext = m_scene -> findWidget( textId );
        if (flotext != NULL) {
            // This only happens when loading files produced by
            // umbrello-1.3-beta2.
            m_pName = static_cast<FloatingTextWidget*>(flotext);
            //return true;
        }
    } else {
        // no textid stored -> get unique new one
        textId = UniqueID::gen();
    }

      //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    if ( !element.isNull() ) {
        QString tag = element.tagName();
        if (tag == "floatingtext") {
            m_pName = new FloatingTextWidget( m_scene, Uml::TextRole::Floating, m_Text, textId );
            if( ! m_pName->loadFromXMI(element) ) {
                // Most likely cause: The FloatingTextWidget is empty.
                delete m_pName;
                m_pName = NULL;
            }
        } else {
            uError() << "unknown tag " << tag;
        }
    }

    return true;
}

#include "pinwidget.moc"
