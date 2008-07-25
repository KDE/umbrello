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

#include "newumlrectwidget.h"

#include "listpopupmenu.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "umlscene.h"
#include "widget_utils.h"
#include "widgethandle.h"

#include <QtGui/QDialog>
#include <QtGui/QGraphicsSceneHoverEvent>

const QSizeF NewUMLRectWidget::DefaultMinimumSize(50, 20);
const QSizeF NewUMLRectWidget::DefaultMaximumSize(1000, 1000);

/**
 * Construct a NewUMLRectWidget associtated with object.
 *
 * @param object UMLObject with which widget should be associated.
 * @see NewUMLWidget::NewUMLWidget()
 */
NewUMLRectWidget::NewUMLRectWidget(UMLObject *object) :
    NewUMLWidget(object),
    m_size(20, 20),
	m_minimumSize(NewUMLRectWidget::DefaultMinimumSize),
	m_maximumSize(NewUMLRectWidget::DefaultMaximumSize),
	m_margin(5.0), // Default margin size
	m_isInstance(false),
    m_resizable(true),
    m_widgetHandle(0)
{
}

/// Destructor
NewUMLRectWidget::~NewUMLRectWidget()
{
	qDeleteAll(m_textItemGroups);
}

/**
 * Sets \a "newSize + QSizeF(@ref margin() * 2, @ref margin() * 2)" as
 * the minimum size.
 *
 * This method only sets the variable m_minimumSize and doesn't update
 * the geometry. The geometry should be explicitly updated using
 * NewUMLWidget::updateGeometry()
 */
void NewUMLRectWidget::setMinimumSize(const QSizeF& newSize)
{
	qreal m = 2 * margin();
	m_minimumSize = newSize;
	m_minimumSize.rwidth() += m;
	m_minimumSize.rheight() += m;
}

/**
 * Sets \a "newSize + QSizeF(@ref margin() * 2, @ref margin() * 2)" as
 * the maximum size.
 *
 * This method only sets the variable m_maximumSize and doesn't update
 * the geometry. The geometry should be explicitly updated using
 * NewUMLWidget::updateGeometry()
 */
void NewUMLRectWidget::setMaximumSize(const QSizeF& newSize)
{
	qreal m = 2 * margin();
	m_maximumSize = newSize;
	m_maximumSize.rwidth() += m;
	m_maximumSize.rheight() += m;
}

/**
 * Sets the size of the widget to \a size.
 *
 * Also notifies self and subclasses with @ref SizeHasChanged
 * notification after setting the new size.
 * @see NewUMLRectWidget::attributeChange
 *
 * @param size The new size (minimumSize < size < maximumSize)
 */
void NewUMLRectWidget::setSize(const QSizeF &size)
{
    const QSizeF oldSize = m_size;

	// Satisfy (minimumSize < size < maximumSize) requirement.
	m_size = minimumSize().expandedTo(size);
    m_size = maximumSize().boundedTo(m_size);

    QRectF boundRect = rect();
    // Adjust bounding rect with half the pen width(lineWidth).
    const qreal hpw = 0.5 * lineWidth();
    boundRect.adjust(-hpw, -hpw, hpw, hpw);

	// Now set the bounding rect, (slightly larger or equal to rect())
    setBoundingRect(boundRect);

    // Notify self and subclasses about change of size.
    attributeChange(SizeHasChanged, oldSize);
}

/**
 * Sets the margin of this widget to \a margin. This method only
 * updates the variable. To see the effective margin, updateGeometry()
 * should be called.
 */
void NewUMLRectWidget::setMargin(qreal margin)
{
	m_margin = margin;
}

/**
 * Set the instance name for this widget. Calls updateGeometry
 * implicitly.
 */
void NewUMLRectWidget::setInstanceName(const QString &name)
{
    m_instanceName = name;
	// No need for attributeChange notification mechanism as of now.
    updateTextItemGroups();
}

