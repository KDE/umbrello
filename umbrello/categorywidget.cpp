/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header file
#include "categorywidget.h"
// system includes
#include <qpainter.h>
#include <kdebug.h>
// local includes
#include "category.h"
#include "umlview.h"
#include "listpopupmenu.h"

CategoryWidget::CategoryWidget(UMLScene * scene, UMLCategory *o) : NewUMLRectWidget(scene, o)
{
    NewUMLRectWidget::setBaseType(Uml::wt_Category);
    //updateComponentSize();  Doing this during loadFromXMI() gives futile updates.
    //                  Instead, it is done afterwards by NewUMLRectWidget::activate()
}

CategoryWidget::~CategoryWidget() {}

void CategoryWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
	QPainter &p = *painter;
	qreal offsetX = 0, offsetY = 0;

    NewUMLRectWidget::setPenFromSettings(p);
    if ( NewUMLRectWidget::getUseFillColour() )
        p.setBrush( NewUMLRectWidget::getFillColour() );
    QFont font = NewUMLRectWidget::getFont();
    font.setUnderline(false);
    font.setBold(false);
    font.setItalic( umlObject()->getAbstract() );
    p.setFont( font );
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const qreal fontHeight  = fm.lineSpacing();
    // the height is our radius
    const qreal h = getHeight();
    const qreal w = getWidth();
    const qreal r = h > w ? h : w;

    //qreal middleX = w / 2;
    const qreal textStartY = (r / 2) - (fontHeight / 2);

    // draw a circle
    p.drawEllipse(offsetX, offsetY, r, r);
    p.setPen(Qt::black);

    QString letterType('D');
    switch( static_cast<UMLCategory*>( umlObject() )->getType() ) {
       case UMLCategory::ct_Disjoint_Specialisation:
           letterType = 'D';
           break;
       case UMLCategory::ct_Overlapping_Specialisation:
           letterType = 'O';
           break;
       case UMLCategory::ct_Union:
           letterType = 'U';
           break;
       default:
           break;
    }

    p.drawText(offsetX + UC_MARGIN, offsetY + textStartY, r - UC_MARGIN * 2, fontHeight, Qt::AlignCenter, letterType );
    NewUMLRectWidget::setPenFromSettings(p);
    if(isSelected()) {
        drawSelected(&p, offsetX, offsetY);
    }
}

QSizeF CategoryWidget::calculateSize() {
    const NewUMLRectWidget::FontType ft = ( umlObject()->getAbstract() ? FT_BOLD_ITALIC : FT_BOLD );
    const QFontMetrics &fm = NewUMLRectWidget::getFontMetrics(ft);
    const qreal fontHeight = fm.lineSpacing();
    qreal radius = UC_RADIUS + fontHeight + UC_MARGIN;

    return QSizeF(radius, radius);
}

void CategoryWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement categoryElement = qDoc.createElement( "categorywidget" );
    NewUMLRectWidget::saveToXMI( qDoc, categoryElement );
    qElement.appendChild( categoryElement );
}

void CategoryWidget::slotMenuSelection(QAction* action){
    UMLCategory* catObj = static_cast<UMLCategory*>(umlObject());
    ListPopupMenu::Menu_Type sel = m_pMenu->getMenuType(action);
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
          NewUMLRectWidget::slotMenuSelection(action);
    }
}

