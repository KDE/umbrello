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

#ifndef UMLWIDGET_H
#define UMLWIDGET_H

#include "associationwidgetlist.h"
#include "umlscene.h"
#include "widgetbase.h"

class AssociationSpaceManager;
class AssociationWidget;
class WidgetHandle;
class TextItemGroup;

/**
 * @short The base class for rectangular base UML widgets.
 *
 * This class extends the functionality provided by WidgetBase with
 * facilities to resize the widget, provide flexible constraints
 * management.
 *
 * Also this class handles connected association widgets whenever
 * required.
 */
class UMLWidget : public WidgetBase
{
    Q_OBJECT
    Q_PROPERTY(UMLSceneSize minimumSize READ minimumSize WRITE setMinimumSize DESIGNABLE false)
    Q_PROPERTY(UMLSceneSize maximumSize READ maximumSize WRITE setMaximumSize DESIGNABLE false)
    Q_PROPERTY(UMLSceneSize size READ size WRITE setSize)
    Q_PROPERTY(bool resizable READ isResizable WRITE setResizable DESIGNABLE false)
    Q_PROPERTY(QRectF rect READ rect DESIGNABLE false)
    Q_PROPERTY(double margin READ margin WRITE setMargin)
    Q_PROPERTY(QString instanceName READ instanceName WRITE setInstanceName)
    Q_PROPERTY(bool isInstance READ isInstance WRITE setIsInstance)
    Q_PROPERTY(bool showStereotype READ showStereotype WRITE setShowStereotype)

public:
    /**
     * This enum presents options for @ref setMinimumSize and @ref
     * setMaximumSize methods.
     */
    enum SizeHintOption {
        DontAddMargin,
        AddMargin ///< adds 2 * margin() to width and height
    };
    static const UMLSceneSize DefaultMinimumSize;
    static const UMLSceneSize DefaultMaximumSize;

    explicit UMLWidget(WidgetType type = wt_UMLWidget, Uml::IDType id = 0);
    UMLWidget(WidgetType type, UMLObject *object);
    virtual ~UMLWidget();

    /// @return The minimum size for this widget.
    UMLSceneSize minimumSize() const {
        return m_minimumSize;
    }
    void setMinimumSize(const UMLSceneSize& newSize, SizeHintOption option = DontAddMargin);

    /// @return The maximum size for this widget.
    UMLSceneSize maximumSize() const {
        return m_maximumSize;
    }
    void setMaximumSize(const UMLSceneSize& newSize, SizeHintOption option = DontAddMargin);

    /// @return The current size of this widget.
    UMLSceneSize size() const {
        return m_size;
    }
    void setSize(const UMLSceneSize &size);
    void setSize(qreal width, qreal height) {
        setSize(UMLSceneSize(width, height));
    }

    /// @return Height of widget.
    qreal height() const {
        return m_size.height();
    }
    /// Shortcut for setSize(width(), ht)
    void setHeight(qreal ht) {
        setSize(m_size.width(), ht);
    }
    /// @return Width of widget.
    qreal width() const {
        return m_size.width();
    }
    /// Shortcut for setSize(wt, height())
    void setWidth(qreal wt) {
        setSize(wt, m_size.height());
    }

    /// @return Whether widget is resizable or not.
    bool isResizable() const {
        return m_resizable;
    }

    /**
     * Shorthand for QRectF(QPointF(0, 0), size()); The rect() area is
     * where all the widget painting should go.
     *
     * @note boundingRect() is always larger than or equal to rect().
     */
    QRectF rect() const {
        return QRectF(QPointF(0, 0), size());
    }
    QRectF sceneRect() const;

    /**
     * @return The margin from the rect() left for good visual appeal.
     */
    UMLSceneValue margin() const {
        return m_margin;
    }
    void setMargin(UMLSceneValue margin);

    /// @return The instance name for this widget.
    QString instanceName() const {
        return m_instanceName;
    }
    void setInstanceName(const QString &name);

    /// @return Whether this is an instance or not.
    bool isInstance() const {
        return m_isInstance;
    }
    void setIsInstance(bool b);

    /// @return Whether to show stereotype or not.
    bool showStereotype() const {
        return m_showStereotype;
    }
    void setShowStereotype(bool b);

    AssociationSpaceManager* associationSpaceManager() const;
    AssociationWidgetList associationWidgetList() const;

    virtual bool activate();
    virtual void adjustAssociations(bool userChangeAdjustAssoc);

    virtual void showPropertiesDialog();

    virtual bool loadFromXMI(QDomElement &qElement);
    virtual void saveToXMI(QDomDocument &qDoc, QDomElement &qElement);

    virtual void contextMenuEvent(UMLSceneContextMenuEvent * event);

protected:
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);
    virtual void updateGeometry();
    virtual void setResizable(bool resizable);

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    TextItemGroup* createTextItemGroup();
    int indexOfTextItemGroup(TextItemGroup *group) const;
    TextItemGroup* textItemGroupAt(int index) const;
    virtual void updateTextItemGroups();

    /// List of TextItemGroup, which manages various texts for this widget.
    QList<TextItemGroup*> m_textItemGroups;

public Q_SLOTS:
    virtual void slotMenuSelection(QAction *trigger);

protected Q_SLOTS:
    virtual void slotUMLObjectDataChanged();

private:
    /// Current size of the widget.
    UMLSceneSize m_size;

    UMLSceneSize m_minimumSize;
    UMLSceneSize m_maximumSize;

    /// Margin for this widget.
    UMLSceneValue m_margin;

    /// Manages the alignment of AssociationWidget's.
    AssociationSpaceManager *m_associationSpaceManager;
    /// List of AssociationWidget's in which this widget is participating.
    QList<AssociationWidget*> m_associationWidgetList;

    QString m_instanceName;
    bool m_isInstance;
    bool m_showStereotype;

    /// Whether resizable or not.
    bool m_resizable;

    /// Widget handle for this widget (for resizing)
    WidgetHandle *m_widgetHandle;

    QGraphicsSceneMouseEvent *m_mouseMoveEventStore;

    // Disable copy constructor and operator=
    Q_DISABLE_COPY(UMLWidget);
};

#endif
