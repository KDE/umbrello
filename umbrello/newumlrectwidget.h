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
	Q_OBJECT;
public:
    /**
     * Construct a NewUMLRectWidget associtated with object.
     *
     * @param object UMLObject with which widget should be associated.
     * @see NewUMLWidget::NewUMLWidget()
     */
	explicit NewUMLRectWidget(UMLObject *object);

    ~NewUMLRectWidget();

    /**
     * This pure virtual method should be implemented by the
     * subclasses to provide Maximum and Minimum size information for
     * this widget. This information is used in setSize and also by
     * the UMLScene.
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
	virtual QSizeF sizeHint(Qt::SizeHint which) = 0;

    /**
     * @return The current size of the widget.
     */
	QSizeF size() const {
		return m_size;
	}
    /**
     * Sets the size of the widget to \a size and its corresponding
     * shape to \a newShape. It also does cosmetic adjustments to the
     * actual boundingRect though.
     *
     * @param size The new size to be set which should effectively
     *             satisfy the sizeHint constraints.
     *
     * @param newShape The new shape corresponding to new size. Pass
     *                 an empty shape (which is also default arg) if
     *                 the shape is rectangular.
     *
     * @note The actual geometry (aka boundingRect()) of the widget
     *       can be slightly bigger than QRectF(0, 0, size,
     *       size). This is to fit the small resize hint rectangles
     *       shown when the widget is selected.  Also half the pen
     *       width should be compensated for boundingRect for the
     *       widgets painting the stroke.
     */
	void setSize(const QSizeF &size, const QPainterPath& newShape = QPainterPath());

    /**
     * This is an overloaded function for convenience.
     * Shortcut for setSize(QSizeF(w, h), newShape)
     *
     * @param w  Width to be set.
     *
     * @param h  Height to be set.
     *
     * @param newShape The new shape corresponding to new size. Pass
     *                 an empty shape (which is also default arg) if
     *                 the shape is rectangular.
     */
    void setSize(qreal w, qreal h, const QPainterPath& newShape = QPainterPath()) {
        setSize(QSizeF(w, h), newShape);
    }

    /**
     * @return The instance name of this widget(if used).
     */
	QString instanceName() const {
		return m_instanceName;
	}
    /**
     * Set the instance name for this widget. Calls updateGeometry
     * implicitly.
     */
	void setInstanceName(const QString &name);

	virtual bool isInstance() const {
		return false;
	}

    /**
     * @return A list of AssociationWidget connected with this widet.
     */
	AssociationWidgetList associationWidgetList() const {
		return m_associationWidgetList;
	}

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
    /**
     * Removes a association widget from the internal list that involves this widget.
     *
     * @param assoc The association widget that should be removed.
     */
	void removeAssociationWidget(AssociationWidget *assoc);

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

protected:
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


private:

    QSizeF m_size;
    QString m_instanceName;

    bool m_resizable;
    AssociationWidgetList m_associationWidgetList;

    // Internal variable to keep track of resizing state.
    bool m_isResizing;
};

#endif //NEWUMLRECTWIDGET_H
