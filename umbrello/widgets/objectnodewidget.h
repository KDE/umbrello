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
    static ObjectNodeType stringToObjectNodeType(const QString& objectNodeType);

    explicit ObjectNodeWidget(ObjectNodeType objectNodeType = Normal, Uml::IDType id = Uml::id_None);
    virtual ~ObjectNodeWidget();

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    void setObjectNodeType(ObjectNodeType objectNodeType);
    ObjectNodeType objectNodeType() const;

    void setState(const QString& state);
    QString state() const;

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

    ObjectNodeType m_objectNodeType;  ///< type of object node
    QString        m_state;           ///< state of object node when it's an objectFlow
    QLineF         m_objectFlowLine;
};

#endif
