/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2003-2011                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#include "constraintlistpage.h"

#include "attribute.h"
#include "debug_utils.h"
#include "classifierlistitem.h"
#include "classifier.h"
#include "enum.h"
#include "entity.h"
#include "entityattribute.h"
#include "enumliteral.h"
#include "object_factory.h"
#include "operation.h"
#include "template.h"
#include "umldoc.h"
#include "uniqueconstraint.h"

#include <kaction.h>
#include <kdialogbuttonbox.h>
#include <klocale.h>
#include <kmenu.h>
#include <kpushbutton.h>

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QApplication>

/**
 *  Sets up the ConstraintListPage
 *
 *  @param parent   The parent to the ConstraintListPage.
 *  @param classifier       The Concept to display the properties of.
 *  @param doc The UMLDoc document
 *  @param type The object type
 */
ConstraintListPage::ConstraintListPage(QWidget* parent, UMLClassifier* classifier,
                                       UMLDoc* doc, UMLObject::ObjectType type)
  : ClassifierListPage( parent, classifier, doc, type )
{
    setupActions();

    buttonMenu = new KMenu(this );
    // add a button menu
    m_pNewClassifierListItemButton->setMenu( buttonMenu );
    buttonMenu->addAction( newPrimaryKeyConstraintAction );
    buttonMenu->addAction( newUniqueConstraintAction );
    buttonMenu->addAction( newForeignKeyConstraintAction );
    buttonMenu->addAction( newCheckConstraintAction );

    // because we order the list items. first the Unique Constraints and then the ForeignKey Constraints
    hideArrowButtons( true );
}

/**
 *  Standard destructor.
 */
ConstraintListPage::~ConstraintListPage()
{
}

void ConstraintListPage::setupActions()
{
    newUniqueConstraintAction = new KAction( i18n( "Unique Constraint..." ), this );
    connect( newUniqueConstraintAction, SIGNAL(triggered(bool)),
             this, SLOT(slotNewUniqueConstraint()));

    newPrimaryKeyConstraintAction = new KAction( i18n( "Primary Key Constraint..." ), this );
    connect( newPrimaryKeyConstraintAction, SIGNAL(triggered(bool)),
             this, SLOT(slotNewPrimaryKeyConstraint()));

    newForeignKeyConstraintAction = new KAction( i18n( "Foreign Key Constraint..." ), this );
    connect( newForeignKeyConstraintAction, SIGNAL(triggered(bool)),
             this, SLOT(slotNewForeignKeyConstraint()));

    newCheckConstraintAction = new KAction( i18n( "Check Constraint..." ), this );
    connect( newCheckConstraintAction, SIGNAL(triggered(bool)),
             this, SLOT(slotNewCheckConstraint()));
}

void ConstraintListPage::slotNewUniqueConstraint()
{
    m_itemType = UMLObject::ot_UniqueConstraint;
    ClassifierListPage::slotNewListItem();

    // shift back
    m_itemType = UMLObject::ot_EntityConstraint;
}

void ConstraintListPage::slotNewPrimaryKeyConstraint()
{
    m_itemType = UMLObject::ot_UniqueConstraint;
    ClassifierListPage::slotNewListItem();

    // set the last object created as Primary Key
    UMLEntity* ent = static_cast<UMLEntity*>( m_pClassifier );

    if ( ent == NULL ) {
        uError() << "Could not set Primary Key. Entity Value is Null";
        return;
    }

    if ( m_pLastObjectCreated!=NULL ) {
        m_bSigWaiting = true;
        ent->setAsPrimaryKey( static_cast<UMLUniqueConstraint*>(m_pLastObjectCreated ) );
        m_itemType = UMLObject::ot_EntityConstraint;
        reloadItemListBox();
    }

    // shift back
    m_itemType = UMLObject::ot_EntityConstraint;
}

void ConstraintListPage::slotNewForeignKeyConstraint()
{
    m_itemType = UMLObject::ot_ForeignKeyConstraint;
    ClassifierListPage::slotNewListItem();

    // shift back
    m_itemType = UMLObject::ot_EntityConstraint;
}

