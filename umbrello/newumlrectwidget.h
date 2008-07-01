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

#ifndef NEWUMLRECTWIDGET_H
#define NEWUMLRECTWIDGET_H

#include "newumlwidget.h"
#include "associationwidgetlist.h"

class AssociationWidget;
class WidgetHandle;
class UMLWidgetController;
/**
 * @short The base class for rectangular base UML widgets.
 *
 * This class extends the functionality provided by NewUMLWidget with
 * facilities to resize the widget, provide flexible constraints
 * management.
 *
 * Also this class handles connected association widgets whenever
 * required.
 */
class NewUMLRectWidget : public NewUMLWidget
{
	Q_OBJECT
public:
    static const QSizeF DefaultMinimumSize;
    static const QSizeF DefaultMaximumSize;
    static const QSizeF DefaultPreferredSize;

    /**
     * Construct a NewUMLRectWidget associtated with object.
     *
     * @param object UMLObject with which widget should be associated.
     * @see NewUMLWidget::NewUMLWidget()
     */
	explicit NewUMLRectWidget(UMLObject *object);
    // DEPRECATED
    NewUMLRectWidget(UMLScene *scene, const Uml::IDType & _id = Uml::id_None);
    NewUMLRectWidget(UMLScene *scene, UMLObject *obj);

    ~NewUMLRectWidget();

    /**
     * This virtual method should be implemented by the subclasses to
     * provide Maximum and Minimum size information for this
     * widget. This information is used in setSize and also by the
     * UMLScene.
     *
     * @param which The Qt::SizeHint enum value indicates what
     *              information is being queried. For now only
     *              Qt::MinimumSize and Qt::MaximumSize is used.
     *
     * @return The minimum or maximum size based on \a which.
     *
     * @note When sizeHint of a subclass needs a change,
     *       updateGeometry method should be called to ensure the
     *       widget is of proper size.
     *
     * @note Non const to allow subclasses to update their cached
     *       sizehint variable if desired.
     */
	virtual QSizeF sizeHint(Qt::SizeHint which);

    /**
     * @return The current size of the widget.
     */
	QSizeF size() const {
		return m_size;
	}
    /**
     * Sets the size of the widget to \a size.
     *
     * It also does cosmetic adjustments(enlarges) to the actual
     * boundingRect to include resize handles.
     *
     * Also notifies self with @ref sizeChanged() virtual method,
     * which can be used to update the shape corresponding to the new
     * size of the widget.
     *
     * @param size The new size to be set which should effectively
     *             satisfy the sizeHint constraints.
     */
	void setSize(const QSizeF &size);

    /**
     * Shortcut for setSize(w, h)
     */
    void setSize(qreal width, qreal height) {
        setSize(QSizeF(width, height));
    }
    // DEPRECATED
    qreal getWidth() const { return size().width(); }
    void setWidth(qreal w) { setSize(w, getHeight()); }
    qreal getHeight() const { return size().height(); }
    void setHeight(qreal h) { setSize(getWidth(), h); }

    /**
     * Shortcut for QRectF(QPointF(0, 0), size())
     *
     * @note rect() is not same as boundingRect(). The latter also
     *       includes extra portions to cover resize handles.
     */
    QRectF rect() const {
        return QRectF(QPointF(0, 0), size());
    }

    /**
     * @return The instance name of this widget(if used).
     */
	QString instanceName() const {
		return m_instanceName;
	}
    // DEPRECATED
    QString getInstanceName() const { return instanceName(); }
    /**
     * Set the instance name for this widget. Calls updateGeometry
     * implicitly.
     */
	void setInstanceName(const QString &name);

    virtual bool isInstance() const {
		return false;
	}
    // DEPRECATED
    bool getIsInstance() const { return isInstance(); }

    bool showStereotype() const { return m_showStereotype; }
    // DEPRECATED
    bool getShowStereotype() const { return showStereotype(); }

    void setShowStereotype(bool b);

    /**
     * @return A list of AssociationWidget connected with this widet.
     */
	AssociationWidgetList associationWidgetList() const {
		return m_associationWidgetList;
	}
    // DEPRECATED
    AssociationWidgetList getAssocList() const { return associationWidgetList(); }

    /**
     * @return Return whether this widget is resizable or not.
     * @see setResizable
     */
    bool isResizable() const {
        return m_resizable;
    }

    /**
     * Adds a association widget to the internal list that involves
     * this widget.
     *
     * @param assoc An association widget connected to this widget.
     */
	void addAssociationWidget(AssociationWidget *assoc);
    //DEPRECATED
    void addAssoc(AssociationWidget *assoc) { addAssociationWidget(assoc); }
    /**
     * Removes a association widget from the internal list that involves this widget.
     *
     * @param assoc The association widget that should be removed.
     */
	void removeAssociationWidget(AssociationWidget *assoc);
    // DEPRECATED
    void removeAssoc(AssociationWidget *assoc) { removeAssociationWidget(assoc); }

	void showPropertiesDialog();
    void setupContextMenuActions(ListPopupMenu &menu);

    /**
     * Adjusts the position and lines of connected association
     * widgets. This method is used usually after this widget moves
     * requiring assocition widgets to be updated.
     */
    void adjustConnectedAssociations();

    bool loadFromXMI(QDomElement &qElement);
    void saveToXMI(QDomDocument &qDoc, QDomElement &qElement);


    ///////////////        DEPRECATED STUFF ///////////////////////
    void adjustAssocs(qreal, qreal) {}
    QSizeF calculateSize() { return sizeHint(Qt::MinimumSize); }
    bool getStartMove() const { return false; }
    bool getIgnoreSnapToGrid() const { return false; }
    void setIgnoreSnapToGrid(bool) {}
    bool getIgnoreSnapComponentSizeToGrid() const { return false; }
    void setIgnoreSnapComponentSizeToGrid(bool) {}
    bool m_bStartMove;
    void adjustUnselectedAssocs(qreal, qreal) {}
    UMLWidgetController* getWidgetController() const { return 0; }
    ListPopupMenu* setupPopupMenu();

protected:
    /**
     * This is called on change of size of widget caused due to @ref
     * NewUMLRectWidget::setSize method.
     * This can be used to update the shape of the widget.
     *
     * The default implementation sets QRectF(0, 0, newSize.width(),
     * newSize.height()) as the shape of this widget.
     *
     * @param oldSize The old size of the widget which can be used for
     *                computation.
     *
     * The new size is available through NewUMLRectWidget::size()
     */
    virtual void sizeHasChanged(const QSizeF& oldSize);

    /**
     * Reimplemented to ensure widget current size fits the sizeHint
     * constraints.
     */
    void updateGeometry();
    /**
     * Set whether this widget is resizable or not.
     */
    void setResizable(bool resizable);
    /**
     * Reimplemented to provide resizing ability.
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    /**
     * Reimplemented to provide resizing ability.
     */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    /**
     * Reimplemented to provide resizing ability.
     */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    /**
     * Reimplemented to provide resizing ability.
     */
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    /**
     * Reimplemented to catch selection change notification and enable/disable hover
     * events based on whether this widget is selected or not.
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    QSizeF m_size;
    QString m_instanceName;
    bool m_showStereotype;

    bool m_resizable;
    AssociationWidgetList m_associationWidgetList;

    QRectF m_oldGeometry;
    WidgetHandle *m_widgetHandle;

    DISABLE_COPY(NewUMLRectWidget)
};

#endif //NEWUMLRECTWIDGET_H
