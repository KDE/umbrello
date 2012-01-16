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
#include "floatingdashlinewidget.h"

//kde includes
#include <kinputdialog.h>
#include <klocale.h>

//app includes
#include "debug_utils.h"
#include "umlview.h"
#include "widget_utils.h"
#include "listpopupmenu.h"

// qt includes
#include <QtGui/QPainter>

FloatingDashLineWidget::FloatingDashLineWidget(UMLScene * scene, Uml::IDType id)
  : UMLWidget(scene, WidgetBase::wt_FloatingDashLine, id)
{
    m_resizable = false;
    m_Text = "";
}

FloatingDashLineWidget::~FloatingDashLineWidget()
{
}

void FloatingDashLineWidget::paint(QPainter & p, int /*offsetX*/, int /*offsetY*/)
{
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    p.setPen(textColor());
    p.setFont(UMLWidget::font());
    p.drawText(getX() + FLOATING_DASH_LINE_TEXT_MARGIN, getY(),
               getWidth() - FLOATING_DASH_LINE_TEXT_MARGIN * 2, fontHeight,
               Qt::AlignLeft, '[' + m_Text + ']');
    p.setPen(QPen(UMLWidget::lineColor(), 0, Qt::DashLine));
    p.drawLine(getX(), getY(), getX() + getWidth(), getY());
    if(m_selected)
        drawSelected(&p, getX(), getY());
}

void FloatingDashLineWidget::setText(const QString& text)
{
    m_Text = text;
}

bool FloatingDashLineWidget::onLine(const QPoint &point)
{
 /*check if the given point is the start or end point of the line */
    if (( (abs( getY() + getHeight() - point.y() )) <= POINT_DELTA) || (abs( getY() - point.y() ) <= POINT_DELTA)) {
        return true;
    }
    /* check if the given point is the start or end point of the line */
   return false;
}

void FloatingDashLineWidget::slotMenuSelection(QAction* action)
{
    bool ok = false;
    QString name = m_Text;

    ListPopupMenu::MenuType sel = m_pMenu->getMenuType(action);
    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        name = KInputDialog::getText( i18n("Enter alternative Name"), i18n("Enter the alternative :"), m_Text, &ok );
        if( ok && name.length() > 0 )
            m_Text = name;
        break;
    default:
        UMLWidget::slotMenuSelection(action);
    }
}

void FloatingDashLineWidget::setY(int y)
{
    if(y >= m_yMin + FLOATING_DASH_LINE_MARGIN && y <= m_yMax - FLOATING_DASH_LINE_MARGIN)
        UMLWidget::setY(y);
}

void FloatingDashLineWidget::setYMin(int yMin)
{
    m_yMin = yMin;
}

void FloatingDashLineWidget::setYMax(int yMax)
{
    m_yMax = yMax;
}

int FloatingDashLineWidget::getYMin()
{
    return m_yMin;
}

int FloatingDashLineWidget::getDiffY()
{
    return (getY() - m_yMin);
}

void FloatingDashLineWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement textElement = qDoc.createElement( "floatingdashlinewidget" );
    UMLWidget::saveToXMI( qDoc, textElement );
    textElement.setAttribute( "text", m_Text );
    textElement.setAttribute( "y", getY() );
    textElement.setAttribute( "minY", m_yMin );
    textElement.setAttribute( "maxY", m_yMax );

    qElement.appendChild( textElement );
}

bool FloatingDashLineWidget::loadFromXMI( QDomElement & qElement )
{
    if( !UMLWidget::loadFromXMI( qElement ) ) {
        return false;
    }
    uDebug() << "load.......";
    m_yMax = qElement.attribute( "maxY", "" ).toInt();
    m_yMin = qElement.attribute( "minY", "" ).toInt();
    setY(qElement.attribute( "y", "" ).toInt());
    m_Text = qElement.attribute( "text", "" );
    uDebug() << "m_y......." << m_y;
    return true;
}

#include "floatingdashlinewidget.moc"
