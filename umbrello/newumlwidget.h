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

#ifndef NEWUMLWIDGET_H
#define NEWUMLWIDGET_H

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
 * Rectangular widgets should use NewUmlRectWidget as its base.
 */
class NewUMLWidget : public QObject, public QGraphicsItem
{
    Q_OBJECT;
public:
	/**
	 * This enumeration is used by NewUMLWidget::attributeChange() to
	 * identify which attribute has changed.
	 * This is modelled on QGraphicsItem::GraphicsItemChange.
	 */
	enum WidgetAttributeChange {
		// These 3 work only if NewUMLWidget::setters are used , that
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

		// Provided by NewUMLRectWidget
		SizeHasChanged
	};

    explicit NewUMLWidget(UMLObject *object);
    ~NewUMLWidget();

	/**
	 * @retval The UMLObject represented by this widget
	 * @retval null if no UMLObject representation.
     */
    UMLObject* umlObject() const {
        return m_umlObject;
    }

    /**
     * Set the UMLObject for this widget to represent.
     *
     * @todo Either remove this method, or allow it to only allow specific
     *       users as making this method public violates encapsulation.
     */
    void setUMLObject(UMLObject *obj);

    /**
     * If this widget represents a UMLObject, the id of that object is
     * returned. Else the id stored in this widget is returned.
     *
     * @return The identifier of this widget.
     */
    Uml::IDType id() const;

    /**
     * If this widget represents a UMLObject, the id of that object is set
     * to \a id. Else the id is stored in this widget.
     *
     * This method issues a @ref IDHasChanged notification after setting
     * the id.
     * @see NewUMLWidget::attributeChange
     */
    void setID(Uml::IDType id);

	/// @return The base type rtti info.
    Uml::Widget_Type baseType() const {
        return m_baseType;
    }

    /**
     * @return The UMLScene for this widget is returned, or 0 if the
     *         widget is not stored in a scene.
     *
     * @note To add or remove widgets to scene, use UMLScene::addItem
     */
    UMLScene* umlScene() const;

    /**
     * This is shortcut method for UMLApp::app()->getDocument()
     *
     * @return Pointer to the UMLDoc object.
     */
    UMLDoc* umlDoc() const;

    /**
     * If this widget represents a UMLObject, the documentation string
     * stored in the object is returned. Else the documentation string
     * stored in the widget is returned.
     *
     * @return A string representing the documentation for this widget
     *         which is usually set by the user.
     */
    QString documentation() const;

    /**
     * If this widget represents a UMLObject, the documentation string
     * of that object is set to \a doc. Else the documentation string
     * stored in the widget is set to \a doc.
     *
     * This method issues a @ref DocumentationHasChanged notification
     * after setting the new documentation.
     * @see NewUMLWidget::attributeChange
     */
    void setDocumentation(const QString& doc);

    /**
     * If this widget represents a UMLObject, the name string stored
     * in the object is returned. Else the name string stored in the
     * widget is returned.
     *
     * @return A string representing the name for this widget
     *         which is usually set by the user.
     */
    QString name() const;

    /**
     * If this widget represents a UMLObject, the name string of that
     * object is set to \a name. Else the name string stored in the
     * widget is set to \a name.
     *
     * This method issues a @ref NameHasChanged notification after setting
     * the new name.
     * @see NewUMLWidget::attributeChange
     */
    void setName(const QString& doc);

	/// @return The color used to draw lines of the widget.
    QColor lineColor() const {
        return m_lineColor;
    }

    /**
     * Set the linecolor to \a color and updates the widget.
     * @param color The color to be set
     *
     * This method issues a @ref LineColorHasChanged notification after
     * setting the new line color.
     * @see NewUMLWidget::attributeChange
     */
    void setLineColor(const QColor& color);

	/// @return The width of the lines drawn in the widget.
    uint lineWidth() const {
        return m_lineWidth;
    }

    /**
     * Sets the line width of widget lines to \a lw and calls
     * updateGeometry() method to let object calculate new bound rect
     * based on the new line width.
     *
     * @param lw  The width of line to be set.
     *
     * This method issues @ref LineWidthHasChanged notification after
     * setting new line width.
     * @see NewUMLWidget::attributeChange
     */
    void setLineWidth(uint lw);

	/// @return Font color used to draw font.
    QColor fontColor() const {
        return m_fontColor;
    }

    /**
     * Sets the color of the font to \a color.
     * If \a color is invalid, line color is used for font color.
     *
     * This method issues @ref FontColorHasChanged notification after
     * setting the new font color.
     * @see NewUMLWidget::attributeChange
     */
    void setFontColor(const QColor& color);

	/// @return The QBrush object used to fill this widget.
    QBrush brush() const {
        return m_brush;
    }

    /**
     * Sets the QBrush object of this widget to \a brush which is used to
     * fill this widget.
     *
     * This method issues @ref BrushHasChanged notification after setting
     * the new brush.
     * @see NewUMLWidget::attributeChange
     */
    void setBrush(const QBrush& brush);

	/// @return The font used for displaying any text
    QFont font() const {
        return m_font;
    }

    /**
     * Set the font used to display text inside this widget.
     *
     * This method issues @ref FontHasChanged notification after setting
     * the new font.
     * @see NewUMLWidget::attributeChange
     */
    void setFont(const QFont& font);

