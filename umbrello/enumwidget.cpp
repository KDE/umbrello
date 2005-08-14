/*
 *  copyright (C) 2003-2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "enumwidget.h"
#include "enum.h"
#include "enumliteral.h"
#include "classifier.h"
#include "umlclassifierlistitemlist.h"
#include "classifierlistitem.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "listpopupmenu.h"

#include <kdebug.h>
#include <qpainter.h>

EnumWidget::EnumWidget(UMLView* view, UMLObject* o) : UMLWidget(view, o) {
    init();
    if ( ! UMLApp::app()->getDocument()->loading() )
    { // set default size - but only if we aren't loading a XMI file at the
        // moment - then just recreate the saved settings
        setSize(100,30);
        calculateSize();
    }
}

void EnumWidget::init() {
    UMLWidget::setBaseType(Uml::wt_Enum);
    m_pMenu = 0;
    //set defaults from m_pView
    if (m_pView) {
        //check to see if correct
        const Settings::OptionState& ops = m_pView->getOptionState();
        m_bShowPackage = ops.classState.showPackage;
    } else {
        // For completeness only. Not supposed to happen.
        m_bShowPackage = false;
    }
}

EnumWidget::~EnumWidget() {}

void EnumWidget::draw(QPainter& p, int offsetX, int offsetY) {
    UMLWidget::setPen(p);
    if(UMLWidget::getUseFillColour())
        p.setBrush(UMLWidget::getFillColour());
    else
        p.setBrush(m_pView -> viewport() -> backgroundColor());

    int w = width();
    int h = height();

    QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    int fontHeight  = fm.lineSpacing();
    QString name;
    if ( m_bShowPackage ) {
        name = m_pObject->getFullyQualifiedName();
    } else {
        name = this -> getName();
    }

    p.drawRect(offsetX, offsetY, w, h);
    p.setPen(QPen(black));

    QFont font = UMLWidget::getFont();
    font.setBold(true);
    p.setFont(font);
    p.drawText(offsetX + ENUM_MARGIN, offsetY,
               w - ENUM_MARGIN * 2,fontHeight,
               AlignCenter, m_pObject -> getStereotype());

    font.setItalic( m_pObject -> getAbstract() );
    p.setFont(font);
    p.drawText(offsetX + ENUM_MARGIN, offsetY + fontHeight,
               w - ENUM_MARGIN * 2, fontHeight, AlignCenter, name);
    font.setBold(false);
    font.setItalic(false);
    p.setFont(font);

    int y = fontHeight * 2;

    UMLWidget::setPen(p);

    p.drawLine(offsetX, offsetY + y, offsetX + w - 1, offsetY + y);

    QFontMetrics fontMetrics(font);
    UMLClassifier *classifier = (UMLClassifier*)m_pObject;
    UMLClassifierListItem* enumLiteral = 0;
    UMLClassifierListItemList list = classifier->getFilteredList(Uml::ot_EnumLiteral);
    for (enumLiteral = list.first(); enumLiteral; enumLiteral = list.next()) {
        QString text = enumLiteral->getName();
        p.setPen( QPen(black) );
        p.drawText(offsetX + ENUM_MARGIN, offsetY + y,
                   fontMetrics.width(text), fontHeight, AlignVCenter, text);
        y+=fontHeight;
    }

    if (m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

void EnumWidget::calculateSize() {
    if (!m_pObject)  {
        return;
    }

    int width, height;
    QFont font = UMLWidget::getFont();
    font.setItalic(false);
    font.setUnderline(false);
    font.setBold(false);
    QFontMetrics fm(font);

    int fontHeight = fm.lineSpacing();

    int lines = 1;//always have one line - for name
    lines++; //for the stereotype

    int numberOfEnumLiterals = ((UMLEnum*)m_pObject)->enumLiterals();

    height = width = 0;
    //set the height of the enum

    lines += numberOfEnumLiterals;
    if (numberOfEnumLiterals == 0) {
        height += fontHeight / 2; //no enum literals, so just add a bit of space
    }

    height += lines * fontHeight;

    //now set the width of the concept
    //set width to name to start with
    if (m_bShowPackage)  {
        width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(m_pObject->getFullyQualifiedName()).width();
    } else {
        width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(getName()).width();
    }
    int w = getFontMetrics(FT_BOLD).boundingRect("«" + m_pObject->getStereotype() + "»").width();


    width = w > width?w:width;

    UMLClassifier *classifier = (UMLClassifier*)m_pObject;
    UMLClassifierListItemList list = classifier->getFilteredList(Uml::ot_EnumLiteral);
    UMLClassifierListItem* listItem = 0;
    for (listItem = list.first(); listItem; listItem = list.next()) {
        int w = fm.width( listItem->getName() );
        width = w > width?w:width;
    }

    //allow for width margin
    width += ENUM_MARGIN * 2;

    setSize(width, height);
    adjustAssocs( getX(), getY() );//adjust assoc lines
}

void EnumWidget::slotMenuSelection(int sel) {
    switch(sel) {
    case ListPopupMenu::mt_EnumLiteral:
        if ( UMLApp::app()->getDocument()->createChildObject(m_pObject, Uml::ot_EnumLiteral) )  {
            UMLApp::app()->getDocument()->setModified();
        }
        break;
    }
    UMLWidget::slotMenuSelection(sel);
}

void EnumWidget::setShowPackage(bool _status) {
    m_bShowPackage = _status;
    calculateSize();
    update();
}

bool EnumWidget::getShowPackage() const {
    return m_bShowPackage;
}

bool EnumWidget::activate(IDChangeLog* ChangeLog /* = 0 */) {
    bool status = UMLWidget::activate(ChangeLog);
    if (status) {
        calculateSize();
    }
    return status;
}

void EnumWidget::saveToXMI( QDomDocument& qDoc, QDomElement& qElement ) {
    QDomElement conceptElement = qDoc.createElement("enumwidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);

    conceptElement.setAttribute("showpackage", m_bShowPackage);
    qElement.appendChild(conceptElement);
}

bool EnumWidget::loadFromXMI( QDomElement & qElement ) {
    if ( !UMLWidget::loadFromXMI(qElement) ) {
        return false;
    }
    QString showpackage = qElement.attribute("showpackage", "0");

    m_bShowPackage = (bool)showpackage.toInt();

    return true;
}

void EnumWidget::toggleShowPackage() {
    m_bShowPackage = !m_bShowPackage;
    calculateSize();
    update();

    return;
}

