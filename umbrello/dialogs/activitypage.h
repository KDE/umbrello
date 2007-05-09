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

#ifndef ACTIVITYPAGE_H
#define ACTIVITYPAGE_H
//qt includes
#include <qwidget.h>
#include <qgroupbox.h>
#include <qlistbox.h>
//kde includes
#include <karrowbutton.h>
//app includes

class StateWidget;
class ListPopupMenu;

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ActivityPage : public QWidget {
    Q_OBJECT
public:
    /**
     *  Constructor
     */
    ActivityPage( QWidget * pParent, StateWidget * pWidget );

    /**
    *   Deconstructor
    */
    ~ActivityPage();

    /**
    *   Sets up the page.
    */
    void setupPage();

    /**
    *   Sets the activities of the widget.
    */
    void updateActivities();

protected:
    /**
     *  Set the state of the widgets on the page with the given value.
     *
     *  @param  state   The state to set the widgets as.
     */
    void enableWidgets(bool state);

    /**
     *  The widget to get the activities from.
     */
    StateWidget * m_pStateWidget;

    /**
     *  Popup menu used.
     */
    ListPopupMenu * m_pMenu;
    //GUI widgets
    QListBox * m_pActivityLB;
    QGroupBox * m_pActivityGB;
    KArrowButton * m_pUpArrowB, * m_pDownArrowB, * m_pTopArrowB, *m_pBottomArrowB;
    QPushButton* m_pDeleteActivityButton;
    QPushButton* m_pRenameButton;

public slots:
    /**
     *  Popup menu item selected
     */
    void slotMenuSelection( int sel );
    void slotClicked( QListBoxItem* item );
    void slotDoubleClicked( QListBoxItem* item );
    void slotRightButtonClicked(QListBoxItem* item, const QPoint& p);
    void slotRightButtonPressed(QListBoxItem* item, const QPoint& p);
    void slotTopClicked();
    void slotUpClicked();
    void slotDownClicked();
    void slotBottomClicked();
    void slotNewActivity();
    void slotDelete();
    void slotRename();
};

#endif