/// Sets whether this object is instance or not.
void NewUMLRectWidget::setIsInstance(bool b)
{
	m_isInstance = b;
	updateTextItemGroups();
}

void NewUMLRectWidget::setShowStereotype(bool b)
{
    m_showStereotype = b;
	// No need for attributeChange notification mechanism as of now.
    updateTextItemGroups();
}

/**
 * Adds a association widget to the internal list that involves this
 * widget.
 *
 * @param assoc An association widget connected to this widget.
 */
void NewUMLRectWidget::addAssociationWidget(AssociationWidget *assoc)
{
    if(!m_associationWidgetList.contains(assoc)) {
        m_associationWidgetList << assoc;
    }
}

/**
 * Removes a association widget from the internal list that involves this widget.
 *
 * @param assoc The association widget that should be removed.
 */
void NewUMLRectWidget::removeAssociationWidget(AssociationWidget *assoc)
{
    m_associationWidgetList.removeAll(assoc);
}

void NewUMLRectWidget::showPropertiesDialog()
{
    QDialog stubDialog;
    stubDialog.setWindowTitle("Stub.. To be worked out still.");
    stubDialog.exec();
}

void NewUMLRectWidget::setupContextMenuActions(ListPopupMenu &menu)
{
    menu.addAction("Stub!!");
}

/**
 * Adjusts the position and lines of connected association
 * widgets. This method is used usually after this widget moves
 * requiring assocition widgets to be updated.
 */
void NewUMLRectWidget::adjustConnectedAssociations()
{
    //TODO: Implement this once AssociationWidget's are implemented.
}

bool NewUMLRectWidget::loadFromXMI(QDomElement &qElement)
{
    if(!NewUMLWidget::loadFromXMI(qElement))
        return false;

    qreal h  = qElement.attribute("height", "-1").toDouble();
    qreal w = qElement.attribute("width", "-1").toDouble();
    if(h < 0) {
        h = 20;
    }
    if(w < 0) {
        w = 20;
    }
    m_size.setWidth(w);
    m_size.setHeight(h);
    updateGeometry(); // Force updation of the shape as well.

    QString instName = qElement.attribute("instancename", QString());
    setInstanceName(instName);

    return true;
}

void NewUMLRectWidget::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    NewUMLWidget::saveToXMI(qDoc, qElement);

    const QSizeF sz = size();
    qElement.setAttribute("width", sz.width());
    qElement.setAttribute("height", sz.height());

    if(!m_instanceName.isEmpty()) {
        qElement.setAttribute("instancename", m_instanceName);
    }
}

/**
 * Reimplemented from NewUMLWidget::attributeChange to handle change
 * of size, change of font and change of font color.
 */
QVariant NewUMLRectWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
	if(change == SizeHasChanged) {
		// We should update the shape of this widget.
		QPainterPath newShape;
		newShape.addRect(rect());
		setShape(newShape);

		// Also update widget Handles position.
		if(m_widgetHandle) {
			m_widgetHandle->updateHandlePosition();
		}
		return QVariant();
	}
	else if(change == FontHasChanged) {
		foreach(TextItemGroup *group, m_textItemGroups) {
			group->setFont(font());
		}
	}
	else if(change == FontColorHasChanged) {
		foreach(TextItemGroup *group, m_textItemGroups) {
			group->setFontColor(fontColor());
		}
	}
	else if(change == NameHasChanged) {
		updateTextItemGroups();
		// No need for base method here as updateTextItemGroups
		// updates the geometry too.
		return QVariant();
	}

	// Pass on remaining handling responsibility to base method.
	return NewUMLWidget::attributeChange(change, oldValue);
}

/**
 * Reimplemented to ensure widget current size fits the sizeHint
 * constraints.
 */
void NewUMLRectWidget::updateGeometry()
{
    // The idea here is to simply call setSize with current size which
    // will take care of the size hint constraints.

    // Subclasses might set the minimum and maximum sizes in the
    // overriden version.
    setSize(m_size);
}

