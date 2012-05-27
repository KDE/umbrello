/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef OBJECTNODEWIDGET_H
#define OBJECTNODEWIDGET_H

#include "umlwidget.h"

/**
 * This class is the graphical version of a UML Object Node which is
 * used in Activity diagrams
 *
 * @short  A graphical version of a UML Activity.
 * @author Florence Mattler <florence.mattler@libertysurf.fr>
 * @author Gopala Krishna
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ObjectNodeWidget : public UMLWidget
{
    Q_OBJECT
    Q_PROPERTY(QString state READ state WRITE setState)

public:
    enum ObjectNodeType
    {
        Normal,
        Data,
        Buffer,
        Flow
    };

    explicit ObjectNodeWidget( ObjectNodeType objectNodeType = Normal, Uml::IDType id = Uml::id_None );
    virtual ~ObjectNodeWidget();

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    /// Returns the type of object node.
    ObjectNodeType objectNodeType() const {
        return m_objectNodeType;
    }
    static ObjectNodeType stringToObjectNodeType(const QString& objectNodeType);

    void setObjectNodeType( ObjectNodeType objectNodeType );

    /// @return the state of object node. (when objectFlow)
    QString state() const {
        return m_state;
    }
    void setState(const QString& state);

    virtual void showPropertiesDialog();

    void askStateForWidget();
    void askForObjectNodeType(UMLWidget* &targetWidget);

    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    virtual bool loadFromXMI(QDomElement& qElement);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    virtual void updateGeometry();
    virtual void updateTextItemGroups();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);

private:
    enum {
        GroupIndex = 0
    };
    static const QSizeF MinimumSize;

    ObjectNodeType m_objectNodeType;
    QString m_state;
    QLineF m_objectFlowLine;
};

#endif
