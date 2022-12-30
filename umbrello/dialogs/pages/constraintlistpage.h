/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CONSTRAINTLISTPAGE_H
#define CONSTRAINTLISTPAGE_H

//app includes
#include "umlclassifierlistitemlist.h"
#include "classifierlistpage.h"

//qt  includes
#include <QWidget>

class QAction;
class QMenu;

/**
 * A dialog page to display entity constraint properties.
 *
 * @short An extension to the classifier list dialog page to display constraint properties.
 * @author Sharan Rao
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ConstraintListPage : public ClassifierListPage
{
    Q_OBJECT
public:

    ConstraintListPage(QWidget* parent, UMLClassifier* classifier,
                       UMLDoc* doc, UMLObject::ObjectType type);
    virtual ~ConstraintListPage();

private:

    QAction* newUniqueConstraintAction;
    QAction* newPrimaryKeyConstraintAction;
    QAction* newForeignKeyConstraintAction;
    QAction* newCheckConstraintAction;
    void setupActions();

private slots:

    void slotNewUniqueConstraint();
    void slotNewPrimaryKeyConstraint();
    void slotNewForeignKeyConstraint();
    void slotNewCheckConstraint();
    virtual UMLClassifierListItemList getItemList();

protected:

    bool greaterThan(UMLObject::ObjectType ct1, UMLObject::ObjectType ct2);

    QMenu* buttonMenu;

};

#endif
