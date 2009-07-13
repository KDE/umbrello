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
    Q_PROPERTY(QColor fontColor READ fontColor WRITE setFontColor)
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
        FontColorHasChanged,
        FontHasChanged,
        BrushHasChanged,

        // Provided by UMLWidget
        SizeHasChanged
    };

    explicit WidgetBase(UMLObject *object);
    virtual ~WidgetBase();

    UMLObject* umlObject() const;
    void setUMLObject(UMLObject *obj);

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

    QColor fontColor() const;
    void setFontColor(const QColor& color);

    QBrush brush() const;
    void setBrush(const QBrush& brush);

    QFont font() const;
    void setFont(const QFont& font);

    QRectF boundingRect() const;
    QPainterPath shape() const;

    virtual void showPropertiesDialog();
    virtual void setupContextMenuActions(ListPopupMenu &menu);

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

    QColor m_fontColor;

    QBrush m_brush;
    QFont m_font;

    // End of properties that will be saved.

    WidgetInterfaceData *m_widgetInterfaceData;

    /**
     * This is used to ensure that there is only one initialization when a
     * new UMLScene is set for this widget for the first time.
     */
    bool m_isSceneSetBefore;
    /**
     * Disable the copy constructor and assignment operator.
     */
    DISABLE_COPY(WidgetBase);


public:

    // DEPRECATED SECTION ///////////////////////////////

    bool firstTime;  // is used in updateComponentSize()

    qreal getX() const { return pos().x(); }
    void setX(qreal x) { setPos(x, y()); }
    qreal getY() const { return pos().y(); }
    void setY(qreal y) { setPos(x(), y); }

    void setBaseType(Uml::Widget_Type type) { m_baseType = type; }

    void setPenFromSettings(QPainter &p) { p.setPen(QPen(m_lineColor, m_lineWidth)); }
    void updateComponentSize();

    void setLineColorcmd(const QColor& col) { setLineColor(col); }
    QColor getFillColour() const { return brush().color(); }
    QColor getFillColor() const { return getFillColour(); }
    void setFillColour(const QColor& col) { setBrush(QBrush(col)); }

    bool getUseFillColour() const { return false; }
    void setUseFillColour(bool) {}

    typedef enum {
        FT_NORMAL = 0,
        FT_BOLD  = 1,
        FT_ITALIC = 2,
        FT_UNDERLINE = 3,
        FT_BOLD_ITALIC = 4,
        FT_BOLD_UNDERLINE = 5,
        FT_ITALIC_UNDERLINE = 6,
        FT_BOLD_ITALIC_UNDERLINE = 7,
        FT_INVALID = 8
    } FontType;

    QFontMetrics  *m_pFontMetrics[FT_INVALID];

    virtual void setDefaultFontMetrics(WidgetBase::FontType fontType);
    virtual void setDefaultFontMetrics(WidgetBase::FontType fontType, QPainter &painter);

    QFontMetrics &getFontMetrics(WidgetBase::FontType fontType);
    void setFontMetrics(WidgetBase::FontType fontType, QFontMetrics fm);
    void setupFontType(QFont &font, WidgetBase::FontType fontType);
    void forceUpdateFontMetrics(QPainter *);

    qreal onWidget(const QPointF& pos) const {
        if(this->contains(mapFromScene(pos))) {
            QSizeF s = boundingRect().size();
            return .5 * (s.width() + s.height());
        }
        return 0;
    }

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
