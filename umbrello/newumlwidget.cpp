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

#include "umlobject.h"
#include "umlscene.h"
#include "uml.h"
#include "widget_utils.h"

#include <QtCore/QTimer>
#include <kdebug.h>

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
    m_lineWidth(0),
    m_widgetInterfaceData(0)
{
    if(!object) {
        m_widgetInterfaceData = new WidgetInterfaceData;
    }
    setFlags(ItemIsSelectable | ItemIsMovable);
    // Call init this way so that virtual methods may be called.
    QTimer::singleShot(0, this, SLOT(slotInit()));
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
    UMLObject *oldObj = m_umlObject;
    m_umlObject = obj;

    if(oldObj) {
        oldObj->disconnect(this);
    }

    if(m_umlObject) {
        delete m_widgetInterfaceData;
        connect(umlObject(), SIGNAL(modified()), this, SLOT(slotUMLObjectDataChanged()));
    }
    else if(!m_widgetInterfaceData) {
        m_widgetInterfaceData = new WidgetInterfaceData;
    }

    umlObjectChanged(oldObj);
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

        if(m_umlObject->getID() != Uml::id_None) {
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

void NewUMLWidget::setLineColor(const QColor& color)
{
    m_lineColor = color;
    if(!m_lineColor.isValid()) {
        uDebug() << "Invalid color";
        m_lineColor = Qt::black;
    }
    update();
}

void NewUMLWidget::setLineWidth(uint lw)
{
    m_lineWidth = lw;
    updateGeometry();
}

void NewUMLWidget::setFontColor(const QColor& color)
{
    m_fontColor = color;
    if(!m_fontColor.isValid()) {
        m_fontColor = m_lineColor;
    }
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
    // NOTE:
    // The "none" is used by kde3 version of umbrello. The current
    // technique to determine whether a property is being used from
    // the diagram or not is just to compare the same, rather than
    // having flags for them.
    //
    // This way, there is no burden to update the flags and the code
    // is more robust.
    const QLatin1String none("none");

    // Load the line color first

    // Workaround for old "linecolour" usage.
    QString lineColor = qElement.attribute("linecolour");
    lineColor = qElement.attribute("linecolor", lineColor);
    if(!lineColor.isEmpty() && lineColor != none) {
        setLineColor(QColor(lineColor));
    }
    else if(umlScene()) {
        setLineColor(umlScene()->getLineColor());
    }

    // Load the line width.
    QString lineWidth = qElement.attribute("linewidth");
    if(!lineWidth.isEmpty() && lineWidth != none) {
        setLineWidth(lineWidth.toInt());
    }
    else if(umlScene()) {
        setLineWidth(umlScene()->getLineWidth());
    }

    // Load the font color, if invalid line color is automatically used.
    QString fontColor = qElement.attribute("fontcolor");
    setFontColor(QColor(fontColor));

    // Load the brush.
    QBrush newBrush;
    bool brushSet = Widget_Utils::loadBrushFromXMI(qElement, newBrush);

    // If this fails, we try to load fillColor attribute which is used in kde3 version of umbrello.
    if(!brushSet) {
        // Workaround for old "fillcolour" usage.
        QString fillColor = qElement.attribute("fillcolour");
        fillColor = qElement.attribute("fillcolor");

        if(!fillColor.isEmpty() && fillColor != none) {
            setBrush(QColor(fillColor));
            brushSet = true;
        }
    }
    else {
        setBrush(newBrush);
    }
    // Set the diagram's brush if it is not yet set.
    if(!brushSet && umlScene()) {
        setBrush(umlScene()->brush());
    }

    // Load the font.
    QString font = qElement.attribute(QLatin1String("font"));
    bool fontSet = false;
    if(!font.isEmpty()) {
        fontSet = m_font.fromString(font);
        if(fontSet) {
            setFont(m_font);
        }
        else {
            uWarning() << "Loading font attribute -> " << font << " failed";
        }
    }
    // Set diagram's default font if font is not yet set.
    if(!fontSet && umlScene()) {
        setFont(umlScene()->font());
    }

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
    qElement.setAttribute("linecolor", m_lineColor.name());
    qElement.setAttribute("linewidth", m_lineWidth);
    qElement.setAttribute("fontcolor", m_fontColor.name());
    qElement.setAttribute("font", m_font.toString());

    Widget_Utils::saveBrushToXMI(qDoc, qElement, m_brush);

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
    updateGeometry();
}

void NewUMLWidget::slotInit()
{
    setUmlObject(m_umlObject);
    updateGeometry();
}

void NewUMLWidget::updateGeometry()
{
    update();
}

void NewUMLWidget::umlObjectChanged(UMLObject *oldObj)
{
    Q_UNUSED(oldObj);
}

void NewUMLWidget::setBoundingRect(const QRectF &rect)
{
    prepareGeometryChange();
    m_boundingRect = rect;
}

void NewUMLWidget::setShape(const QPainterPath& path)
{
    m_shape = path;
    if(m_shape.isEmpty()) {
        m_shape.addRect(boundingRect());
    }
}

#include "newumlwidget.moc"
