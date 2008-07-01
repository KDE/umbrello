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


EnumWidget::EnumWidget(UMLScene* view, UMLObject* o) : NewUMLRectWidget(view, o) {
    init();
}

void EnumWidget::init() {
    NewUMLRectWidget::setBaseType(Uml::wt_Enum);
    setSize(100, 30);
    //set defaults from umlScene()
    if (umlScene()) {
        //check to see if correct
        const Settings::OptionState& ops = umlScene()->getOptionState();
        m_bShowPackage = ops.classState.showPackage;
    } else {
        // For completeness only. Not supposed to happen.
        m_bShowPackage = false;
    }
    if (! UMLApp::app()->getDocument()->loading())
        updateComponentSize();
}

EnumWidget::~EnumWidget() {}

void EnumWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
	QPainter &p = *painter;
	qreal offsetX = 0, offsetY = 0;

    setPenFromSettings(p);
    if(NewUMLRectWidget::getUseFillColour())
        p.setBrush(NewUMLRectWidget::getFillColour());
    else {
        // [PORT]
        //p.setBrush( umlScene()->viewport()->palette().color(QPalette::Background) );
    }

    const qreal w = getWidth();
    const qreal h = getHeight();

    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const qreal fontHeight  = fm.lineSpacing();
    QString name;
    if ( m_bShowPackage ) {
        name = umlObject()->getFullyQualifiedName();
    } else {
        name = this->getName();
    }

    p.drawRect(offsetX, offsetY, w, h);
    p.setPen(QPen(Qt::black));

    QFont font = NewUMLRectWidget::getFont();
    font.setBold(true);
    p.setFont(font);
    p.drawText(offsetX + ENUM_MARGIN, offsetY,
               w - ENUM_MARGIN * 2,fontHeight,
               Qt::AlignCenter, umlObject()->getStereotype(true));

    font.setItalic( umlObject()->getAbstract() );
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
    UMLClassifier *classifier = (UMLClassifier*)umlObject();
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
    if (!umlObject()) {
        return NewUMLRectWidget::calculateSize();
    }

    qreal width, height;
    QFont font = NewUMLRectWidget::getFont();
    font.setItalic(false);
    font.setUnderline(false);
    font.setBold(false);
    const QFontMetrics fm(font);

    const qreal fontHeight = fm.lineSpacing();

    qreal lines = 1;//always have one line - for name
    lines++; //for the stereotype

    const qreal numberOfEnumLiterals = ((UMLEnum*)umlObject())->enumLiterals();

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
        width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(umlObject()->getFullyQualifiedName()).width();
    } else {
        width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(getName()).width();
    }
    qreal w = getFontMetrics(FT_BOLD).boundingRect(umlObject()->getStereotype(true)).width();


    width = w > width?w:width;

    UMLClassifier *classifier = (UMLClassifier*)umlObject();
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
        if (Object_Factory::createChildObject(static_cast<UMLClassifier*>(umlObject()),
                                              Uml::ot_EnumLiteral) )  {
            /* I don't know why it works without these calls:
            updateComponentSize();
            update();
             */
            UMLApp::app()->getDocument()->setModified();
        }
        return;
    }
    NewUMLRectWidget::slotMenuSelection(action);
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
    NewUMLRectWidget::saveToXMI(qDoc, conceptElement);

    conceptElement.setAttribute("showpackage", m_bShowPackage);
    qElement.appendChild(conceptElement);
}

bool EnumWidget::loadFromXMI( QDomElement & qElement ) {
    if ( !NewUMLRectWidget::loadFromXMI(qElement) ) {
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

