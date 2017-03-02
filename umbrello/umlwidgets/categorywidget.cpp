/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header file
#include "categorywidget.h"

// local includes
#include "category.h"
#include "debug_utils.h"
#include "umlview.h"
#include "listpopupmenu.h"

// qt includes
#include <QPainter>

/**
 *  Creates a Category widget.
 *
 *  @param  scene  The parent of the widget.
 *  @param  o      The UMLCategory to represent.
 */
CategoryWidget::CategoryWidget(UMLScene * scene, UMLCategory *o)
  : UMLWidget(scene, WidgetBase::wt_Category, o)
{
    m_fixedAspectRatio = true;
}

/**
 * Destructor.
 */
CategoryWidget::~CategoryWidget()
{
}

/**
 *   Overrides the standard paint event.
 */
void CategoryWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    UMLWidget::setPenFromSettings(painter);
    if (UMLWidget::useFillColor())
        painter->setBrush(UMLWidget::fillColor());
    QFont font = UMLWidget::font();
    font.setUnderline(false);
    font.setBold(false);
    font.setItalic(m_umlObject->isAbstract());
    painter->setFont(font);
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    // the height is our radius
    const int h = height();
    const int w = width();
    const int r = h > w ? h : w;

    //int middleX = w / 2;
    const int textStartY = (r / 2) - (fontHeight / 2);

    // draw a circle
    painter->drawEllipse(0, 0, r, r);
    painter->setPen(textColor());

    QString letterType(QLatin1Char('D'));
    switch(m_umlObject->asUMLCategory()->getType()) {
       case UMLCategory::ct_Disjoint_Specialisation:
           letterType = QLatin1Char('D');
           break;
       case UMLCategory::ct_Overlapping_Specialisation:
           letterType = QLatin1Char('O');
           break;
       case UMLCategory::ct_Union:
           letterType = QLatin1Char('U');
           break;
       default:
           break;
    }

    painter->drawText(UC_MARGIN, textStartY, r - UC_MARGIN * 2, fontHeight, Qt::AlignCenter, letterType);
    UMLWidget::setPenFromSettings(painter);

    UMLWidget::paint(painter, option, widget);
}

/**
 * Overrides method from UMLWidget.
 */
QSizeF CategoryWidget::minimumSize() const
{
    const UMLWidget::FontType ft = (m_umlObject->isAbstract() ? FT_BOLD_ITALIC : FT_BOLD);
    const QFontMetrics &fm = UMLWidget::getFontMetrics(ft);
    const int fontHeight = fm.lineSpacing();
    int radius = UC_RADIUS + fontHeight + UC_MARGIN;

    return QSizeF(radius, radius);
}

/**
 * Saves this Category to file.
 */
void CategoryWidget::saveToXMI1(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement categoryElement = qDoc.createElement(QLatin1String("categorywidget"));
    UMLWidget::saveToXMI1(qDoc, categoryElement);
    qElement.appendChild(categoryElement);
}

/**
 * Will be called when a menu selection has been made from the
 * popup menu.
 *
 * @param action    The action that has been selected.
 */
void CategoryWidget::slotMenuSelection(QAction* action)
{
    UMLCategory* catObj = umlObject()->asUMLCategory();
    if (!catObj) {
        uWarning() << "No UMLCategory for this widget.";
        return;
    }

    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
      case ListPopupMenu::mt_DisjointSpecialisation:
          catObj->setType(UMLCategory::ct_Disjoint_Specialisation);
          break;

      case ListPopupMenu::mt_OverlappingSpecialisation:
          catObj->setType(UMLCategory::ct_Overlapping_Specialisation);
          break;

      case ListPopupMenu::mt_Union:
          catObj->setType(UMLCategory::ct_Union);
          break;

      default:
          UMLWidget::slotMenuSelection(action);
    }
}

