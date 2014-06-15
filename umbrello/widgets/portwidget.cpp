/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "portwidget.h"

// app includes
#include "port.h"
#include "package.h"
#include "debug_utils.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "componentwidget.h"
#include "floatingtextwidget.h"

// qt includes
#include <QPainter>
#include <QToolTip>

// sys includes
#include <cmath>

const QSizeF PortWidget::FixedSize(15, 15);

/**
 * Constructs a PortWidget.
 *
 * @param scene   The parent of this PortWidget.
 * @param d       The UMLPort this will be representing.
 */
PortWidget::PortWidget(UMLScene *scene, UMLPort *d) 
  : UMLWidget(scene, WidgetBase::wt_Port, d)
{
    m_ignoreSnapToGrid = true;
    m_ignoreSnapComponentSizeToGrid = true;
    m_resizable = false;
    m_pName = NULL;
    setMinimumSize(FixedSize);
    setMaximumSize(FixedSize);
    setSize(FixedSize);
    setToolTip(d->name());

    UMLWidget *owner = ownerWidget();
    if (owner) {
        ComponentWidget *compWidget = static_cast<ComponentWidget*>(owner);
        connect(compWidget, SIGNAL(sigCompMoved(qreal, qreal)), this, SLOT(slotOwnerMoved(qreal, qreal)));
    } else {
        uError() << "ownerWidget() returns NULL";
    }
}

/**
 * Standard deconstructor.
 */
PortWidget::~PortWidget()
{
}

UMLWidget* PortWidget::ownerWidget()
{
    const Uml::ID::Type compWidgetId = m_umlObject->umlPackage()->id();
    return m_scene->widgetOnDiagram(compWidgetId);
}

/**
 * Overrides method from UMLWidget in order to set a tooltip.
 * The tooltip is set to the port name.
 * The reason for using a tooltip for the name is that the size of the port
 * widget is not large enough to accomodate the average name.
 */
void PortWidget::updateWidget()
{
    QString strName = name();
    uDebug() << " port name is " << strName;
    if (m_pName) {
        m_pName->setText(strName);
    } else {
        setToolTip(strName);
    }
}

/**
 * Overridden from UMLWidget.
 * Moves the widget to a new position using the difference between the
 * current position and the new position.
 * Movement is constrained such that the port is always attached to its
 * component.
 *
 * @param diffX The difference between current X position and new X position.
 * @param diffY The difference between current Y position and new Y position.
 */
void PortWidget::moveWidgetBy(qreal diffX, qreal diffY)
{
    UMLWidget* owner = ownerWidget();
    qreal newX = x() + diffX;
    qreal newY = y() + diffY;
    if (owner == NULL) {
        uError() << "ownerWidget() returns NULL";
        setX(newX);
        setY(newY);
        return;
    }
    const qreal deltaTop    = fabs(y() + height() - owner->y());
    const qreal deltaBottom = fabs(owner->y() + owner->height() - y());
    const qreal deltaLeft   = fabs(x() + width() - owner->x());
    const qreal deltaRight  = fabs(owner->x() + owner->width() - x());
    bool didAnyMovement = false;
    if (deltaTop < 1.0 || deltaBottom < 1.0) {
        if (newX < owner->x() - width())
            newX = owner->x() - width();
        else if (newX > owner->x() + owner->width())
            newX = owner->x() + owner->width();
        setX(newX);
        didAnyMovement = true;
    }
    if (deltaLeft < 1.0 || deltaRight < 1.0) {
        if (newY < owner->y() - height())
            newY = owner->y() - height();
        else if (newY > owner->y() + owner->height())
            newY = owner->y() + owner->height();
        setY(newY);
        didAnyMovement = true;
    }
    if (!didAnyMovement) {
        uDebug() << "constraint failed for (" << diffX << ", " << diffY << ")";
        setX(newX);
        setY(newY);
    }
}

/**
 * Align this PortWidget's position such that it is attached at one of the
 * sides of its owner's widget.
 */
void PortWidget::attachToOwner() {
    UMLWidget *owner = ownerWidget();
    const QPointF scenePos = m_scene->pos();
    if (owner == NULL) {
        uError() << "ownerWidget() returns NULL";
        setX(scenePos.x());
        setY(scenePos.y());
        return;
    }
    bool xIsWithinComponent = false;
    if (scenePos.x() < owner->x() - width()) {
        setX(owner->x() - width());
    } else if (scenePos.x() <= owner->x() + owner->width()) {
        setX(scenePos.x());
        xIsWithinComponent = true;
    } else {
        setX(owner->x() + owner->width());
    }
    if (scenePos.y() < owner->y() - height()) {
        setY(owner->y() - height());
    } else if (scenePos.y() <= owner->y() + owner->height()) {
        if (xIsWithinComponent) {
            if (scenePos.y() <= owner->y() + owner->height() / 2.0)
                setY(owner->y() - height());
            else
                setY(owner->y() + owner->height());
        } else {
            setY(scenePos.y());
        }
    } else {
        setY(owner->y() + owner->height());
    }
}

