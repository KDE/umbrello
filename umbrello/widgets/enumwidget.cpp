/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "enumwidget.h"

// app includes
#include "classifier.h"
#include "classifierlistitem.h"
#include "debug_utils.h"
#include "enum.h"
#include "enumliteral.h"
#include "listpopupmenu.h"
#include "object_factory.h"
#include "uml.h"
#include "umlclassifierlistitemlist.h"
#include "umldoc.h"
#include "umlscene.h"


/**
 * Constructs an instance of EnumWidget.
 *
 * @param scene      The parent of this EnumWidget.
 * @param o         The UMLObject this will be representing.
 */
EnumWidget::EnumWidget(UMLScene *scene, UMLObject* o)
  : UMLWidget(scene, WidgetBase::wt_Enum, o),
    m_showPackage(false)
{
    setSize(100, 30);
    //set defaults from m_scene
    if (m_scene) {
        //check to see if correct
        const Settings::OptionState& ops = m_scene->optionState();
        m_showPackage = ops.classState.showPackage;
    } else {
        // For completeness only. Not supposed to happen.
        m_showPackage = false;
    }
}

/**
 * Destructor.
 */
EnumWidget::~EnumWidget()
{
}

/**
 * Returns the status of whether to show Package.
 *
 * @return  True if package is shown.
 */
bool EnumWidget::showPackage() const
{
    return m_showPackage;
}

/**
 * Set the status of whether to show Package.
 *
 * @param _status             True if package shall be shown.
 */
void EnumWidget::setShowPackage(bool _status)
{
    m_showPackage = _status;
    updateComponentSize();
    update();
}

/**
 * Toggles the status of whether to show package.
 */
void EnumWidget::toggleShowPackage()
{
    m_showPackage = !m_showPackage;
    updateComponentSize();
    update();
}

/**
 * Draws the enum as a rectangle with a box underneith with a list of literals
 * Reimplemented from UMLWidget::paint
 */
void EnumWidget::paint(QPainter& p, int offsetX, int offsetY)
{
    setPenFromSettings(p);
    if(UMLWidget::useFillColor())
        p.setBrush(UMLWidget::fillColor());
    else
        p.setBrush( m_scene->viewport()->palette().color(QPalette::Background) );

    const int w = width();
    const int h = height();

    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    QString name;
    if ( m_showPackage ) {
        name = m_pObject->fullyQualifiedName();
    } else {
        name = this->name();
    }

    p.drawRect(offsetX, offsetY, w, h);
    p.setPen(textColor());

    QFont font = UMLWidget::font();
    font.setBold(true);
    p.setFont(font);
    p.drawText(offsetX + ENUM_MARGIN, offsetY,
               w - ENUM_MARGIN * 2,fontHeight,
               Qt::AlignCenter, m_pObject->stereotype(true));

    font.setItalic( m_pObject->isAbstract() );
    p.setFont(font);
    p.drawText(offsetX + ENUM_MARGIN, offsetY + fontHeight,
               w - ENUM_MARGIN * 2, fontHeight, Qt::AlignCenter, name);
    font.setBold(false);
    font.setItalic(false);
    p.setFont(font);

    int y = fontHeight * 2;

    setPenFromSettings(p);

    p.drawLine(offsetX, offsetY + y, offsetX + w - 1, offsetY + y);

    QFontMetrics fontMetrics(font);
    UMLClassifier *classifier = (UMLClassifier*)m_pObject;
    UMLClassifierListItem* enumLiteral = 0;
    UMLClassifierListItemList list = classifier->getFilteredList(UMLObject::ot_EnumLiteral);
    foreach (enumLiteral , list ) {
        QString text = enumLiteral->name();
        p.setPen(textColor());
        p.drawText(offsetX + ENUM_MARGIN, offsetY + y,
                   fontMetrics.width(text), fontHeight, Qt::AlignVCenter, text);
        y+=fontHeight;
    }

    if (m_selected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

/**
 * Loads from an "enumwidget" XMI element.
 */
bool EnumWidget::loadFromXMI( QDomElement & qElement )
{
    if ( !UMLWidget::loadFromXMI(qElement) ) {
        return false;
    }
    QString showpackage = qElement.attribute("showpackage", "0");

    m_showPackage = (bool)showpackage.toInt();

    return true;
}

/**
 * Saves to the "enumwidget" XMI element.
 */
void EnumWidget::saveToXMI( QDomDocument& qDoc, QDomElement& qElement )
{
    QDomElement conceptElement = qDoc.createElement("enumwidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);

    conceptElement.setAttribute("showpackage", m_showPackage);
    qElement.appendChild(conceptElement);
}

/**
 * Will be called when a menu selection has been made from the
 * popup menu.
 *
 * @param action       The action that has been selected.
 */
void EnumWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = m_pMenu->getMenuType(action);
    if (sel == ListPopupMenu::mt_EnumLiteral) {
        if (Object_Factory::createChildObject(static_cast<UMLClassifier*>(m_pObject),
                                              UMLObject::ot_EnumLiteral) )  {
            /* I don't know why it works without these calls:
            updateComponentSize();
            update();
             */
            UMLApp::app()->document()->setModified();
        }
        return;
    }
    UMLWidget::slotMenuSelection(action);
}

/**
 * Overrides method from UMLWidget.
 */
UMLSceneSize EnumWidget::minimumSize()
{
    if (!m_pObject) {
        return UMLWidget::minimumSize();
    }

    int width, height;
    QFont font = UMLWidget::font();
    font.setItalic(false);
    font.setUnderline(false);
    font.setBold(false);
    const QFontMetrics fm(font);

    const int fontHeight = fm.lineSpacing();

    int lines = 1;//always have one line - for name
    lines++; //for the stereotype

    const int numberOfEnumLiterals = ((UMLEnum*)m_pObject)->enumLiterals();

    height = width = 0;
    //set the height of the enum

    lines += numberOfEnumLiterals;
    if (numberOfEnumLiterals == 0) {
        height += fontHeight / 2; //no enum literals, so just add a bit of space
    }

    height += lines * fontHeight;

    //now set the width of the concept
    //set width to name to start with
    if (m_showPackage)  {
        width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(m_pObject->fullyQualifiedName()).width();
    } else {
        width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(name()).width();
    }
    int w = getFontMetrics(FT_BOLD).boundingRect(m_pObject->stereotype(true)).width();


    width = w > width?w:width;

    UMLClassifier *classifier = (UMLClassifier*)m_pObject;
    UMLClassifierListItemList list = classifier->getFilteredList(UMLObject::ot_EnumLiteral);
    UMLClassifierListItem* listItem = 0;
    foreach (listItem , list ) {
        int w = fm.width( listItem->name() );
        width = w > width?w:width;
    }

    //allow for width margin
    width += ENUM_MARGIN * 2;

    return UMLSceneSize(width, height);
}
