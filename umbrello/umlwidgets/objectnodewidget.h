/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
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

    explicit ObjectNodeWidget(UMLScene * scene, ObjectNodeType objectNodeType = Normal, Uml::ID::Type id = Uml::ID::None);
    virtual ~ObjectNodeWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);

    ObjectNodeType objectNodeType() const;
    static ObjectNodeType toObjectNodeType(const QString& type);

    void setObjectNodeType(ObjectNodeType objectNodeType);
    void setObjectNodeType(const QString& type) ;

    void setState(const QString& state);
    QString state() const;

    virtual bool showPropertiesDialog();

    void askStateForWidget();
    void askForObjectNodeType(UMLWidget* &targetWidget);

    virtual void saveToXMI(QXmlStreamWriter& writer);
    virtual bool loadFromXMI(QDomElement& qElement);

protected:
    QSizeF minimumSize() const;

public slots:
    void slotMenuSelection(QAction* action);
    void slotOk();

private:
    ObjectNodeType m_objectNodeType;  ///< type of object node
    QString        m_state;           ///< state of object node when it's an objectFlow

};

#endif
