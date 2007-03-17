/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "endoflifewidget.h"

// qt includes
#include <qpainter.h>

// kde includes
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>

// app includes
#include "uml.h"
#include "umldoc.h"
#include "docwindow.h"
#include "umlview.h"
#include "listpopupmenu.h"
#include "dialogs/activitydialog.h"
#include "objectwidget.h"
#include "classifier.h"
#include "uniqueid.h"

//Added by qt3to4:
//Added by qt3to4:
#include <QMouseEvent>
#include <QPolygon>
#include <QMoveEvent>
#include <QResizeEvent>

EndOfLifeWidget::EndOfLifeWidget(UMLView * view, ObjectWidget* a, Uml::IDType id )
        : UMLWidget(view, id)
{
    init();
    m_pOw[Uml::A] = a;
    int y = getY();
    m_nY = y;
    //updateResizability();
    updateComponentSize();
    // calculateWidget();
    y = y < getMinY() ? getMinY() : y;
    y = y > getMaxY() ? getMaxY() : y;
    m_nY = y;
    this->activate();

}

EndOfLifeWidget::~EndOfLifeWidget() {}

void EndOfLifeWidget::init() {
    UMLWidget::setBaseType(Uml::wt_EndOfLife);
    m_bIgnoreSnapToGrid = true;
    m_bIgnoreSnapComponentSizeToGrid = true;
    m_bResizable =  true ;
    m_pOw[Uml::A] = NULL;
    m_nY = 0;
    setVisible(true);
}

void EndOfLifeWidget::draw(QPainter & p, int offsetX, int offsetY) {
    int w = width();
    int h = height();

    int x1 = m_pOw[Uml::A]->getX() + m_pOw[Uml::A]->getWidth() / 2;
    x1 -= w/2;
    setX(x1);
    int y1 = offsetY;

    //test if y isn't above the object
    if (y1 <= m_pOw[Uml::A]->getY() + m_pOw[Uml::A]->getHeight() ) {
        y1 = m_pOw[Uml::A]->getY() + m_pOw[Uml::A]->getHeight() + 15;
    }
    setY(y1);
    UMLWidget::setPen(p);
    if ( UMLWidget::getUseFillColour() ) {
        p.setBrush( UMLWidget::getFillColour() );
    }
    {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();

        p.drawLine(x1, y1, x1+w, y1+h);
        p.drawLine(x1, y1+h, x1+w, y1);
        p.setPen(Qt::black);
        p.setFont( UMLWidget::getFont() );
        m_pOw[Uml::A]->setEndLine(y1+h/2);
    }
    if (m_bSelected)
        drawSelected(&p, x1, y1);
}


QSize EndOfLifeWidget::calculateSize() {
    int width = 10, height = 10;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    height = fontHeight;
    height = height > ENDOFLIFE_HEIGHT ? height : ENDOFLIFE_HEIGHT;
    height += ENDOFLIFE_MARGIN * 2;

    return QSize(width, height);

}


void EndOfLifeWidget::slotMenuSelection(int sel) {
    bool done = false;

    bool ok = false;
    QString name = m_Text;

    switch ( sel ) {
    case ListPopupMenu::mt_Rename:
        name = KInputDialog::getText( i18n("Enter Precondition Name"), i18n("Enter the precondition :"), m_Text, &ok );
        if ( ok && name.length() > 0 )
            m_Text = name;
        done = true;
        calculateWidget();
        break;
    }
    if ( !done )
        UMLWidget::slotMenuSelection( sel );
}

void EndOfLifeWidget::calculateWidget() {
    calculateDimensions();

    setVisible(true);

    setX(m_nPosX);
    setY(m_nY);
}

void EndOfLifeWidget::activate(IDChangeLog * Log /*= 0*/) {
    m_pView->resetPastePoint();
    UMLWidget::activate(Log);
    if (m_pOw[Uml::A] == NULL) {
        kDebug() << "EndOfLifeWidget::activate: can't make precondition" << endl;
        return;
    }

    connect(m_pOw[Uml::A], SIGNAL(sigWidgetMoved(Uml::IDType)), this, SLOT(slotWidgetMoved(Uml::IDType)));

    calculateDimensions();
}

void EndOfLifeWidget::calculateDimensions() {
    int x = 0;
    int w = 0;
    int h = 0;
    int x1 = m_pOw[Uml::A]->getX();
    int w1 = m_pOw[Uml::A]->getWidth() / 2;

    x1 += w1;

    QSize q = calculateSize();
    w = q.width() > width() ? q.width() : width();
    h = q.height() > height() ? q.height() : height();

    x = x1 - w/2;

    m_nPosX = x;

    setSize(w,h);

}

void EndOfLifeWidget::slotWidgetMoved(Uml::IDType id) {
    const Uml::IDType idA = m_pOw[Uml::A]->getLocalID();
    if (idA != id ) {
        kDebug() << "MessageWidget::slotWidgetMoved(" << ID2STR(id)
        << "): ignoring for idA=" << ID2STR(idA) << endl;
        return;
    }
    m_nY = getY();
    if (m_nY < getMinY())
        m_nY = getMinY();
    if (m_nY > getMaxY())
        m_nY = getMaxY();

    calculateDimensions();
    if (m_pView->getSelectCount(true) > 1)
        return;

}


int EndOfLifeWidget::getMinY() {
    if (!m_pOw[Uml::A]) {
        return 0;
    }

    int heightA = m_pOw[Uml::A]->getY() + m_pOw[Uml::A]->getHeight();
    int height = heightA;
    return height;
}

int EndOfLifeWidget::getMaxY() {
    if ( !m_pOw[Uml::A]) {
        return 0;
    }

    int heightA = (int)((ObjectWidget*)m_pOw[Uml::A])->getEndLineY();
    int height = heightA;
    return (height - this->height());
}


void EndOfLifeWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement preconditionElement = qDoc.createElement( "endoflifewidget" );
    UMLWidget::saveToXMI( qDoc, preconditionElement );
    preconditionElement.setAttribute( "widgetaid", ID2STR(m_pOw[Uml::A]->getLocalID()) );
    preconditionElement.setAttribute( "endoflifename", m_Text );
    preconditionElement.setAttribute( "documentation", m_Doc );
    qElement.appendChild( preconditionElement );
}

bool EndOfLifeWidget::loadFromXMI( QDomElement & qElement ) {
    if ( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    QString widgetaid = qElement.attribute( "widgetaid", "-1" );
    m_Text = qElement.attribute( "endoflifename", "" );
    m_Doc = qElement.attribute( "documentation", "" );

    Uml::IDType aId = STR2ID(widgetaid);

    UMLWidget *pWA = m_pView -> findWidget( aId );
    if (pWA == NULL) {
        kDebug() << "EndOfLifeWidget::loadFromXMI: role A object "
        << ID2STR(aId) << " not found" << endl;
        return false;
    }

    m_pOw[Uml::A] = dynamic_cast<ObjectWidget*>(pWA);
    if (m_pOw[Uml::A] == NULL) {
        kDebug() << "EndOfLifeWidget::loadFromXMI: role A widget "
        << ID2STR(aId) << " is not an ObjectWidget" << endl;
        return false;
    }

    return true;
}


#include "endoflifewidget.moc"