/**
 * Set whether this widget is resizable or not.
 */
void NewUMLRectWidget::setResizable(bool resizable)
{
    m_resizable = resizable;
	if(!m_resizable) {
		delete m_widgetHandle;
		m_widgetHandle = 0;
	}
}

void NewUMLRectWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    NewUMLWidget::mousePressEvent(event);
    m_geometryBeforeResize = rect(); // save the current geometry
}

void NewUMLRectWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
    NewUMLWidget::mouseMoveEvent(e);
}

void NewUMLRectWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    NewUMLWidget::mouseReleaseEvent(event);
}

void NewUMLRectWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
}

/**
 * Reimplemented to catch selection change notification and
 * enable/disable widget handles based on whether this widget is
 * selected or not.
 */
QVariant NewUMLRectWidget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == QGraphicsItem::ItemSelectedHasChanged) {
        bool selection = value.toBool();
        if(selection) {
            if(!m_widgetHandle) {
                m_widgetHandle = new WidgetHandle(this);
            }
        }
        else {
            delete m_widgetHandle;
            m_widgetHandle = 0;
        }
    }
    return NewUMLWidget::itemChange(change, value);
}

/**
 * Creates a new TextItemGroup, appends it to internal list -
 * m_textItemGroups and returns a pointer to the same.
 */
TextItemGroup* NewUMLRectWidget::createTextItemGroup()
{
	TextItemGroup *newGrp = new TextItemGroup(this);
    m_textItemGroups.append(newGrp);
	return newGrp;
}

/**
 * @return Index of the group in the internal list - m_textItemGroups
 */
int NewUMLRectWidget::indexOfTextItemGroup(TextItemGroup *group) const
{
	return m_textItemGroups.indexOf(group);
}

/**
 * @return TextItemGroup present at index in internal list -
 *         m_textItemGroups
 */
TextItemGroup* NewUMLRectWidget::textItemGroupAt(int index) const
{
	if(index < 0 || index >= m_textItemGroups.size()) {
		qFatal("NewUMLRectWidget::textItemGroupAt: Invalid index %d", index);
	}
	return m_textItemGroups[index];
}

/**
 * Subclasses should reimlement this method to update the TextItem's
 * texts and properties for all the TextItemGroup's of this widget i.e
 * \a m_textItemGroups.
 */
void NewUMLRectWidget::updateTextItemGroups()
{
	// Update the geometry as we don't know about texts.
	updateGeometry();
}

/**
 * Reimplmented from NewUMLWidget::slotUMLObjectDataChanged to update
 * the texts of the TextItemGroups
 */
void NewUMLRectWidget::slotUMLObjectDataChanged()
{
    updateTextItemGroups();
}

/////////////////////////// DEPRECATED STUFF ///////////////////////////
NewUMLRectWidget::NewUMLRectWidget(UMLScene *scene, UMLObject *object) :
    NewUMLWidget(scene, object),
    m_size(20, 20),
	m_minimumSize(NewUMLRectWidget::DefaultMinimumSize),
	m_maximumSize(NewUMLRectWidget::DefaultMaximumSize),
	m_margin(5),
	m_isInstance(false),
	m_resizable(true),
    m_widgetHandle(0)
{
}

NewUMLRectWidget::NewUMLRectWidget(UMLScene *scene, const Uml::IDType & id) :
    NewUMLWidget(scene, id),
    m_size(20, 20),
	m_minimumSize(NewUMLRectWidget::DefaultMinimumSize),
	m_maximumSize(NewUMLRectWidget::DefaultMaximumSize),
	m_margin(5),
	m_isInstance(false),
	m_resizable(true),
    m_widgetHandle(0)
{
}

ListPopupMenu* NewUMLRectWidget::setupPopupMenu()
{
    delete m_pMenu;
    m_pMenu = new ListPopupMenu(0, this, false, false);
    return m_pMenu;
}

#include "newumlrectwidget.moc"
