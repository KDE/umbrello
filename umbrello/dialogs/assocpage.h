/*
 *  copyright (C) 2002-2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef ASSOCPAGE_H
#define ASSOCPAGE_H

#include <qwidget.h>
#include <q3groupbox.h>
#include <q3listbox.h>
#include <q3ptrlist.h>

#include "../umlobject.h"
#include "../umlview.h"
#include "../associationwidgetlist.h"
#include "../associationwidget.h"
#include "../listpopupmenu.h"


/**
 * Displays a page on the tabbed dialog window of @ref ClassPropDlg.
 * The page shows all the Associations that belong to a @ref ConceptWidget.
 *
 *
 * @see ClassPropDlg

 * @see ConceptWidget
 *
 * @short The page shows all the Associations that belong to a @ref ConceptWidget.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AssocPage : public QWidget {
    Q_OBJECT
public:
    /**
     *  Constructs an instance of AssocPage.
     *
     *  @param  parent  The parent of the page
     *  @param  v       The view the UMLObject being represented.
     *  @param  o       The UMLObject being represented
     */
    AssocPage(QWidget *parent, UMLView * v, UMLObject * o);

    /**
     *  Standard deconstructor.
     */
    ~AssocPage();
private:
    UMLObject * m_pObject;
    UMLView * m_pView;
    Q3ListBox * m_pAssocLB;
    Q3GroupBox * m_pAssocGB;
    AssociationWidgetList m_List;
    ListPopupMenu * m_pMenu;

    /**
     *  Fills the list box with the objects associations.
     */
    void fillListBox();
public slots:
    void slotDoubleClick(Q3ListBoxItem * i);
    void slotRightButtonClicked(Q3ListBoxItem */* item*/, const QPoint &/* p*/);
    void slotRightButtonPressed(Q3ListBoxItem * item, const QPoint & p);
    void slotPopupMenuSel(int id);
};

#endif
