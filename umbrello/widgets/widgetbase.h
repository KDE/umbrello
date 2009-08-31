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

#ifndef WIDGETBASE_H
#define WIDGETBASE_H

#include "umlnamespace.h"

#include <QtCore/QObject>

#include <QtGui/QAction>
#include <QtGui/QBrush>
#include <QtGui/QFont>
#include <QtGui/QGraphicsItem>
#include <QtGui/QPainter>
#include <QtGui/QPen>

#include <QtXml/QDomDocument>

// Forward delcarations
class UMLObject;
class UMLScene;
class UMLDoc;
class WidgetInterfaceData;
class ListPopupMenu;
class IDChangeLog;

/**
 * @short The base class for UML widgets that appear on UML diagrams.
 *
 * This class provides the common interface required for all the UML
 * widgets including rectangular and non rectangular widgets.
 * Rectangular widgets should use UMLWidget as its base.
 */
class WidgetBase : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString documentation READ documentation WRITE setDocumentation)
    Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor)
    Q_PROPERTY(uint lineWidth READ lineWidth WRITE setLineWidth)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
    Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
    Q_PROPERTY(QFont font READ font WRITE setFont)
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    /**
     * This enumeration is used by WidgetBase::attributeChange() to
     * identify which attribute has changed.
     * This is modelled on QGraphicsItem::GraphicsItemChange.
     */
    enum WidgetAttributeChange {
        // These 3 work only if WidgetBase::setters are used , that
        // is, the notifications arent' sent when UMLObject::setters
        // are used.
        IDHasChanged,
        DocumentationHasChanged,
        NameHasChanged,

        // Graphical attribute change notification
        LineColorHasChanged,
        LineWidthHasChanged,
        TextColorHasChanged,
        FontHasChanged,
        BrushHasChanged,

        // Provided by UMLWidget
        SizeHasChanged
    };

    explicit WidgetBase(UMLObject *object);
    virtual ~WidgetBase();

    UMLObject* umlObject() const;
    virtual void setUMLObject(UMLObject *obj);

    Uml::IDType id() const;
    void setID(Uml::IDType id);

    Uml::Widget_Type baseType() const;

    UMLScene* umlScene() const;
    UMLDoc* umlDoc() const;

    QString documentation() const;
    void setDocumentation(const QString& doc);

    QString name() const;
    void setName(const QString& name);

    QColor lineColor() const;
    void setLineColor(const QColor& color);

    uint lineWidth() const;
    void setLineWidth(uint lw);

    QColor textColor() const;
    void setTextColor(const QColor& color);

    QBrush brush() const;
    void setBrush(const QBrush& brush);

    bool usesDiagramLineColor() const;
    void setUsesDiagramLineColor(bool status);

    bool usesDiagramLineWidth() const;
    void setUsesDiagramLineWidth(bool status);

    bool usesDiagramBrush() const;
    void setUsesDiagramBrush(bool status);

    bool usesDiagramFont() const;
    void setUsesDiagramFont(bool status);

    bool usesDiagramTextColor() const;
    void setUsesDiagramTextColor(bool status);

    QFont font() const;
    void setFont(const QFont& font);

    QRectF boundingRect() const;
    QPainterPath shape() const;

    bool isActivated() const;
    void setActivatedFlag(bool value);

    virtual bool activate();

    virtual void showPropertiesDialog();

    virtual bool loadFromXMI(QDomElement &qElement);
    virtual void saveToXMI(QDomDocument &qDoc, QDomElement &qElement);

    static bool widgetHasUMLObject(Uml::Widget_Type type);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction *trigger);

protected Q_SLOTS:
    virtual void slotUMLObjectDataChanged();

private Q_SLOTS:
    void slotInit();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

    virtual void updateGeometry();

    virtual void umlObjectChanged(UMLObject *old);

    virtual void delayedInitialize();
    virtual void sceneSetFirstTime();

    void setBoundingRect(const QRectF &rect);
    void setShape(const QPainterPath& path);

    QRectF m_boundingRect;
    QPainterPath m_shape;

    // Property that will be saved.
    Uml::Widget_Type m_baseType;

private:
    UMLObject *m_umlObject;

    // Properties that will be saved.

    QColor m_lineColor;
    uint m_lineWidth;

    QColor m_textColor;

    QBrush m_brush;
    QFont m_font;

    // End of properties that will be saved.

    /**
     * This flag is used to hold whether a widget is partially constructed (due to
     * delayed construction of sub components) or not.
     */
    bool m_activated;
    WidgetInterfaceData *m_widgetInterfaceData;
    /**
     * This acts like stash to temporarily store data in loadFromXMI, which
     * are then applied in the activation method.
     *
     * This pointer is deleted on setActivatedFlag(true) and is newed in 
     * WidgetBase::loadFromXMI
     */
    QVariantMap *m_loadData;

    /**
     * This is used to ensure that there is only one initialization when a
     * new UMLScene is set for this widget for the first time.
     */
    bool m_isSceneSetBefore:1;

    bool m_usesDiagramLineColor:1;
    bool m_usesDiagramLineWidth:1;
    bool m_usesDiagramBrush:1;
    bool m_usesDiagramFont:1;
    bool m_usesDiagramTextColor:1;

    /**
     * Disable the copy constructor and assignment operator.
     */
    DISABLE_COPY(WidgetBase);


public:

    // DEPRECATED SECTION ///////////////////////////////

    QColor getFillColor() const;
    void setFillColour(const QColor& col) { setBrush(QBrush(col)); }

    bool getUseFillColour() const { return false; }
    void setUseFillColour(bool) {}

    ////////////////////////////////////////////////////////////////////////////////

};

/**
 * @return The bounding rectangle for this widget.
 * @see setBoundingRect
 */
inline QRectF WidgetBase::boundingRect() const
{
    return m_boundingRect;
}

/**
 * @return The shape of this widget.
 * @see setShape
 */
inline QPainterPath WidgetBase::shape() const
{
    return m_shape;
}

#endif