void ConstraintListPage::slotNewCheckConstraint()
{
    m_itemType = UMLObject::ot_CheckConstraint;
    ClassifierListPage::slotNewListItem();

    // shift back
    m_itemType = UMLObject::ot_EntityConstraint;
}

// /**
//  * Calculates the new index to be assigned when an object of type ot is to
//  * be added to the list box. The default Implementation is to add it to the end of the list
//  * @param ot The Object Type to be added
//  * @return The index
//  */
// int ConstraintListPage::calculateNewIndex(Uml::ObjectType ot)
// {
//     // we want to show all Unique Constraints first , followed by ForeignKey Constraints
//     UMLClassifierListItemList ucList, fkcList,  ccList;
//     ucList =  m_pClassifier->getFilteredList(UMLObject::ot_UniqueConstraint);
//     fkcList = m_pClassifier->getFilteredList(UMLObject::ot_ForeignKeyConstraint);
//     ccList =  m_pClassifier->getFilteredList(UMLObject::ot_CheckConstraint);
// 
//     int ucCount,  fkcCount, ccCount;
//     ucCount = ucList.count();
//     fkcCount = fkcList.count();
//     ccCount = ccList.count();
// 
//     int index = 0;
// 
//     if ( greaterThan( UMLObject::ot_UniqueConstraint, ot ) ) {
//         index += ucCount;
//     }
// 
//     if ( greaterThan( UMLObject::ot_ForeignKeyConstraint, ot ) ) {
//         index += fkcCount;
//     }
// 
//     if ( greaterThan( UMLObject::ot_CheckConstraint, ot ) ) {
//         index += ccCount;
//     }
// 
//     // we subtract 1 from the count as the new item is already in the list (m_List) and
//     // hence contributes to the count we obtained
//     index = index - 1;
// 
//     return index;
// }

// /**
//  * Returns the index of the Item in the List Box
//  */
// int ConstraintListPage::relativeIndexOf(QListWidgetItem* item)
// {
//     int actualIndex = ClassifierListPage::relativeIndexOf( item );
// 
//     int ucCount = m_pClassifier->getFilteredList( UMLObject::ot_UniqueConstraint ).count();
//     int fkcCount = m_pClassifier->getFilteredList( UMLObject::ot_ForeignKeyConstraint ).count();
//     //int ccCount = m_pClassifier->getFilteredList( UMLObject::ot_CheckConstraint ).count();
// 
//     //if ( m_itemType == UMLObject::ot_EntityConstraint )
//     //    return actualIndex;
// 
//     int newIndex = actualIndex;
// 
//     if ( !greaterThan( m_itemType, UMLObject::ot_UniqueConstraint ) ) {
//         newIndex -= ucCount;
//     }
// 
//     if ( !greaterThan( m_itemType, UMLObject::ot_ForeignKeyConstraint ) ) {
//         newIndex -= fkcCount;
//     }
// 
//     return newIndex;
// }

/**
 * Will return true if ot1 has a higher (top)  place in the list than ot2
 *
 * @param ct1 Contraint Type 1
 * @param ct2 Constraint Type 2
 * @return true if ct1 is to be shown above ct2 else false
 */
bool ConstraintListPage::greaterThan(UMLObject::ObjectType ct1, UMLObject::ObjectType ct2)
{
    // define ordering
    switch( ct1 ) {
       case UMLObject::ot_EntityConstraint:
       case UMLObject::ot_UniqueConstraint:
           // Unique Constraint greater than all others
            return true;
            break;
       case UMLObject::ot_ForeignKeyConstraint:
           if ( ct2 != UMLObject::ot_UniqueConstraint )
             return true;
           else
               return false;
           break;
       case UMLObject::ot_CheckConstraint:
           if ( ct2 != UMLObject::ot_CheckConstraint )
               return false;
           else
               return true;
           break;
       default:
           return false;
    }
}

#include "constraintlistpage.moc"
