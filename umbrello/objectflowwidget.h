/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef OBJECTFLOWWIDGET_H
#define OBJECTFLOWWIDGET_H

#include "umlwidget.h"
#include "worktoolbar.h"

#define OBJECTFLOW_MARGIN 5
#define OBJECTFLOW_WIDTH 30
#define OBJECTFLOW_HEIGHT 10

/**
 * This class is the graphical version of a UML Object Flow.  A ObjectFlowWidget is created
 * by a @ref UMLView.  An ObjectFlowWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * The ObjectFlowWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UML object flow.
 * @author Hassan KOUCH <hkouch@hotmail.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ObjectFlowWidget : public UMLWidget {
    Q_OBJECT

public:

    /**
     * Creates a Object Flow widget.
     *
     * @param view              The parent of the widget.
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
    ObjectFlowWidget( UMLView * view, Uml::IDType id = Uml::id_None );

    /**
     *  destructor
     */
    virtual ~ObjectFlowWidget();

    /**
     * Overrides the standard paint event.
     */
    void draw(QPainter & p, int offsetX, int offsetY);



    /**
     * Sets the state in m_Text
     * 
     * @param strState The state to be set
     */
   	virtual void setState(const QString &strState);
    
    /**
     * Gets the state from the corresponding UMLObject.
     * Returns the local m_Text if m_pObject is NULL.
     *
     * @return The currently set state.
     */
    virtual QString getState() const;


	void askStateForWidget();

    /**
     * Returns the type of ObjectFlow.
     */
    // ObjectFlowType getObjectFlowType() const;

    /**
     * Sets the type of ObjectFlow.
     */
    // void setObjectFlowType( ObjectFlowType objectflowType );
//
//     /**
//      * Show a properties dialog for an ObjectFlowWidget.
//      *
//      * @return  True if we modified the ObjectFlow.

//     bool showProperties();
//
//     /**
//      * Determines whether a toolbar button represents an ObjectFlow.
//      * CHECK: currently unused - can this be removed?
//      *
//      * @param tbb               The toolbar button enum input value.
//      * @param resultType        The ObjectFlowType corresponding to tbb.
//      *                  This is only set if tbb is an ObjectFlow.
//      * @return  True if tbb represents an ObjectFlow.
//      */
//     static bool isObjectFlow( WorkToolBar::ToolBar_Buttons tbb,
//                             ObjectFlow& resultType );

    /**
     * Saves the widget to the <objectflowwidget> XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads the widget from the <objectflowwidget> XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

protected:
    /**
     * Overrides method from UMLWidget
     */
    QSize calculateSize();

    /**
     * State variable
     */
    QString m_State;


    /**
     * Type of ObjectFlow.
     */
    // ObjectFlow m_ObjectFlowType;

//public slots:

    /**
     * Captures any popup menu signals for menus it created.
     */
    // void slotMenuSelection(int sel);
};

#endif
