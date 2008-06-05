/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "enumwidget.h"

// qt/kde includes
#include <qpainter.h>
#include <kdebug.h>

// app includes
#include "enum.h"
#include "enumliteral.h"
#include "classifier.h"
#include "umlclassifierlistitemlist.h"
#include "classifierlistitem.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "listpopupmenu.h"
#include "object_factory.h"
#include "umlscene.h"


EnumWidget::EnumWidget(UMLScene* view, UMLObject* o) : UMLWidget(view, o) {
    init();
}

void EnumWidget::init() {
    UMLWidget::setBaseType(Uml::wt_Enum);
    setSize(100, 30);
    //set defaults from m_pScene
    if (m_pScene) {
        //check to see if correct
        const Settings::OptionState& ops = m_pScene->getOptionState();
        m_bShowPackage = ops.classState.showPackage;
    } else {
        // For completeness only. Not supposed to happen.
        m_bShowPackage = false;
    }
    if (! UMLApp::app()->getDocument()->loading())
        updateComponentSize();
}

EnumWidget::~EnumWidget() {}

void EnumWidget::draw(QPainter& p, int offsetX, int offsetY) {
    setPenFromSettings(p);
    if(UMLWidget::getUseFillColour())
        p.setBrush(UMLWidget::getFillColour());
    else {
        // [PORT]
        //p.setBrush( m_pScene->viewport()->palette().color(QPalette::Background) );
    }

    const qreal w = getWidth();
    const qreal h = getHeight();

    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const qreal fontHeight  = fm.lineSpacing();
    QString name;
    if ( m_bShowPackage ) {
        name = m_pObject->getFullyQualifiedName();
    } else {
        name = this -> getName();
    }

    p.drawRect(offsetX, offsetY, w, h);
    p.setPen(QPen(Qt::black));

    QFont font = UMLWidget::getFont();
    font.setBold(true);
    p.setFont(font);
    p.drawText(offsetX + ENUM_MARGIN, offsetY,
               w - ENUM_MARGIN * 2,fontHeight,
               Qt::AlignCenter, m_pObject->getStereotype(true));

    font.setItalic( m_pObject -> getAbstract() );
    p.setFont(font);
    p.drawText(offsetX + ENUM_MARGIN, offsetY + fontHeight,
               w - ENUM_MARGIN * 2, fontHeight, Qt::AlignCenter, name);
    font.setBold(false);
    font.setItalic(false);
    p.setFont(font);

    qreal y = fontHeight * 2;

    setPenFromSettings(p);

    p.drawLine(offsetX, offsetY + y, offsetX + w - 1, offsetY + y);

    QFontMetrics fontMetrics(font);
    UMLClassifier *classifier = (UMLClassifier*)m_pObject;
    UMLClassifierListItem* enumLiteral = 0;
    UMLClassifierListItemList list = classifier->getFilteredList(Uml::ot_EnumLiteral);
    foreach (enumLiteral , list ) {
        QString text = enumLiteral->getName();
        p.setPen( QPen(Qt::black) );
        p.drawText(offsetX + ENUM_MARGIN, offsetY + y,
                   fontMetrics.width(text), fontHeight, Qt::AlignVCenter, text);
        y+=fontHeight;
    }

    if (isSelected()) {
        drawSelected(&p, offsetX, offsetY);
    }
}

QSizeF EnumWidget::calculateSize() {
    if (!m_pObject) {
        return UMLWidget::calculateSize();
    }

    qreal width, height;
    QFont font = UMLWidget::getFont();
    font.setItalic(false);
    font.setUnderline(false);
    font.setBold(false);
    const QFontMetrics fm(font);

    const qreal fontHeight = fm.lineSpacing();

    qreal lines = 1;//always have one line - for name
    lines++; //for the stereotype

    const qreal numberOfEnumLiterals = ((UMLEnum*)m_pObject)->enumLiterals();

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
    qreal w = getFontMetrics(FT_BOLD).boundingRect(m_pObject->getStereotype(true)).width();


    width = w > width?w:width;

    UMLClassifier *classifier = (UMLClassifier*)m_pObject;
    UMLClassifierListItemList list = classifier->getFilteredList(Uml::ot_EnumLiteral);
    UMLClassifierListItem* listItem = 0;
    foreach (listItem , list ) {
        qreal w = fm.width( listItem->getName() );
        width = w > width?w:width;
    }

    //allow for width margin
    width += ENUM_MARGIN * 2;

    return QSizeF(width, height);
}

void EnumWidget::slotMenuSelection(QAction* action) {
    ListPopupMenu::Menu_Type sel = m_pMenu->getMenuType(action);
    if (sel == ListPopupMenu::mt_EnumLiteral) {
        if (Object_Factory::createChildObject(static_cast<UMLClassifier*>(m_pObject),
                                              Uml::ot_EnumLiteral) )  {
            /* I don't know why it works without these calls:
            updateComponentSize();
            update();
             */
            UMLApp::app()->getDocument()->setModified();
        }
        return;
    }
    UMLWidget::slotMenuSelection(action);
}

void EnumWidget::setShowPackage(bool _status) {
    m_bShowPackage = _status;
    updateComponentSize();
    update();
}

bool EnumWidget::getShowPackage() const {
    return m_bShowPackage;
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
    updateComponentSize();
    update();

    return;
}

