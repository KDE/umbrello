/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef OBJECTNODEWIDGET_H
#define OBJECTNODEWIDGET_H

#include "umlwidget.h"
#include "worktoolbar.h"

#define OBJECTNODE_MARGIN 5
#define OBJECTNODE_WIDTH 30
#define OBJECTNODE_HEIGHT 10

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

    /**
     * Creates a Object Node widget.
     *
     * @param scene              The parent of the widget.
     * @param objectNodeType      The type of object node
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
    explicit ObjectNodeWidget( UMLScene * scene, ObjectNodeType objectNodeType = Normal, Uml::IDType id = Uml::id_None );


    /**
     *  destructor
     */
    virtual ~ObjectNodeWidget();

    /**
     * Overrides the standard paint event.
     */
    void paint(QPainter & p, int offsetX, int offsetY);

    /**
     * Returns the type of object node.
     */
    ObjectNodeType objectNodeType() const;
    ObjectNodeType objectNodeType(const QString& type) const;

    /**
     * Sets the type of object node.
     */
    void setObjectNodeType( ObjectNodeType objectNodeType );
    void setObjectNodeType( const QString& type ) ;

     /**
     * Sets the state of an object node when it's an objectflow.
     */
    void setState(const QString& state);

    /**
     * Returns the state of object node.
     */
    QString state();

    /**
     * Show a properties dialog for an ObjectNodeWidget.
     *
     */
    virtual void showPropertiesDialog();


    /**
     * Saves the widget to the "objectnodewidget" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads the widget from the "objectnodewidget" XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

    /**
     * Open a dialog box to select the objectNode type (Data, Buffer or Flow)
     */
    void askForObjectNodeType(UMLWidget* &targetWidget);

    /**
     * Open a dialog box to input the state of the widget
     * This box is shown only if m_ObjectNodeType = Flow
     */
    void askStateForWidget();

protected:
    /**
     * Overrides method from UMLWidget
     */
    UMLSceneSize minimumSize();

    /**
     * Type of object node.
     */
    ObjectNodeType m_ObjectNodeType;

    /**
     * State of the object node when it's an objectFlow
     */
    QString m_State;

public slots:

    /**
     * Captures any popup menu signals for menus it created.
     */
    void slotMenuSelection(QAction* action);

    void slotOk();
};

#endif
