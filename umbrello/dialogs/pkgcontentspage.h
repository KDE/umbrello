/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#ifndef PKGCONTENTSPAGE_H
#define PKGCONTENTSPAGE_H

#include "listpopupmenu.h"
#include "package.h"

#include <QtGui/QWidget>
#include <QtGui/QGroupBox>
#include <QtGui/QListWidget>

class QListWidgetItem;

/**
 * @short The page shows all the objects that belong to a @ref UMLPackage.
 * @author Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class PkgContentsPage : public QWidget
{
    Q_OBJECT
public:
    PkgContentsPage(QWidget *parent, UMLPackage *pkg);
    ~PkgContentsPage();

private:
    UMLPackage * m_package;
    QListWidget * m_contentLW;
    QGroupBox * m_contentGB;
    ListPopupMenu * m_menu;

    void fillListBox();

public slots:
    void slotDoubleClick(QListWidgetItem * item);
    void slotShowContextMenu(const QPoint&);
    void slotPopupMenuSel(QAction* action);
};

#endif
