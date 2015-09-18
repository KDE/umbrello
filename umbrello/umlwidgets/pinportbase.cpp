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
#include "pinportbase.h"

// app includes
#include "port.h"
#include "package.h"
#include "debug_utils.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "floatingtextwidget.h"

// qt includes
#include <QPainter>
#include <QToolTip>

// sys includes
#include <cmath>

PinPortBase::PinPortBase(UMLScene *scene, WidgetType type, UMLObject *o)
    : UMLWidget(scene, type, o)
{
    init();
}

PinPortBase::PinPortBase(UMLScene *scene, WidgetType type, UMLWidget *a, Uml::ID::Type id)
    : UMLWidget(scene, type, id)
{
    init(a);
}

/**
 * Standard destructor.
 */
PinPortBase::~PinPortBase()
{
}

/**
 * Performs initializations which are common to PinWidget and PortWidget.
 */
void PinPortBase::init(UMLWidget *owner)
{
    m_ignoreSnapToGrid = true;
    m_ignoreSnapComponentSizeToGrid = true;
    m_resizable = false;
    m_pOw = owner;
    m_pName = NULL;
    m_motionConnected = false;
    const int edgeLength = 15;  // old: (m_baseType == wt_Pin ? 10 : 15);
    const QSizeF FixedSize(edgeLength, edgeLength);
    setMinimumSize(FixedSize);
    setMaximumSize(FixedSize);
    setSize(FixedSize);
}

UMLWidget* PinPortBase::ownerWidget()
{
    return m_pOw;
}

/**
 * Overrides method from UMLWidget in order to set a tooltip.
 * The tooltip is set to the name().
 * The reason for using a tooltip for the name is that the size of this
 * widget is not large enough to accommodate the average name.
 */
void PinPortBase::updateWidget()
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
 * Overrides method from UMLWidget to set the name.
 */
void PinPortBase::setName(const QString &strName)
{
    UMLWidget::setName(strName);
    updateGeometry();
    if (m_pName) {
        m_pName->setText(strName);
    }
}

/**
 * Overridden from UMLWidget.
 * Moves the widget to a new position using the difference between the
 * current position and the new position.
 * Movement is constrained such that the port is always attached to its
 * owner widget.
 *
 * @param diffX The difference between current X position and new X position.
 * @param diffY The difference between current Y position and new Y position.
 */
void PinPortBase::moveWidgetBy(qreal diffX, qreal diffY)
{
    UMLWidget* owner = ownerWidget();
    qreal newX = x() + diffX;
    qreal newY = y() + diffY;
    uDebug() << "PinPortBase::moveWidgetBy " << diffX << "," << diffY;
    if (owner == NULL) {
        uError() << "PinPortBase::moveWidgetBy: ownerWidget() returns NULL";
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
 * Align this widget's position such that it is attached at one of the
 * sides of its owner's widget.
 */
void PinPortBase::attachToOwner() {
    UMLWidget *owner = ownerWidget();
    const QPointF scenePos = m_scene->pos();
    if (owner == NULL) {
        uError() << "PinPortBase::attachToOwner: ownerWidget() returns NULL";
        setX(scenePos.x());
        setY(scenePos.y());
        return;
    }
    bool xIsWithinOwner = false;
    if (scenePos.x() < owner->x() - width()) {
        setX(owner->x() - width());
    } else if (scenePos.x() <= owner->x() + owner->width()) {
        setX(scenePos.x());
        xIsWithinOwner = true;
    } else {
        setX(owner->x() + owner->width());
    }
    if (scenePos.y() < owner->y() - height()) {
        setY(owner->y() - height());
    } else if (scenePos.y() <= owner->y() + owner->height()) {
        if (xIsWithinOwner) {
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

    if (m_motionConnected) {
        uDebug() << "connectOwnerMotion was already done";
    } else {
        connectOwnerMotion();
        m_motionConnected = true;
    }
}

/**
 * Overrides standard method.
 */
void PinPortBase::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    setPenFromSettings(painter);
    if (UMLWidget::useFillColor())  {
        painter->setBrush(UMLWidget::fillColor());
    } else {
        painter->setBrush(m_scene->backgroundColor());
    }

    int w = width();
    int h = height();

    painter->drawRect(0, 0, w, h);

    UMLWidget::paint(painter, option, widget);
}

void PinPortBase::slotOwnerMoved(qreal diffX, qreal diffY)
{
    setX(x() + diffX);
    setY(y() + diffY);
}

/**
 * Captures any popup menu signals for menus it created.
 */
void PinPortBase::slotMenuSelection(QAction* action)
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
                uError() << "PinPortBase::slotMenuSelection: ownerWidget() returns NULL";
                setX(x());
                setY(y());
            } else {
                const qreal w = width();
                const qreal h = height();
                if (x() < owner->x())
                    m_pName->setX(-m_pName->width());
                else if (x() >= owner->x() + owner->width())
                    m_pName->setX(w);
                else
                    m_pName->setX(-m_pName->width() / 2.0 + w / 2.0);
                if (y() < owner->y())
                    m_pName->setY(-m_pName->height() - 2);
                else if (y() >= owner->y() + owner->height())
                    m_pName->setY(h);
                else
                    m_pName->setY(-m_pName->height() / 2.0 + h / 2.0);
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

FloatingTextWidget *PinPortBase::floatingTextWidget() {
    return m_pName;
}

void PinPortBase::setFloatingTextWidget(FloatingTextWidget *ft) {
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
UMLWidget* PinPortBase::onWidget(const QPointF &p)
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
UMLWidget* PinPortBase::widgetWithID(Uml::ID::Type id)
{
    if (UMLWidget::widgetWithID(id))
        return this;
    if (m_pName && m_pName->widgetWithID(id))
        return m_pName;
    return NULL;
}


/**
 * Saves the widget to the "pinwidget" or "portwidget" XMI element.
 */
void PinPortBase::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement element = qDoc.createElement(m_baseType == wt_Pin ? QLatin1String("pinwidget")
                                                                  : QLatin1String("portwidget"));
    element.setAttribute(QLatin1String("widgetaid"), Uml::ID::toString(ownerWidget()->id()));
    UMLWidget::saveToXMI(qDoc, element);
    if (m_pName && !m_pName->text().isEmpty()) {
        m_pName->saveToXMI(qDoc, element);
    }
    qElement.appendChild(element);
}

/**
 * Loads from a "pinwidget" or from a "portwidget" XMI element.
 */
bool PinPortBase::loadFromXMI(QDomElement & qElement)
{
    if (!UMLWidget::loadFromXMI(qElement))
        return false;

    QString widgetaid = qElement.attribute(QLatin1String("widgetaid"), QLatin1String("-1"));
    Uml::ID::Type aId = Uml::ID::fromString(widgetaid);
    UMLWidget *owner = m_scene->findWidget(aId);
    if (owner == NULL) {
        DEBUG(DBG_SRC) << "owner object " << Uml::ID::toString(aId) << " not found";
        return false;
    }
    m_pOw = owner;

    // Optional child element: floatingtext
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    if (!element.isNull()) {
        QString tag = element.tagName();
        if (tag == QLatin1String("floatingtext")) {
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

    if (m_motionConnected) {
        uDebug() << "connectOwnerMotion was already done";
    } else {
        connectOwnerMotion();
        m_motionConnected = true;
    }
    return true;
}

