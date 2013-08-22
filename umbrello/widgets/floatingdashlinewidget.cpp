/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/
// own header
#include "floatingdashlinewidget.h"
#include "combinedfragmentwidget.h"

//kde includes
#include <kinputdialog.h>
#include <klocale.h>

//app includes
#include "debug_utils.h"
#include "umlview.h"
#include "widget_utils.h"
#include "listpopupmenu.h"

// qt includes
#include <QPainter>

DEBUG_REGISTER_DISABLED(FloatingDashLineWidget)

/**
 * Creates a floating dash line.
 * @param scene   The parent of the widget
 * @param id      The ID to assign (-1 will prompt a new ID)
 */
FloatingDashLineWidget::FloatingDashLineWidget(UMLScene * scene, Uml::ID::Type id, CombinedFragmentWidget *parent)
  : UMLWidget(scene, WidgetBase::wt_FloatingDashLine, id),
    m_parent(parent)
{
    m_resizable = false;
    m_Text = "";
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight = fm.lineSpacing();
    setSize(10, fontHeight);
}

/**
 * Destructor.
 */
FloatingDashLineWidget::~FloatingDashLineWidget()
{
    if (m_parent)
        m_parent->removeDashLine(this);
}

/**
 * Overrides the standard paint event.
 */
void FloatingDashLineWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    painter->setPen(textColor());
    painter->setFont(UMLWidget::font());
    painter->drawText(FLOATING_DASH_LINE_TEXT_MARGIN, 0,
               width() - FLOATING_DASH_LINE_TEXT_MARGIN * 2, fontHeight,
               Qt::AlignLeft, '[' + m_Text + ']');
    painter->setPen(QPen(UMLWidget::lineColor(), 0, Qt::DashLine));
    painter->drawLine(0, 0, width(), 0);
    if(m_selected)
        paintSelected(painter);
}

/**
 * Sets m_text.
 */
void FloatingDashLineWidget::setText(const QString& text)
{
    m_Text = text;
}

/**
 * Returns true if the given point is near the floatingdashline.
 */
bool FloatingDashLineWidget::onLine(const UMLScenePoint &point)
{
    // check if the given point is the start or end point of the line
    if (((abs(y() + height() - point.y())) <= POINT_DELTA) || (abs(y() - point.y()) <= POINT_DELTA)) {
        return true;
    }
    // check if the given point is the start or end point of the line
   return false;
}

void FloatingDashLineWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_Rename:
        {
            bool ok = false;
            QString name = m_Text;
            name = KInputDialog::getText(i18n("Enter alternative Name"), i18n("Enter the alternative :"), m_Text, &ok);
            if (ok && name.length() > 0)
                m_Text = name;
        }
        break;
    default:
        UMLWidget::slotMenuSelection(action);
    }
}

/**
 * Overrides the setY method.
 */
void FloatingDashLineWidget::setY(UMLSceneValue y)
{
    if(y >= m_yMin + FLOATING_DASH_LINE_MARGIN && y <= m_yMax - FLOATING_DASH_LINE_MARGIN)
        UMLWidget::setY(y);
}

/**
 * Sets m_yMin.
 */
void FloatingDashLineWidget::setYMin(UMLSceneValue yMin)
{
    m_yMin = yMin;
}

/**
 * Sets m_yMax.
 */
void FloatingDashLineWidget::setYMax(UMLSceneValue yMax)
{
    m_yMax = yMax;
}

/**
 * Returns m_yMin.
 */
UMLSceneValue FloatingDashLineWidget::getYMin() const
{
    return m_yMin;
}

/**
 * Returns the difference between the y-coordinate of the dash line and m_yMin.
 */
UMLSceneValue FloatingDashLineWidget::getDiffY() const
{
    return (y() - getYMin());
}

/**
 * Creates the "floatingdashline" XMI element.
 */
void FloatingDashLineWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement textElement = qDoc.createElement("floatingdashlinewidget");
    UMLWidget::saveToXMI(qDoc, textElement);
    textElement.setAttribute("text", m_Text);
    textElement.setAttribute("y", y());
    textElement.setAttribute("minY", m_yMin);
    textElement.setAttribute("maxY", m_yMax);

    qElement.appendChild(textElement);
}

/**
 * Loads the "floatingdashline" XMI element.
 */
bool FloatingDashLineWidget::loadFromXMI(QDomElement & qElement)
{
    if(!UMLWidget::loadFromXMI(qElement)) {
        return false;
    }
    DEBUG(DBG_SRC) << "load.......";
    m_yMax = qElement.attribute("maxY", "").toFloat();
    m_yMin = qElement.attribute("minY", "").toFloat();
    m_Text = qElement.attribute("text", "");
    DEBUG(DBG_SRC) << "m_y......." << m_y;
    return true;
}

#include "floatingdashlinewidget.moc"
