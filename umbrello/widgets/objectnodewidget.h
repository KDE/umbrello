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
 * This class is the graphical version of a UML Object Node.  A ObjectNodeWidget is created
 * by a @ref UMLView.  An ObjectNodeWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * The ObjectNodeWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UML Activity.
 * @author Florence Mattler <florence.mattler@libertysurf.fr>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ObjectNodeWidget : public UMLWidget
{
    Q_OBJECT

public:
    enum ObjectNodeType
    {
        Normal,
        Data,
        Buffer,
        Flow
    };

    explicit ObjectNodeWidget(UMLScene * scene, ObjectNodeType objectNodeType = Normal, Uml::IDType id = Uml::id_None);
    virtual ~ObjectNodeWidget();

    void draw(QPainter & p, int offsetX, int offsetY);

    ObjectNodeType objectNodeType() const;
    ObjectNodeType objectNodeType(const QString& type) const;

    void setObjectNodeType(ObjectNodeType objectNodeType);
    void setObjectNodeType(const QString& type) ;

    void setState(const QString& state);
    QString state() const;

    virtual void showPropertiesDialog();

    void askStateForWidget();
    void askForObjectNodeType(UMLWidget* &targetWidget);

    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    virtual bool loadFromXMI(QDomElement& qElement);

protected:
    UMLSceneSize minimumSize();

public slots:
    void slotMenuSelection(QAction* action);
    void slotOk();

private:
    ObjectNodeType m_objectNodeType;  ///< type of object node
    QString        m_state;           ///< state of object node when it's an objectFlow

};

#endif
