/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "entitywidget.h"

// qt/kde includes
#include <qpainter.h>
#include <kdebug.h>

// app includes
#include "entity.h"
#include "entityattribute.h"
#include "classifier.h"
#include "umlclassifierlistitemlist.h"
#include "classifierlistitem.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "listpopupmenu.h"
#include "object_factory.h"


EntityWidget::EntityWidget(UMLView* view, UMLObject* o): UMLWidget(view, o) {
    init();
}

void EntityWidget::init() {
    UMLWidget::setBaseType(Uml::wt_Entity);
    setSize(100, 30);
    //set defaults from m_pView
    if (m_pView) {
        //check to see if correct
        const Settings::OptionState& ops = m_pView->getOptionState();
    }
    if (! UMLApp::app()->getDocument()->loading())
        updateComponentSize();
}

EntityWidget::~EntityWidget() {}

void EntityWidget::draw(QPainter& p, int offsetX, int offsetY) {
    UMLWidget::setPen(p);
    if(UMLWidget::getUseFillColour())
        p.setBrush(UMLWidget::getFillColour());
    else
        p.setBrush(m_pView -> viewport() -> backgroundColor());

    const int w = width();
    const int h = height();

    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    int fontHeight  = fm.lineSpacing();
    const QString name = this->getName();

    p.drawRect(offsetX, offsetY, w, h);
    p.setPen(QPen(Qt::black));

    QFont font = UMLWidget::getFont();
    font.setBold(true);
    p.setFont(font);
    int y = 0;
    if ( !m_pObject->getStereotype().isEmpty() ) {
        p.drawText(offsetX + ENTITY_MARGIN, offsetY,
                   w - ENTITY_MARGIN * 2,fontHeight,
                   Qt::AlignCenter, m_pObject->getStereotype(true));
        font.setItalic( m_pObject -> getAbstract() );
        p.setFont(font);
        p.drawText(offsetX + ENTITY_MARGIN, offsetY + fontHeight,
                   w - ENTITY_MARGIN * 2, fontHeight, Qt::AlignCenter, name);
        font.setBold(false);
        font.setItalic(false);
        p.setFont(font);
        y = fontHeight * 2;
    } else {
        font.setItalic( m_pObject -> getAbstract() );
        p.setFont(font);
        p.drawText(offsetX + ENTITY_MARGIN, offsetY,
                   w - ENTITY_MARGIN * 2, fontHeight, Qt::AlignCenter, name);
        font.setBold(false);
        font.setItalic(false);
        p.setFont(font);

        y = fontHeight;
    }

    UMLWidget::setPen(p);

    p.drawLine(offsetX, offsetY + y, offsetX + w - 1, offsetY + y);

    QFontMetrics fontMetrics(font);
    UMLClassifier *classifier = (UMLClassifier*)m_pObject;
    UMLClassifierListItem* entityattribute = 0;
    UMLClassifierListItemList list = classifier->getFilteredList(Uml::ot_EntityAttribute);
    for (entityattribute = list.first(); entityattribute; entityattribute = list.next()) {
        QString text = entityattribute->getName();
        p.setPen( QPen(Qt::black) );
        UMLEntityAttribute* casted = dynamic_cast<UMLEntityAttribute*>( entityattribute );
        if( casted && casted->getIndexType() == Uml::Primary )
        {
            font.setUnderline( true );
            p.setFont( font );
            font.setUnderline( false );
        }
        p.drawText(offsetX + ENTITY_MARGIN, offsetY + y,
                   fontMetrics.width(text), fontHeight, Qt::AlignVCenter, text);
        p.setFont( font );
        y+=fontHeight;
    }

    if (m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

QSize EntityWidget::calculateSize() {
    if (!m_pObject) {
        return UMLWidget::calculateSize();
    }

    int width, height;
    QFont font = UMLWidget::getFont();
    font.setItalic(false);
    font.setUnderline(false);
    font.setBold(false);
    const QFontMetrics fm(font);

    const int fontHeight = fm.lineSpacing();

    int lines = 1;//always have one line - for name
    if ( !m_pObject->getStereotype().isEmpty() ) {
        lines++;
    }

    const int numberOfEntityAttributes = ((UMLEntity*)m_pObject)->entityAttributes();

    height = width = 0;
    //set the height of the entity

    lines += numberOfEntityAttributes;
    if (numberOfEntityAttributes == 0) {
        height += fontHeight / 2; //no entity literals, so just add a bit of space
    }

    height += lines * fontHeight;

    //now set the width of the concept
    //set width to name to start with
    // FIXME spaces to get round beastie with font width,
    // investigate UMLWidget::getFontMetrics()
    width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(' ' + getName() + ' ').width();

    const int w = getFontMetrics(FT_BOLD).boundingRect(m_pObject->getStereotype(true)).width();

    width = w > width?w:width;

    UMLClassifier* classifier = (UMLClassifier*)m_pObject;
    UMLClassifierListItemList list = classifier->getFilteredList(Uml::ot_EntityAttribute);
    UMLClassifierListItem* listItem = 0;
    for (listItem = list.first(); listItem; listItem = list.next()) {
        int w = fm.width( listItem->getName() );
        width = w > width?w:width;
    }

    //allow for width margin
    width += ENTITY_MARGIN * 2;

    return QSize(width, height);
}

void EntityWidget::slotMenuSelection(int sel) {
    switch(sel) {
    case ListPopupMenu::mt_EntityAttribute:
        if (Object_Factory::createChildObject(static_cast<UMLClassifier*>(m_pObject),
                                              Uml::ot_EntityAttribute) )  {
            UMLApp::app()->getDocument()->setModified();
        }
        break;
    }
    UMLWidget::slotMenuSelection(sel);
}

void EntityWidget::saveToXMI( QDomDocument& qDoc, QDomElement& qElement ) {
    QDomElement conceptElement = qDoc.createElement("entitywidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

bool EntityWidget::loadFromXMI( QDomElement & qElement ) {
    if ( !UMLWidget::loadFromXMI(qElement) ) {
        return false;
    }
    return true;
}

