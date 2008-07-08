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

class NewUMLRectWidget : public NewUMLWidget
{
	Q_OBJECT
public:
    static const QSizeF DefaultMinimumSize;
    static const QSizeF DefaultMaximumSize;
    static const QSizeF DefaultPreferredSize;

	explicit NewUMLRectWidget(UMLObject *object);
    ~NewUMLRectWidget();

    virtual QSizeF sizeHint(Qt::SizeHint which);
    QSizeF size() const {
		return m_size;
	}
    void setSize(const QSizeF &size);
    void setSize(qreal width, qreal height) {
        setSize(QSizeF(width, height));
    }
    bool isResizable() const {
        return m_resizable;
    }
    QRectF rect() const {
        return QRectF(QPointF(0, 0), size());
    }

    QString instanceName() const {
		return m_instanceName;
	}
    void setInstanceName(const QString &name);
    virtual bool isInstance() const {
		return false;
	}

    bool showStereotype() const { return m_showStereotype; }
    void setShowStereotype(bool b);

    AssociationWidgetList associationWidgetList() const {
		return m_associationWidgetList;
	}
    void addAssociationWidget(AssociationWidget *assoc);
    void removeAssociationWidget(AssociationWidget *assoc);
    void adjustConnectedAssociations();

    void showPropertiesDialog();
    void setupContextMenuActions(ListPopupMenu &menu);

    bool loadFromXMI(QDomElement &qElement);
    void saveToXMI(QDomDocument &qDoc, QDomElement &qElement);

protected:
    virtual void sizeHasChanged(const QSizeF& oldSize);
    void updateGeometry();
    void setResizable(bool resizable);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    QSizeF m_size;
    QString m_instanceName;
    bool m_showStereotype;

    bool m_resizable;
    AssociationWidgetList m_associationWidgetList;

    QRectF m_oldGeometry;
    WidgetHandle *m_widgetHandle;

    DISABLE_COPY(NewUMLRectWidget);

public:
    //////////////////  DEPRECATED //////////////////////////
    NewUMLRectWidget(UMLScene *scene, const Uml::IDType & _id = Uml::id_None);
    NewUMLRectWidget(UMLScene *scene, UMLObject *obj);

    qreal getWidth() const { return size().width(); }
    void setWidth(qreal w) { setSize(w, getHeight()); }
    qreal getHeight() const { return size().height(); }
    void setHeight(qreal h) { setSize(getWidth(), h); }

    QString getInstanceName() const { return instanceName(); }
    bool getIsInstance() const { return isInstance(); }
    bool getShowStereotype() const { return showStereotype(); }

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

    AssociationWidgetList getAssocList() const { return associationWidgetList(); }
    void addAssoc(AssociationWidget *assoc) { addAssociationWidget(assoc); }
    void removeAssoc(AssociationWidget *assoc) { removeAssociationWidget(assoc); }
};

#endif //NEWUMLRECTWIDGET_H
