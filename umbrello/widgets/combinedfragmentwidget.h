/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef COMBINEDFRAGMENTWIDGET_H
#define COMBINEDFRAGMENTWIDGET_H

#include <QtCore/QList>

#include "umlwidget.h"
#include "worktoolbar.h"
#include "floatingdashlinewidget.h"

#define COMBINED_FRAGMENT_MARGIN 5
#define COMBINED_FRAGMENT_WIDTH 100
#define COMBINED_FRAGMENT_HEIGHT 50

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
class CombinedFragmentWidget : public UMLWidget
{
    Q_OBJECT

public:
    enum CombinedFragmentType
    {
        Ref = 0,
        Opt,
        Break,
        Loop,
        Neg,
        Crit,
        Ass,
        Alt,
        Par
    };

    /**
     * Creates a Combined Fragment widget.
     *
     * @param scene              The parent of the widget.
     * @param combinedfragmentType      The type of combined fragment.
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
    explicit CombinedFragmentWidget( UMLScene * scene, CombinedFragmentType combinedfragmentType = Ref, Uml::IDType id = Uml::id_None );


    /**
     *  destructor
     */
    virtual ~CombinedFragmentWidget();

    /**
     * Overrides the standard paint event.
     */
    void paint(QPainter & p, int offsetX, int offsetY);

    /**
     * Returns the type of combined fragment.
     */
    CombinedFragmentType combinedFragmentType() const;
    CombinedFragmentType combinedFragmentType(const QString& combinedfragmentType) const;

    /**
     * Sets the type of combined fragment.
     */
    void setCombinedFragmentType( CombinedFragmentType combinedfragmentType );
    void setCombinedFragmentType( const QString& combinedfragmentType );

    void askNameForWidgetType(UMLWidget* &targetWidget, const QString& dialogTitle,
                      const QString& dialogPrompt, const QString& defaultName);

    /**
     * Saves the widget to the "combinedFragmentwidget" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads the widget from the "CombinedFragmentwidget" XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

public slots:
    /**
     * Overrides the function from UMLWidget.
     *
     * @param action  The command to be executed.
     */
    void slotMenuSelection(QAction* action);


protected:
    /**
     * Overrides method from UMLWidget
     */
    UMLSceneSize minimumSize();

    /**
     * Type of CombinedFragment.
     */
    CombinedFragmentType m_CombinedFragment;

private:
    /**
     * Dash lines of an alternative or parallel combined fragment
     */
    QList<FloatingDashLineWidget*> m_dashLines ;
};

#endif
