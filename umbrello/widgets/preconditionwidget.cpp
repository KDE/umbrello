/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "preconditionwidget.h"

// app includes
#include "debug_utils.h"
#include "listpopupmenu.h"
#include "objectwidget.h"
#include "uml.h"
#include "umlscene.h"
#include "uniqueid.h"
#include "idchangelog.h"

// kde includes
#include <kinputdialog.h>
#include <klocale.h>

// qt includes
#include <QPainter>

DEBUG_REGISTER_DISABLED(PreconditionWidget)

#define PRECONDITION_MARGIN 5
#define PRECONDITION_WIDTH 30
#define PRECONDITION_HEIGHT 10

/**
 * Creates a Precondition widget.
 *
 * @param scene   The parent of the widget.
 * @param a       The role A widget for this precondition.
 * @param id      The ID to assign (-1 will prompt a new ID).
 */
PreconditionWidget::PreconditionWidget(UMLScene* scene, ObjectWidget* a, Uml::ID::Type id)
  : UMLWidget(scene, WidgetBase::wt_Precondition, id),
    m_objectWidget(a)
{
    m_ignoreSnapToGrid = true;
    m_ignoreSnapComponentSizeToGrid = true;
    m_resizable =  true ;
    setVisible(true);
    //updateResizability();
    // calculateWidget();
    if (y() < minY())
        m_nY = minY();
    else if (y() > maxY())
        m_nY = maxY();
    else
        m_nY = y();

    activate();
}

/**
 * Destructor.
 */
PreconditionWidget::~PreconditionWidget()
{
}

/**
 * Overrides the standard paint event.
 */
void PreconditionWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    int w = width();
    int h = height();

    int x = m_objectWidget->x() + m_objectWidget->width() / 2;
    x -= w/2;
    setX(x);
    int y = this->y();

    //test if y isn't above the object
    if (y <= m_objectWidget->y() + m_objectWidget->height()) {
        y = m_objectWidget->y() + m_objectWidget->height() + 15;
    }
    if (y + h >= m_objectWidget->getEndLineY()) {
        y = m_objectWidget->getEndLineY() - h;
    }
    setY(y);
    setPenFromSettings(painter);
    if (UMLWidget::useFillColor()) {
        painter->setBrush(UMLWidget::fillColor());
    }
    {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing();
        const QString precondition_value = QLatin1String("{ ") + name() + QLatin1String(" }");
        //int middleX = w / 2;
        int textStartY = (h / 2) - (fontHeight / 2);
        painter->drawRoundRect(0, 0, w, h, (h * 60) / w, 60);
        painter->setPen(textColor());
        painter->setFont(UMLWidget::font());
        painter->drawText(PRECONDITION_MARGIN, textStartY,
                       w - PRECONDITION_MARGIN * 2, fontHeight, Qt::AlignCenter, precondition_value);
    }

    UMLWidget::paint(painter, option, widget);
}

/**
 * Overrides method from UMLWidget.
 */
QSizeF PreconditionWidget::minimumSize()
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

    return QSizeF(width, height);
}

/**
 * Calculate the geometry of the widget.
 */
void PreconditionWidget::calculateWidget()
{
    calculateDimensions();

    setVisible(true);

    setX(m_nPosX);
    setY(m_nY);
}

/**
 * Activates a PreconditionWidget.  Connects the WidgetMoved signal from
 * its m_objectWidget pointer so that PreconditionWidget can adjust to the move of
 * the object widget.
 */
bool PreconditionWidget::activate(IDChangeLog * Log /*= 0*/)
{
    m_scene->resetPastePoint();
    UMLWidget::activate(Log);

    loadObjectWidget();

    if (!m_objectWidget) {
        DEBUG(DBG_SRC) << "role A widget " << Uml::ID::toString(m_widgetAId)
            << " could not be found";
        return false;
    }

    connect(m_objectWidget, SIGNAL(sigWidgetMoved(Uml::ID::Type)), this, SLOT(slotWidgetMoved(Uml::ID::Type)));

    calculateDimensions();
    return true;
}

