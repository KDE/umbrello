/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
// own header
#include "floatingdashlinewidget.h"

//kde includes
#include "kdebug.h"
#include "kinputdialog.h"
#include "klocale.h"

//app includes
#include "umlview.h"
#include "widget_utils.h"
#include "listpopupmenu.h"
#include "umlscene.h"

// qt includes
#include <qpainter.h>

FloatingDashLineWidget::FloatingDashLineWidget(UMLScene * scene, Uml::IDType id)
: UMLWidget(scene, id)
{
    UMLWidget::setBaseType(Uml::wt_FloatingDashLine);
    m_bResizable = false;
    m_Text = "";
    updateComponentSize();
}

FloatingDashLineWidget::~FloatingDashLineWidget() {}

void FloatingDashLineWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
    QPainter &p = *painter;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const qreal fontHeight  = fm.lineSpacing();
    p.setPen(Qt::black);
    p.setFont(UMLWidget::getFont());
    p.drawText(getX() + FLOATING_DASH_LINE_TEXT_MARGIN, getY(),
               getWidth() - FLOATING_DASH_LINE_TEXT_MARGIN * 2, fontHeight,
               Qt::AlignLeft, '[' + m_Text + ']');
    p.setPen(*(new QPen(UMLWidget::getLineColor(), 0, Qt::DashLine)));
    p.drawLine(getX(), getY(), getX() + getWidth(), getY());
    if(isSelected())
        drawSelected(&p, getX(), getY());
}

void FloatingDashLineWidget::setText(const QString& text)
{
    m_Text = text;
}

bool FloatingDashLineWidget::onLine(const QPointF &point) {
 /*check if the given point is the start or end point of the line */
    if (( (qAbs( getY() + getHeight() - point.y() )) <= POINT_DELTA) || (qAbs( getY() - point.y() ) <= POINT_DELTA)) {
        return true;
    }
    /* check if the given point is the start or end point of the line */
   return false;
}

void FloatingDashLineWidget::slotMenuSelection(QAction* action) {
    bool ok = false;
    QString name = m_Text;

    ListPopupMenu::Menu_Type sel = m_pMenu->getMenuType(action);
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

void FloatingDashLineWidget::setY(qreal y)
{
    if(y >= m_yMin + FLOATING_DASH_LINE_MARGIN && y <= m_yMax - FLOATING_DASH_LINE_MARGIN)
        UMLWidget::setY(y);
}

void FloatingDashLineWidget::setYMin(qreal yMin)
{
    m_yMin = yMin;
}

void FloatingDashLineWidget::setYMax(qreal yMax)
{
    m_yMax = yMax;
}

qreal FloatingDashLineWidget::getYMin()
{
    return m_yMin;
}

qreal FloatingDashLineWidget::getDiffY()
{
    return (getY() - m_yMin);
}

void FloatingDashLineWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement textElement = qDoc.createElement( "floatingdashlinewidget" );
    UMLWidget::saveToXMI( qDoc, textElement );
    textElement.setAttribute( "text", m_Text );
    textElement.setAttribute( "y", getY() );
    textElement.setAttribute( "minY", m_yMin );
    textElement.setAttribute( "maxY", m_yMax );

    qElement.appendChild( textElement );
}

bool FloatingDashLineWidget::loadFromXMI( QDomElement & qElement ) {
    if( !UMLWidget::loadFromXMI( qElement ) ) {
        return false;
    }
    uDebug() <<"load.......";
    m_yMax = qElement.attribute( "maxY", "" ).toDouble();
    m_yMin = qElement.attribute( "minY", "" ).toDouble();
    setY(qElement.attribute( "y", "" ).toDouble());
    m_Text = qElement.attribute( "text", "" );
    uDebug() <<"m_y......." <<m_y;
    return true;
}

#include "floatingdashlinewidget.moc"
