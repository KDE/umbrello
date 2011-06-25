/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ASSOCPAGE_H
#define ASSOCPAGE_H

#include <QtGui/QWidget>
#include <QtGui/QListWidgetItem>

#include "umlobject.h"
#include "associationwidgetlist.h"
#include "associationwidget.h"
#include "listpopupmenu.h"

class QGroupBox;
class QListWidget;
class UMLView;

/**
 * Displays a page on the tabbed dialog window of @ref ClassPropDlg.
 * The page shows all the Associations that belong to a UMLClassifier.
 *
 * @short The page shows all the Associations that belong to a UMLClassifier.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AssocPage : public QWidget
{
    Q_OBJECT

public:
    AssocPage(QWidget *parent, UMLView * v, UMLObject * o);
    ~AssocPage();

private:
    UMLObject * m_pObject;
    UMLView * m_pView;
    QListWidget * m_pAssocLW;
    QGroupBox * m_pAssocGB;
    AssociationWidgetList m_List;
    ListPopupMenu * m_pMenu;

    void fillListBox();

public slots:
    void slotDoubleClick(QListWidgetItem * item);
    void slotRightButtonPressed(const QPoint & p);
    void slotPopupMenuSel(QAction* action);
};

#endif
