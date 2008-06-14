/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "newumlwidget.h"

#include <kdebug.h>

#include "umlobject.h"
#include "umlscene.h"
#include "uml.h"
#include "widget_utils.h"

/**
 * @short A class to encapsulate some properties to be stored as a
 * part of Lazy initialization.
 *
 * The properties encapsulated here are used only if the widget
 * does not have UMLObject representation.
 */
struct WidgetInterfaceData
{
    WidgetInterfaceData() : id(Uml::id_None)
    {
    }

    Uml::IDType id;
    QString documentation;
    QString name;
};

NewUMLWidget::NewUMLWidget(UMLObject *object) :
    QObject(),
    QGraphicsItem(0),

    m_umlObject(object),
    m_widgetInterfaceData(0)
{
    if(!object) {
        m_widgetInterfaceData = new WidgetInterfaceData;
    }
    setFlags(ItemIsSelectable | ItemIsMovable);
}

NewUMLWidget::~NewUMLWidget()
{
    delete m_widgetInterfaceData;
}

UMLObject* NewUMLWidget::umlObject() const
{
    return m_umlObject;
}

void NewUMLWidget::setUmlObject(UMLObject *obj)
{
    m_umlObject = obj;
    // If obj is not null, then we will be using objects properties.
    if(obj) {
        delete m_widgetInterfaceData;
    }
}

Uml::IDType NewUMLWidget::id() const
{
    if(m_umlObject) {
        return m_umlObject->getID();
    }
    return m_widgetInterfaceData->id;
}

void NewUMLWidget::setId(Uml::IDType id)
{
    if(m_umlObject) {

        if (m_umlObject->getID() != Uml::id_None) {
            uWarning() << "changing old UMLObject " << ID2STR(m_umlObject->getID())
                       << " to " << ID2STR(id) << endl;
        }

        m_umlObject->setID(id);
    }
    else {
        m_widgetInterfaceData->id = id;
    }
}

Uml::Widget_Type NewUMLWidget::baseType() const
{
    return m_baseType;
}

void NewUMLWidget::setBaseType(Uml::Widget_Type  type)
{
    m_baseType = type;
}

UMLScene* NewUMLWidget::umlScene() const
{
    return qobject_cast<UMLScene*>(this->scene());
}

UMLDoc* NewUMLWidget::umlDoc() const
{
    return UMLApp::app()->getDocument();
}

QString NewUMLWidget::documentation() const
{
    if(m_umlObject) {
        return m_umlObject->getDoc();
    }
    return m_widgetInterfaceData->documentation;
}

void NewUMLWidget::setDocumentation(const QString& doc)
{
    if(m_umlObject) {
        m_umlObject->setDoc(doc);
    }
    else {
        m_widgetInterfaceData->documentation = doc;
    }
}

QString NewUMLWidget::name() const
{
    if(m_umlObject) {
        return m_umlObject->getName();
    }
    return m_widgetInterfaceData->name;
}

void NewUMLWidget::setName(const QString& name)
{
    if(m_umlObject) {
        m_umlObject->setName(name);
    }
    else {
        m_widgetInterfaceData->name = name;
    }
    updateGeometry();
}

void NewUMLWidget::setPen(const QPen& pen)
{
    m_pen = pen;
    updateGeometry();
}

void NewUMLWidget::setBrush(const QBrush& brush)
{
    m_brush = brush;
    update();
}

void NewUMLWidget::setFont(const QFont& font)
{
    m_font = font;
    updateGeometry();
}

void NewUMLWidget::showPropertiesDialog()
{
}

void NewUMLWidget::setupContextMenuActions(ListPopupMenu &menu)
{
	Q_UNUSED(menu);
}

bool NewUMLWidget::loadFromXMI(QDomElement &qElement)
{
    //TODO: Add support for older versions.
    Widget_Utils::loadPainterInfoFromXMI(qElement, m_pen, m_brush, m_font);


    QString id = qElement.attribute("xmi.id", "-1");
    // Assert for the correctness of id loaded and the created object.
    if(m_umlObject) {
        if(id != ID2STR(this->id())) {
            uWarning() << "ID mismatch between UMLWidget and its UMLObject"
                       << "So the id read will be ignored.";
        }
    }
    else {
        setId(STR2ID(id));
    }

    qreal x = qElement.attribute("x", "0").toDouble();
    qreal y = qElement.attribute("y", "0").toDouble();
    setPos(x, y);

    return true;
}

void NewUMLWidget::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    Widget_Utils::savePainterInfoToXMI(qDoc, qElement, m_pen, m_brush, m_font);

    qElement.setAttribute("xmi.id", ID2STR(id()));
    qElement.setAttribute("x", pos().x());
    qElement.setAttribute("y", pos().y());
}

bool NewUMLWidget::widgetHasUMLObject(Uml::Widget_Type type)
{
	switch(type)
	{
	case Uml::wt_Actor:
	case Uml::wt_UseCase:
	case Uml::wt_Class:
	case Uml::wt_Interface:
	case Uml::wt_Enum:
	case Uml::wt_Datatype:
	case Uml::wt_Package:
	case Uml::wt_Component:
	case Uml::wt_Node:
	case Uml::wt_Artifact:
	case Uml::wt_Object:
		return true;
	default:
		return false;
	}
}


void NewUMLWidget::slotUMLObjectDataChanged()
{

}

void NewUMLWidget::updateGeometry()
{
    update();
}

void NewUMLWidget::setBoundingRectAndShape(const QRectF &rect, const QPainterPath& path)
{
    prepareGeometryChange();
    m_boundingRect = rect;
    m_shape = path;
    if(m_shape.isEmpty()) {
        m_shape.addRect(rect);
    }
}

#include "newumlwidget.moc"