	/**
	 * @return The bounding rectangle for this widget.
	 * @see setBoundingRect
	 */
    QRectF boundingRect() const {
        return m_boundingRect;
    }
	/**
	 * @return The shape of this widget.
	 * @see setShape
	 */
    QPainterPath shape() const {
        return m_shape;
    }

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

    /**
     * This slot is used to intialize the widget, also allowing virtual
     * methods to be called.
     */
    void slotInit();

protected:

    /**
     * Reimplemented to show appropriate context menu.
     */
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

	virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

    virtual void updateGeometry();

    virtual void umlObjectChanged(UMLObject *old);

    /**
     * This method sets the bounding rectangle of this widget to \a
     * rect. The bounding rect being set is cached locally for performance
     * reasons.
     *
     * Also the prepareGeometryChange() method is implicitly called to
     * update QGraphicsScene indexes.
     *
     * @param rect The bounding rectangle for this widget.
     *
     * @note The bounding rect being set should also be compensated with
     *       half pen width if the widget is painting an outline/stroke.
     *
     * @note Also note that, subclasses reimplementing @ref boundingRect()
     *       virtual method will not be affected by this method unless the
     *       subclassed widget explicitly uses it.
     *
     * @see Widget_Utils::adjustedBoundingRect
     */
    void setBoundingRect(const QRectF &rect);

    /**
     * This method sets the shape of the widget to \a path. The shape of
     * the widget is cached for performance reasons.
     *
     * @param path The shape of this widget. If empty, boundingRect will
     *             be used as widget shape.
     *
     * @see NewUMLWidget::setBoundingRect
     *
     * @todo Check the accuracy of this method for non rectangular widgets
     *       as this doesn't call prepareGeometryChange.
     *
     * @note Also note that, subclasses reimplementing @ref shape()
     *       virtual method will not be affected by this method unless the
     *       subclassed widget explicitly uses it.
     */
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
    /*
     * Disable the copy constructor and assignment operator.
     */
    DISABLE_COPY(NewUMLWidget);


public:

    // DEPRECATED SECTION ///////////////////////////////

    bool firstTime;

    NewUMLWidget(UMLScene *scene, const Uml::IDType &id = Uml::id_None);
    NewUMLWidget(UMLScene *scene, UMLObject *object);

    UMLObject* getUMLObject() const { return umlObject(); }

    Uml::IDType getID() const { return id(); }
    qreal getX() const { return pos().x(); }
    void setX(qreal x) { setPos(x, y()); }
    qreal getY() const { return pos().y(); }
    void setY(qreal y) { setPos(x(), y); }

    Uml::Widget_Type getBaseType() const { return baseType(); }
    void setBaseType(Uml::Widget_Type type) { m_baseType = type; }

    void setPenFromSettings(QPainter &p) { p.setPen(QPen(m_lineColor, m_lineWidth)); }
    void updateComponentSize();

    QString getDoc() const { return documentation(); }
    void setDoc(const QString &doc) { setDocumentation(doc); }
    QString getName() const { return name(); }
    QColor getLineColor() const { return lineColor(); }
    void setLineColorcmd(const QColor& col) { setLineColor(col); }
    uint getLineWidth() const { return lineWidth(); }
    QColor getFillColour() const { return brush().color(); }
    QColor getFillColor() const { return getFillColour(); }
    void setFillColour(const QColor& col) { setBrush(QBrush(col)); }
    void setFillColourcmd(const QColor& col) { setFillColour(col); }

    bool getUseFillColour() const { return false; }
    void setUseFillColour(bool) {}
    QFont getFont() const { return font(); }

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
    /** Template Method, override this to set the default
     *  font metric.
     */
    virtual void setDefaultFontMetrics(NewUMLWidget::FontType fontType);
    virtual void setDefaultFontMetrics(NewUMLWidget::FontType fontType, QPainter &painter);

    /** Returns the font metric used by this object for Text which uses bold/italic fonts*/
    QFontMetrics &getFontMetrics(NewUMLWidget::FontType fontType);
    /** set the font metric to use */
    void setFontMetrics(NewUMLWidget::FontType fontType, QFontMetrics fm);
    void setupFontType(QFont &font, NewUMLWidget::FontType fontType);
    void forceUpdateFontMetrics(QPainter *);
    void drawSelected(QPainter *, qreal, qreal) {}
    QString m_Text;
    ListPopupMenu *m_pMenu;
    virtual ListPopupMenu* setupPopupMenu();
    qreal m_origZ;
    void setZ(qreal z) { setZValue(z); }
    qreal getZ() const { return zValue(); }
    void setActivated() {}
    bool activate(IDChangeLog*) { return true;}
    void cleanup() {}
    UMLScene* getUMLScene() const { return umlScene(); }
    qreal onWidget(const QPointF& pos) const {
        if(this->contains(mapFromScene(pos))) {
            QSizeF s = boundingRect().size();
            return .5 * (s.width() + s.height());
        }
        return 0;
    }
    void constrain(qreal, qreal) {}
    ////////////////////////////////////////////////////////////////////////////////

};

#endif //NEWUMLWIDGET_H
