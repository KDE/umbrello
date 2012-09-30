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

#include "basictypes.h"
#include "umlscene.h"

#include <QAction>
#include <QBrush>
#include <QDomDocument>
#include <QFont>
#include <QGraphicsObject>
#include <QObject>
#include <QPainter>
#include <QPen>

// forward delcarations
class IDChangeLog;
class ListPopupMenu;
class UMLDoc;
class UMLObject;
class UMLScene;
class WidgetInterfaceData;

/**
 * @short The base class for UML widgets that appear on UML diagrams.
 *
 * This class provides the common interface required for all the UML
 * widgets including rectangular and non rectangular widgets.
 * Rectangular widgets should use UMLWidget as its base.
 */
class WidgetBase : public QGraphicsObject
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
    Q_ENUMS(WidgetType)

public:
    enum WidgetType
    {
        wt_UMLWidget = 300,         // does not have UMLObject representation
        wt_Actor,                   // has UMLObject representation
        wt_UseCase,                 // has UMLObject representation
        wt_Class,                   // has UMLObject representation
        wt_Interface,               // has UMLObject representation
        wt_Datatype,                // has UMLObject representation
        wt_Enum,                    // has UMLObject representation
        wt_Entity,                  // has UMLObject representation
        wt_Package,                 // has UMLObject representation
        wt_Object,                  // has UMLObject representation
        wt_Note,                    // does not have UMLObject representation
        wt_Box,                     // does not have UMLObject representation
        wt_Message,                 // does not have UMLObject representation
        wt_Text,                    // does not have UMLObject representation
        wt_State,                   // does not have UMLObject representation
        wt_Activity,                // does not have UMLObject representation
        wt_Component,               // has UMLObject representation
        wt_Artifact,                // has UMLObject representation
        wt_Node,                    // has UMLObject representation
        wt_Association,             // has UMLObject representation
        wt_ForkJoin,                // does not have UMLObject representation
        wt_Precondition,            // does not have UMLObject representation
        wt_CombinedFragment,        // does not have UMLObject representation
        wt_FloatingDashLine,        // does not have UMLObject representation
        wt_Signal,                  // does not have UMLObject representation
        wt_Pin,
        wt_ObjectNode,
        wt_Region,
        wt_Category                 // has UMLObject representation
    };

    static QString toString(WidgetType wt);

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

    enum UserChangeType {
        PositionChange = 0x1,
        GeometryChange = 0x2,
        AssocAdjustChange = 0x4
    };
    Q_DECLARE_FLAGS(UserChange, UserChangeType)

    // FIXME: differ from trunk because of different implementations 
    explicit WidgetBase(WidgetType type = wt_UMLWidget, UMLObject *object = 0);
    virtual ~WidgetBase();

    UMLObject* umlObject() const;
    virtual void setUMLObject(UMLObject *obj, bool notifyAsSlot = false);

    Uml::IDType id() const;
    void setID(Uml::IDType id);

    WidgetType baseType() const;
    QLatin1String baseTypeStr() const;

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
    void setUsesDiagramLineColor(bool state);

    bool usesDiagramLineWidth() const;
    void setUsesDiagramLineWidth(bool state);

    bool usesDiagramBrush() const;
    void setUsesDiagramBrush(bool state);

    bool usesDiagramFont() const;
    void setUsesDiagramFont(bool state);

    bool usesDiagramTextColor() const;
    void setUsesDiagramTextColor(bool state);

    QFont font() const;
    void setFont(const QFont& font);

    QRectF boundingRect() const;
    QPainterPath shape() const;

    bool isActivated() const;
    void setActivatedFlag(bool value);

    virtual bool activate();

    bool userChange(UserChangeType c) const;
    UserChange userChanges() const;
    void setUserChange(UserChangeType c, bool value = true);

    virtual void showPropertiesDialog();

    virtual bool loadFromXMI(QDomElement &qElement);
    virtual void saveToXMI(QDomDocument &qDoc, QDomElement &qElement);

    static bool widgetHasUMLObject(WidgetType type);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction *trigger);

protected Q_SLOTS:
    virtual void slotUMLObjectDataChanged();

protected:
    virtual void contextMenuEvent(UMLSceneContextMenuEvent *event);
    virtual void setupContextMenuActions(ListPopupMenu &menu);

    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

    virtual void updateGeometry();

    virtual void umlObjectChanged(UMLObject *old);

    void setBoundingRect(const QRectF &rect);
    void setShape(const QPainterPath& path);

    QRectF m_boundingRect;
    QPainterPath m_shape;
    QPointF m_itemPositionChangePos;

    /**
     * This acts like stash to temporarily store data in loadFromXMI, which
     * are then applied in the activation method.
     *
     * This pointer is deleted on setActivatedFlag(true) and is newed in 
     * WidgetBase::loadFromXMI
     */
    QVariantMap m_loadData;
    WidgetType m_baseType;

private:
    UMLObject *m_umlObject;
    QColor m_lineColor;
    uint m_lineWidth;
    QColor m_textColor;
    QBrush m_brush;
    QFont m_font;

    /**
     * This flag is used to hold whether a widget is partially constructed (due to
     * delayed construction of sub components) or not.
     */
    bool m_activated;
    WidgetInterfaceData *m_widgetInterfaceData;

    UserChange m_userChange;

    bool m_usesDiagramLineColor:1;
    bool m_usesDiagramLineWidth:1;
    bool m_usesDiagramBrush:1;
    bool m_usesDiagramFont:1;
    bool m_usesDiagramTextColor:1;

    // Disable the copy constructor and assignment operator.
    Q_DISABLE_COPY(WidgetBase)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WidgetBase::UserChange)

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
