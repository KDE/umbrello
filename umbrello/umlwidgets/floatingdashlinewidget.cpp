/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/
// own header
#include "floatingdashlinewidget.h"
#include "combinedfragmentwidget.h"

//kde includes
#include <KLocalizedString>

//app includes
#include "debug_utils.h"
#include "dialog_utils.h"
#include "umlview.h"
#include "widget_utils.h"
#include "listpopupmenu.h"

// qt includes
#include <QPainter>
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(FloatingDashLineWidget)

/**
 * Creates a floating dash line.
 * @param scene   The parent of the widget
 * @param id      The ID to assign (-1 will prompt a new ID)
 * @param parent  The CombinedFragmentWidget which acts as the parent
 */
FloatingDashLineWidget::FloatingDashLineWidget(UMLScene * scene, Uml::ID::Type id, CombinedFragmentWidget *parent)
  : UMLWidget(scene, WidgetBase::wt_FloatingDashLine, id),
    m_yMin(0),
    m_yMax(0),
    m_parent(parent)
{
    m_resizable = false;
    m_Text = QString();
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
               Qt::AlignLeft, QLatin1Char('[') + m_Text + QLatin1Char(']'));
    painter->setPen(QPen(UMLWidget::lineColor(), 0, Qt::DashLine));
    painter->drawLine(0, 0, width(), 0);

    UMLWidget::paint(painter, option, widget);
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
bool FloatingDashLineWidget::onLine(const QPointF& point)
{
    // check if the given point is the start or end point of the line
    if (((abs((long)(y() + height() - point.y()))) <= POINT_DELTA) || (abs((long)(y() - point.y())) <= POINT_DELTA)) {
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
            QString name = m_Text;
            bool ok = Dialog_Utils::askName(i18n("Enter alternative Name"),
                                            i18n("Enter the alternative:"),
                                            name);
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
void FloatingDashLineWidget::setY(qreal y)
{
    if(y >= m_yMin + FLOATING_DASH_LINE_MARGIN && y <= m_yMax - FLOATING_DASH_LINE_MARGIN)
        UMLWidget::setY(y);
}

/**
 * Sets m_yMin.
 */
void FloatingDashLineWidget::setYMin(qreal yMin)
{
    m_yMin = yMin;
}

/**
 * Sets m_yMax.
 */
void FloatingDashLineWidget::setYMax(qreal yMax)
{
    m_yMax = yMax;
}

/**
 * Returns m_yMin.
 */
qreal FloatingDashLineWidget::getYMin() const
{
    return m_yMin;
}

/**
 * Returns the difference between the y-coordinate of the dash line and m_yMin.
 */
qreal FloatingDashLineWidget::getDiffY() const
{
    return (y() - getYMin());
}

/**
 * Creates the "floatingdashline" XMI element.
 */
void FloatingDashLineWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("floatingdashlinewidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeAttribute(QStringLiteral("text"), m_Text);
    writer.writeAttribute(QStringLiteral("minY"), QString::number(m_yMin));
    writer.writeAttribute(QStringLiteral("maxY"), QString::number(m_yMax));

    writer.writeEndElement();
}

/**
 * Loads the "floatingdashline" XMI element.
 */
bool FloatingDashLineWidget::loadFromXMI(QDomElement & qElement)
{
    m_yMax = qElement.attribute(QStringLiteral("maxY")).toFloat();
    m_yMin = qElement.attribute(QStringLiteral("minY")).toFloat();
    m_Text = qElement.attribute(QStringLiteral("text"));

    if(!UMLWidget::loadFromXMI(qElement)) {
        return false;
    }

    return true;
}

