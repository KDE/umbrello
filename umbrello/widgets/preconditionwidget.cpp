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
#include "preconditionwidget.h"

// kde includes
#include <klocale.h>
#include <kinputdialog.h>

// app includes
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "docwindow.h"
#include "umlview.h"
#include "listpopupmenu.h"
#include "objectwidget.h"
#include "classifier.h"
#include "uniqueid.h"

// qt includes
#include <QtGui/QPainter>

PreconditionWidget::PreconditionWidget(UMLScene * scene, ObjectWidget* a, Uml::IDType id )
  : UMLWidget(scene, WidgetBase::wt_Precondition, id)
{
    init();
    m_pOw = a;
    int y = getY();
    m_nY = y;
    //updateResizability();
   // calculateWidget();
    y = y < getMinY() ? getMinY() : y;
    y = y > getMaxY() ? getMaxY() : y;
    m_nY = y;
    this->activate();
}

PreconditionWidget::~PreconditionWidget()
{
}

void PreconditionWidget::init()
{
    m_ignoreSnapToGrid = true;
    m_ignoreSnapComponentSizeToGrid = true;
    m_resizable =  true ;
    m_pOw = NULL;
    m_nY = 0;
    setVisible(true);
}

void PreconditionWidget::paint(QPainter & p, int /*offsetX*/, int offsetY)
{
    int w = width();
    int h = height();

    int x = m_pOw->getX() + m_pOw->getWidth() / 2;
    x -= w/2;
    setX(x);
    int y = offsetY;

    //test if y isn't above the object
    if (y <= m_pOw->getY() + m_pOw->getHeight() ) {
        y = m_pOw->getY() + m_pOw->getHeight() + 15;
    }
    if (y + h >= m_pOw->getEndLineY()) {
        y = m_pOw->getEndLineY() - h;
    }
    setY(y);
    setPenFromSettings(p);
    if ( UMLWidget::useFillColor() ) {
        p.setBrush( UMLWidget::fillColor() );
    }
    {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();
        const QString precondition_value = "{ " + name() + " }";
        //int middleX = w / 2;
        int textStartY = (h / 2) - (fontHeight / 2);
        p.drawRoundRect(x, y, w, h, (h * 60) / w, 60);
        p.setPen(textColor());
        p.setFont( UMLWidget::font() );
        p.drawText(x + PRECONDITION_MARGIN, y + textStartY,
                       w - PRECONDITION_MARGIN * 2, fontHeight, Qt::AlignCenter, precondition_value);
    }
    if(m_selected)
        drawSelected(&p, x, y);
}

UMLSceneSize PreconditionWidget::minimumSize()
{
    int width = 10, height = 10;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const int textWidth = fm.width(name()) + 25;
    height = fontHeight;
    width = textWidth > PRECONDITION_WIDTH ? textWidth : PRECONDITION_WIDTH;
    height = height > PRECONDITION_HEIGHT ? height : PRECONDITION_HEIGHT;
    width += PRECONDITION_MARGIN * 2;
    height += PRECONDITION_MARGIN * 2;

    return UMLSceneSize(width, height);
}

void PreconditionWidget::slotMenuSelection(QAction* action)
{
    bool ok = false;
    QString name = m_Text;

    ListPopupMenu::MenuType sel = m_pMenu->getMenuType(action);
    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        name = KInputDialog::getText( i18n("Enter Precondition Name"), i18n("Enter the precondition :"), m_Text, &ok );
        if( ok && name.length() > 0 )
            m_Text = name;
        calculateWidget();
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

void PreconditionWidget::calculateWidget()
{
    calculateDimensions();

    setVisible(true);

    setX(m_nPosX);
    setY(m_nY);
}

bool PreconditionWidget::activate(IDChangeLog * Log /*= 0*/)
{
    m_scene->resetPastePoint();
    UMLWidget::activate(Log);
    if (m_pOw == NULL) {
        uDebug() << "cannot make precondition";
        return false;
    }

    connect(m_pOw, SIGNAL(sigWidgetMoved(Uml::IDType)), this, SLOT(slotWidgetMoved(Uml::IDType)));

    calculateDimensions();
    return true;
}

void PreconditionWidget::calculateDimensions()
{
    int x = 0;
    int w = 0;
    int h = 0;
    int x1 = m_pOw->getX();
    int w1 = m_pOw->getWidth() / 2;

    x1 += w1;

    QSize q = minimumSize();
    w = q.width() > width() ? q.width() : width();
    h = q.height() > height() ? q.height() : height();

    x = x1 - w/2;

    m_nPosX = x;

    setSize(w,h);
}

void PreconditionWidget::slotWidgetMoved(Uml::IDType id)
{
    const Uml::IDType idA = m_pOw->localID();
    if (idA != id ) {
        uDebug() << "id=" << ID2STR(id) << ": ignoring for idA=" << ID2STR(idA);
        return;
    }
    m_nY = getY();
    if (m_nY < getMinY())
        m_nY = getMinY();
    if (m_nY > getMaxY())
        m_nY = getMaxY();

    calculateDimensions();
    if (m_scene->getSelectCount(true) > 1)
        return;

}

int PreconditionWidget::getMinY()
{
    if (!m_pOw) {
        return 0;
    }

    int heightA = m_pOw->getY() + m_pOw->getHeight();
    int height = heightA;
    return height;
}

int PreconditionWidget::getMaxY()
{
    if( !m_pOw) {
        return 0;
    }

    int heightA = (int)m_pOw->getEndLineY();
    int height = heightA;
    return (height - this->height());
}

void PreconditionWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement preconditionElement = qDoc.createElement( "preconditionwidget" );
    UMLWidget::saveToXMI( qDoc, preconditionElement );
    preconditionElement.setAttribute( "widgetaid", ID2STR(m_pOw->localID()) );
    preconditionElement.setAttribute( "preconditionname", m_Text );
    preconditionElement.setAttribute( "documentation", m_Doc );
    qElement.appendChild( preconditionElement );
}

bool PreconditionWidget::loadFromXMI( QDomElement & qElement )
{
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    QString widgetaid = qElement.attribute( "widgetaid", "-1" );
    m_Text = qElement.attribute( "preconditionname", "" );
    m_Doc = qElement.attribute( "documentation", "" );

    Uml::IDType aId = STR2ID(widgetaid);

    UMLWidget *pWA = m_scene -> findWidget( aId );
    if (pWA == NULL) {
        uDebug() << "role A object " << ID2STR(aId) << " not found";
        return false;
    }

    m_pOw = dynamic_cast<ObjectWidget*>(pWA);
    if (m_pOw == NULL) {
        uDebug() << "role A widget " << ID2STR(aId) << " is not an ObjectWidget";
        return false;
    }

    return true;
}


#include "preconditionwidget.moc"