/**
 * Overrides standard method.
 */
void PortWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    setPenFromSettings(painter);
    if (UMLWidget::useFillColor())  {
        painter->setBrush(UMLWidget::fillColor());
    } else {
        painter->setBrush(m_scene->activeView()->viewport()->palette().color(QPalette::Background));
    }

    int w = width();
    int h = height();

    painter->drawRect(0, 0, w, h);

    UMLWidget::paint(painter, option, widget);
}

void PortWidget::slotOwnerMoved(qreal diffX, qreal diffY)
{
    setX(x() + diffX);
    setY(y() + diffY);
}

/**
 * Captures any popup menu signals for menus it created.
 */
void PortWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_NameAsTooltip:
        if (m_pName) {
            action->setChecked(true);
            delete m_pName;
            m_pName = NULL;
            setToolTip(name());
        } else {
            action->setChecked(false);
            m_pName = new FloatingTextWidget(m_scene, Uml::TextRole::Floating, name());
            m_pName->setParentItem(this);
            m_pName->setText(name());  // to get geometry update
            m_pName->activate();
            UMLWidget* owner = ownerWidget();
            if (owner == NULL) {
                uError() << "ownerWidget() returns NULL";
                setX(x());
                setY(y());
            } else {
                if (x() < owner->x())
                    m_pName->setX(-m_pName->width());
                else if (x() >= owner->x() + owner->width())
                    m_pName->setX(15);
                else
                    m_pName->setX(-m_pName->width() / 2.0 + 7);
                if (y() < owner->y())
                    m_pName->setY(-m_pName->height() - 2);
                else if (y() >= owner->y() + owner->height())
                    m_pName->setY(15);
                else
                    m_pName->setY(-m_pName->height() / 2.0 + 7);
            }
            m_pName->update();
            setToolTip(QString());
            QToolTip::hideText();
        }
        break;

    default:
        UMLWidget::slotMenuSelection(action);
    }
}

FloatingTextWidget *PortWidget::floatingTextWidget() {
    return m_pName;
}

void PortWidget::setFloatingTextWidget(FloatingTextWidget *ft) {
    m_pName = ft;
    if (m_pName)
        m_pName->setParentItem(this);
}

/**
 * Override method from UMLWidget in order to additionally check m_pName.
 *
 * @param p Point to be checked.
 *
 * @return 'this' if UMLWidget::onWidget(p) returns non NULL;
 *         m_pName if m_pName is non NULL and m_pName->onWidget(p) returns non NULL;
 *         else NULL.
 */
UMLWidget* PortWidget::onWidget(const QPointF &p)
{
    if (UMLWidget::onWidget(p) != NULL)
        return this;
    if (m_pName) {
        uDebug() << "floatingtext: " << m_pName->text();
        return m_pName->onWidget(p);
    }
    return NULL;
}

/**
 * Reimplement function from UMLWidget
 */
UMLWidget* PortWidget::widgetWithID(Uml::ID::Type id)
{
    if (UMLWidget::widgetWithID(id))
        return this;
    if (m_pName && m_pName->widgetWithID(id))
        return m_pName;
    return NULL;
}


/**
 * Loads from a "portwidget" XMI element.
 */
bool PortWidget::loadFromXMI(QDomElement & qElement)
{
    if (!UMLWidget::loadFromXMI(qElement))
        return false;

    // Optional child element: floatingtext
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    if (!element.isNull()) {
        QString tag = element.tagName();
        if (tag == "floatingtext") {
            m_pName = new FloatingTextWidget(m_scene, Uml::TextRole::Floating,
                                             name(), Uml::ID::Reserved);
            if (!m_pName->loadFromXMI(element)) {
                // Most likely cause: The FloatingTextWidget is empty.
                delete m_pName;
                m_pName = NULL;
            } else {
                m_pName->setParentItem(this);
                m_pName->activate();
                m_pName->update();
            }
        } else {
            uError() << "unknown tag " << tag;
        }
    }
    return true;
}

/**
 * Saves to the "portwidget" XMI element.
 */
void PortWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement conceptElement = qDoc.createElement("portwidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    if (m_pName && !m_pName->text().isEmpty()) {
        m_pName -> saveToXMI(qDoc, conceptElement);
    }
    qElement.appendChild(conceptElement);
}

#include "portwidget.moc"
