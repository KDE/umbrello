/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CONSTRAINTLISTPAGE_H
#define CONSTRAINTLISTPAGE_H

//qt  includes
#include <QtGui/QWidget>
//kde includes
#include <karrowbutton.h>
#include <kaction.h>

//app includes
#include "../umlclassifierlistitemlist.h"
#include "classifierlistpage.h"


/**
 * A dialog page to display entity constraint properties.
 *
 * @short An extension to the classifier list dialog page to display constraint properties.
 * @author Sharan Rao
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ConstraintListPage : public ClassifierListPage
{
    Q_OBJECT
public:

    /**
     *  Sets up the ConstraintListPage
     *
     *  @param parent   The parent to the ConstraintListPage.
     *  @param classifier       The Concept to display the properties of.
     *  @param doc The UMLDoc document
     *  @param type The object type
     */
    ConstraintListPage(QWidget* parent, UMLClassifier* classifier, UMLDoc* doc, Uml::Object_Type type);

    /**
     *  Standard destructor.
     */
    ~ConstraintListPage();

private:

    KAction* newUniqueConstraintAction;
    KAction* newPrimaryKeyConstraintAction;
    KAction* newForeignKeyConstraintAction;
    KAction* newCheckConstraintAction;

    void setupActions();

private slots:

    void slotNewUniqueConstraint();
    void slotNewPrimaryKeyConstraint();
    void slotNewForeignKeyConstraint();
    void slotNewCheckConstraint();

protected:

    int relativeIndexOf(QListWidgetItem* item);

    int calculateNewIndex(Uml::Object_Type ot);

    bool greaterThan(Uml::Object_Type ct1,Uml::Object_Type ct2);

    KMenu* buttonMenu;

};

#endif
