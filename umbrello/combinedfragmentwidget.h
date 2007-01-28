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

#ifndef COMBINEDFRAGMENTWIDGET_H
#define COMBINEDFRAGMENTWIDGET_H

#include "umlwidget.h"
#include "worktoolbar.h"

#define COMBINED_FRAGMENT_MARGIN 5
#define COMBINED_FRAGMENT_WIDTH 30
#define COMBINED_FRAGMENT_HEIGHT 10

/**
 * This class is the graphical version of a UML combined fragment.  A combinedfragmentWidget is created
 * by a @ref UMLView.  An combinedfragmentWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * The combinedfragmentWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UML combined fragment.
 * @author Hassan KOUCH <hkouch@hotmail.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class CombinedFragmentWidget : public UMLWidget {
    Q_OBJECT

public:
    enum CombinedFragmentType
    {
        Normal,
        Ref,
        Opt
    };

    /**
     * Creates a Activity widget.
     *
     * @param view              The parent of the widget.
     * @param combinedfragmentType      The type of combined fragment.
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
    CombinedFragmentWidget( UMLView * view, CombinedFragmentType combinedfragmentType = Opt, Uml::IDType id = Uml::id_None );


    /**
     *  destructor
     */
    virtual ~CombinedFragmentWidget();

    /**
     * Overrides the standard paint event.
     */
    void draw(QPainter & p, int offsetX, int offsetY);

    /**
     * Returns the type of combined fragment.
     */
    CombinedFragmentType getCombinedFragmentType() const;

    /**
     * Sets the type of combined fragment.
     */
    void setCombinedFragmentType( CombinedFragmentType combinedfragmentType );

    /**
     * Show a properties dialog for a CombinedFragmentWidget.
     *
     * @return  True if we modified the Combined Fragment.
     */
    bool showProperties();

    /**
     * Determines whether a toolbar button represents a combined fragment.
     * CHECK: currently unused - can this be removed?
     *
     * @param tbb               The toolbar button enum input value.
     * @param resultType        The CombinedFragmentType corresponding to tbb.
     *                  This is only set if tbb is a CombinedFragment.
     * @return  True if tbb represents a CombinedFragment.
     */
    static bool isCombinedFragment( WorkToolBar::ToolBar_Buttons tbb,
                            CombinedFragmentType& resultType );

    /**
     * Saves the widget to the <combinedFragmentwidget> XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads the widget from the <CombinedFragmentwidget> XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

protected:
    /**
     * Overrides method from UMLWidget
     */
    QSize calculateSize();

    /**
     * Type of CombinedFragment.
     */
    CombinedFragmentType m_CombinedFragment;

public slots:

    /**
     * Captures any popup menu signals for menus it created.
     */
    void slotMenuSelection(int sel);
};

#endif
