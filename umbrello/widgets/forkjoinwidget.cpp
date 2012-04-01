/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "forkjoinwidget.h"

//app includes
#include "debug_utils.h"
#include "listpopupmenu.h"

/**
 * Constructs a ForkJoinWidget.
 * @param ori   The orientation of ForkJoinWidget.
 * @param id    ID of the widget. (-1 for new id)
 */
ForkJoinWidget::ForkJoinWidget(Qt::Orientation ori, Uml::IDType id)
  : BoxWidget(id, WidgetBase::wt_ForkJoin),
    m_orientation(ori)
{
    setMargin(0);
    setBrush(QBrush(Qt::black));
}

/**
 * Destructor.
 */
ForkJoinWidget::~ForkJoinWidget()
{
}

/**
 * Get whether to draw the plate vertically or horizontally.
 */
Qt::Orientation ForkJoinWidget::orientation() const
{
    return m_orientation;
}

/**
 * Set whether to draw the plate vertically or horizontally.
 */
void ForkJoinWidget::setOrientation(Qt::Orientation ori)
{
    m_orientation = ori;
    updateGeometry();
}

/**
 * Reimplemented from UMLWidget::paint to draw the plate of
 * fork join.
 */
void ForkJoinWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->fillRect(rect(), brush());
}

/**
 * Reimplemented from UMLWidget::loadFromXMI to load widget
 * info from XMI element - 'forkjoin'.
 */
bool ForkJoinWidget::loadFromXMI(QDomElement& qElement)
{
    if ( !UMLWidget::loadFromXMI(qElement) ) {
        return false;
    }

    QString drawVerticalStr = qElement.attribute("drawvertical", "0");
    bool drawVertical = (bool)drawVerticalStr.toInt();
    if (drawVertical) {
        setOrientation(Qt::Vertical);
    }
    else {
        setOrientation(Qt::Horizontal);
    }

    return true;
}

/**
 * Reimplemented from UMLWidget::saveToXMI to save widget info
 * into XMI element - 'forkjoin'.
 */
void ForkJoinWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement fjElement = qDoc.createElement("forkjoin");
    UMLWidget::saveToXMI(qDoc, fjElement);
    bool drawVertical = true;
    if (m_orientation == Qt::Horizontal) {
        drawVertical = false;
    }
    fjElement.setAttribute("drawvertical", drawVertical);
    qElement.appendChild(fjElement);
}

/**
 * Reimplemented form UMLWidget::slotMenuSelection to handle
 * Flip action.
 */
void ForkJoinWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu *menu = ListPopupMenu::menuFromAction(action);
    if (!menu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }
    if (menu->getMenuType(action) == ListPopupMenu::mt_Flip) {
        setOrientation(m_orientation == Qt::Horizontal ?
                       Qt::Vertical : Qt::Horizontal);
    }
    else {
        BoxWidget::slotMenuSelection(action);
    }
}

/**
 * Reimplemented from UMLWidget::updateGeometry to update the
 * minimum and maximum size of the widget based on current
 * orientation.
 */
void ForkJoinWidget::updateGeometry()
{
    if(m_orientation == Qt::Horizontal) {
        setMinimumSize(QSizeF(40, 4));
        setMaximumSize(QSizeF(100, 10));
    }
    else {
        setMinimumSize(QSizeF(4, 40));
        setMaximumSize(QSizeF(10, 100));
    }

    BoxWidget::updateGeometry();
}

#include "forkjoinwidget.moc"