/**
 * Resolve references of this precondition so it references the correct
 * new object widget after paste.
 */
void PreconditionWidget::resolveObjectWidget(IDChangeLog* log) {
    m_widgetAId = log->findNewID(m_widgetAId);
}

/**
 * Calculates the size of the widget.
 */
void PreconditionWidget::calculateDimensions()
{
    int x = 0;
    int w = 0;
    int h = 0;
    int x1 = m_objectWidget->x();
    int w1 = m_objectWidget->width() / 2;

    x1 += w1;

    QSizeF q = minimumSize();
    w = q.width() > width() ? q.width() : width();
    h = q.height() > height() ? q.height() : height();

    x = x1 - w/2;

    m_nPosX = x;

    setSize(w, h);
}

/**
 * Slot when widget is moved.
 */
void PreconditionWidget::slotWidgetMoved(Uml::ID::Type id)
{
    const Uml::ID::Type idA = m_objectWidget->localID();
    if (idA != id) {
        DEBUG(DBG_SRC) << "id=" << Uml::ID::toString(id) << ": ignoring for idA=" << Uml::ID::toString(idA);
        return;
    }
    m_nY = y();
    if (m_nY < minY())
        m_nY = minY();
    if (m_nY > maxY())
        m_nY = maxY();

    calculateDimensions();
    if (m_scene->selectedCount(true) > 1)
        return;
}

/**
 * Returns the minimum height this widget should be set at on
 * a sequence diagrams. Takes into account the widget positions
 * it is related to.
 */
int PreconditionWidget::minY() const
{
    if (m_objectWidget) {
        return m_objectWidget->y() + m_objectWidget->height();
    }
    return 0;
}

/**
 * Returns the maximum height this widget should be set at on
 * a sequence diagrams. Takes into account the widget positions
 * it is related to.
 */
int PreconditionWidget::maxY() const
{
    if (m_objectWidget) {
        return ((int)m_objectWidget->getEndLineY() - height());
    }
    return 0;
}

/**
 * Captures any popup menu signals for menus it created.
 */
void PreconditionWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_Rename:
        {
            bool ok = false;
            QString text = name();
            text = KInputDialog::getText(i18n("Enter Precondition Name"),
                                          i18n("Enter the precondition :"),
                                          text, &ok);
            if (ok && !text.isEmpty()) {
                setName(text);
            }
            calculateWidget();
        }
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

/**
 * Saves the widget to the "preconditionwidget" XMI element.
 */
void PreconditionWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement preconditionElement = qDoc.createElement(QLatin1String("preconditionwidget"));
    UMLWidget::saveToXMI(qDoc, preconditionElement);

    preconditionElement.setAttribute(QLatin1String("widgetaid"), Uml::ID::toString(m_objectWidget->localID()));
    preconditionElement.setAttribute(QLatin1String("preconditionname"), name());
    preconditionElement.setAttribute(QLatin1String("documentation"), documentation());
    qElement.appendChild(preconditionElement);
}

/**
 * Loads the widget from the "preconditionwidget" XMI element.
 */
bool PreconditionWidget::loadFromXMI(QDomElement& qElement)
{
    if(!UMLWidget::loadFromXMI(qElement))
        return false;
    QString widgetaid = qElement.attribute(QLatin1String("widgetaid"), QLatin1String("-1"));
    setName(qElement.attribute(QLatin1String("preconditionname")));
    setDocumentation(qElement.attribute(QLatin1String("documentation")));

    m_widgetAId = Uml::ID::fromString(widgetaid);

    // Lookup the ObjectWidget, if it can't be found, assume it will be
    // resolved later
    loadObjectWidget();

    return true;
}

/**
 * Load the object widget from m_widgetAId
 *
 * This method is called in loadFromXMI() when loading an XMI file, and called
 * from activate() when activating a widget after pasting.
 */
void PreconditionWidget::loadObjectWidget()
{
    if (m_objectWidget == 0) {
        m_objectWidget = dynamic_cast<ObjectWidget*>(
            umlScene()->findWidget(m_widgetAId)
        );
    }
}

#include "preconditionwidget.moc"
