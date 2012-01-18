/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "forkjoinwidget.h"

//app includes
#include "debug_utils.h"
#include "umlview.h"
#include "listpopupmenu.h"

/**
 * Constructs a ForkJoinWidget.
 *
 * @param scene          The parent to this widget.
 * @param drawVertical  Whether to draw the plate horizontally or vertically.
 * @param id            The ID to assign (-1 will prompt a new ID.)
 */
ForkJoinWidget::ForkJoinWidget(UMLScene * scene, bool drawVertical, Uml::IDType id)
  : BoxWidget(scene, id, WidgetBase::wt_ForkJoin),
    m_drawVertical(drawVertical)
{
}

/**
 * Destructor.
 */
ForkJoinWidget::~ForkJoinWidget()
{
}

/**
 * Get whether to draw the plate vertically.
 */
bool ForkJoinWidget::getDrawVertical() const {
    return m_drawVertical;
}

/**
 * Set whether to draw the plate vertically.
 */
void ForkJoinWidget::setDrawVertical(bool to) {
    m_drawVertical = to;
    updateComponentSize();
    UMLWidget::adjustAssocs( getX(), getY() );
}

/**
 * Reimplemented from UMLWidget::paint to draw the plate of
 * fork join.
 */
void ForkJoinWidget::paint(QPainter& p, int offsetX, int offsetY)
{
    p.fillRect( offsetX, offsetY, width(), height(), QBrush( Qt::black ));

    if (m_selected) {
        drawSelected(&p, offsetX, offsetY);
    }
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
    QString drawVertical = qElement.attribute("drawvertical", "0");
    setDrawVertical( (bool)drawVertical.toInt() );
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
    fjElement.setAttribute("drawvertical", m_drawVertical);
    qElement.appendChild(fjElement);
}

/**
 * Reimplemented form UMLWidget::slotMenuSelection to handle
 * Flip action.
 */
void ForkJoinWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = m_pMenu->getMenuType(action);
    switch (sel) {
    case ListPopupMenu::mt_Flip:
        setDrawVertical(!m_drawVertical);
        break;
    default:
        break;
    }
}

/**
 * Overrides the function from UMLWidget.
 */
UMLSceneSize ForkJoinWidget::minimumSize()
{
    if (m_drawVertical) {
        return UMLSceneSize(4, 40);
    } else {
        return UMLSceneSize(40, 4);
    }
}

/**
 * Reimplement method from UMLWidget to suppress the resize corner.
 * Although the ForkJoinWidget supports resizing, we suppress the
 * resize corner because it is too large for this very slim widget.
 */
void ForkJoinWidget::drawSelected(QPainter *, int /*offsetX*/, int /*offsetY*/)
{
}

/**
 * Reimplement method from UMLWidget.
 */
void ForkJoinWidget::constrain(int& width, int& height)
{
    if (m_drawVertical) {
        if (width < 4)
            width = 4;
        else if (width > 10)
            width = 10;
        if (height < 40)
            height = 40;
        else if (height > 100)
            height = 100;
    } else {
        if (height < 4)
            height = 4;
        else if (height > 10)
            height = 10;
        if (width < 40)
            width = 40;
        else if (width > 100)
            width = 100;
    }
}
