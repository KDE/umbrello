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
    void setUMLObject(UMLObject *obj);

    Uml::IDType id() const;
    void setID(Uml::IDType id);

	/// @return The base type rtti info.
    Uml::Widget_Type baseType() const {
        return m_baseType;
    }

    UMLScene* umlScene() const;
    UMLDoc* umlDoc() const;

    QString documentation() const;
    void setDocumentation(const QString& doc);

    QString name() const;
    void setName(const QString& doc);

	/// @return The color used to draw lines of the widget.
    QColor lineColor() const {
        return m_lineColor;
    }
    void setLineColor(const QColor& color);

	/// @return The width of the lines drawn in the widget.
    uint lineWidth() const {
        return m_lineWidth;
    }
    void setLineWidth(uint lw);

	/// @return Font color used to draw font.
    QColor fontColor() const {
        return m_fontColor;
    }
    void setFontColor(const QColor& color);

	/// @return The QBrush object used to fill this widget.
    QBrush brush() const {
        return m_brush;
    }
    void setBrush(const QBrush& brush);

	/// @return The font used for displaying any text
    QFont font() const {
        return m_font;
    }
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
    void slotInit();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

	virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);

    virtual void updateGeometry();

    virtual void umlObjectChanged(UMLObject *old);

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
