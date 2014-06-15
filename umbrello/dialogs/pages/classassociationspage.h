/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef ASSOCPAGE_H
#define ASSOCPAGE_H

#include "associationwidgetlist.h"
#include "dialogpagebase.h"

class AssociationWidget;
class ListPopupMenu;
class QGroupBox;
class QListWidget;
class QListWidgetItem;
class UMLObject;
class UMLScene;

/**
 * Displays a page on the tabbed dialog window of @ref ClassPropertiesDialog.
 * The page shows all the Associations that belong to a UMLClassifier.
 *
 * @short The page shows all the Associations that belong to a UMLClassifier.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class ClassAssociationsPage : public DialogPageBase
{
    Q_OBJECT
public:
    ClassAssociationsPage(QWidget *parent, UMLScene * scene, UMLObject * o);
    ~ClassAssociationsPage();

private:
    UMLObject             *m_pObject;
    UMLScene              *m_pScene;
    QListWidget           *m_pAssocLW;
    QGroupBox             *m_pAssocGB;
    AssociationWidgetList  m_List;

    void fillListBox();

public slots:
    void slotDoubleClick(QListWidgetItem *item);
    void slotRightButtonPressed(const QPoint &p);
    void slotMenuSelection(QAction* action);
};

#endif
