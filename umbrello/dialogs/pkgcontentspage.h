/*
 *  copyright (C) 2003-2004
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

#ifndef PKGCONTENTSPAGE_H
#define PKGCONTENTSPAGE_H

#include <qwidget.h>
#include <q3groupbox.h>
#include <q3listbox.h>
#include <q3ptrlist.h>

#include "../package.h"
#include "../listpopupmenu.h"

/**
 * @short The page shows all the objects that belong to a @ref UMLPackage.
 * @author Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class PkgContentsPage : public QWidget {
    Q_OBJECT
public:
    /**
     * Constructs an instance of PkgContentsPage.
     *
     * @param parent    The parent of the page.
     * @param pkg               The UMLPackage being represented.
     */
    PkgContentsPage(QWidget *parent, UMLPackage *pkg);

    /**
     * Standard destructor.
     */
    ~PkgContentsPage();

private:
    UMLPackage * m_pPackage;
    Q3ListBox * m_pContentLB;
    Q3GroupBox * m_pContentGB;
    ListPopupMenu * m_pMenu;

    /**
     * Fills the list box with the package's contents.
     */
    void fillListBox();

public slots:
    void slotDoubleClick(Q3ListBoxItem * i);
    void slotRightButtonClicked(Q3ListBoxItem */* item*/, const QPoint &/* p*/);
    void slotRightButtonPressed(Q3ListBoxItem * item, const QPoint & p);
    void slotPopupMenuSel(int id);
};

#endif
