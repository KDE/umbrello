/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CLASSASSOCIATIONSPAGE_H
#define CLASSASSOCIATIONSPAGE_H

#include "associationwidgetlist.h"
#include "dialogpagebase.h"

class AssociationWidget;
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
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ClassAssociationsPage : public DialogPageBase
{
    Q_OBJECT
public:
    ClassAssociationsPage(QWidget *parent, UMLScene * scene, UMLObject * o);
    ~ClassAssociationsPage();

    Q_SLOT void slotDoubleClick(QListWidgetItem *item);
    Q_SLOT void slotRightButtonPressed(const QPoint &p);
    Q_SLOT void slotMenuSelection(QAction* action);
private:
    UMLObject             *m_pObject;
    UMLScene              *m_pScene;
    QListWidget           *m_pAssocLW;
    QGroupBox             *m_pAssocGB;
    AssociationWidgetList  m_List;

    void fillListBox();
};

#endif
