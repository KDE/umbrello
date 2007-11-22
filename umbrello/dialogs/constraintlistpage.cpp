/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2003-2007                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#include "constraintlistpage.h"
#include "../attribute.h"
#include "../classifierlistitem.h"
#include "../classifier.h"
#include "../enum.h"
#include "../entity.h"
#include "../entityattribute.h"
#include "../enumliteral.h"
#include "../object_factory.h"
#include "../operation.h"
#include "../template.h"
#include "../umldoc.h"
#include "../uniqueconstraint.h"

#include <kdebug.h>
#include <kdialogbuttonbox.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <qlayout.h>

//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>



ConstraintListPage::ConstraintListPage(QWidget* parent, UMLClassifier* classifier, UMLDoc* doc, Uml::Object_Type type):ClassifierListPage( parent, classifier, doc, type )
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


ConstraintListPage::~ConstraintListPage()
{
}

void ConstraintListPage::setupActions()
{
    newUniqueConstraintAction = new KAction( i18n( "Unique Constraint..." ), this );
    connect( newUniqueConstraintAction, SIGNAL( triggered( bool ) ), this , SLOT( slotNewUniqueConstraint() ) );

    newPrimaryKeyConstraintAction = new KAction( i18n( "Primary Key Constraint..." ), this );
    connect( newPrimaryKeyConstraintAction, SIGNAL( triggered( bool ) ), this ,  SLOT( slotNewPrimaryKeyConstraint() ) );

    newForeignKeyConstraintAction = new KAction( i18n( "Foreign Key Constraint..." ), this );
    connect( newForeignKeyConstraintAction, SIGNAL( triggered( bool ) ), this ,  SLOT( slotNewForeignKeyConstraint() ) );

    newCheckConstraintAction = new KAction( i18n( "Check Constraint..." ), this );
    connect( newCheckConstraintAction, SIGNAL( triggered( bool ) ), this ,  SLOT( slotNewCheckConstraint() ) );
}


void ConstraintListPage::slotNewUniqueConstraint()
{
    m_itemType = Uml::ot_UniqueConstraint;
    ClassifierListPage::slotNewListItem();

    // shift back
    m_itemType = Uml::ot_EntityConstraint;
}

void ConstraintListPage::slotNewPrimaryKeyConstraint()
{
    m_itemType = Uml::ot_UniqueConstraint;
    ClassifierListPage::slotNewListItem();

    // set the last object created as Primary Key
    UMLEntity* ent = static_cast<UMLEntity*>( m_pClassifier );

    if ( ent == NULL ) {
        uError()<<"Couldn't set Primary Key. Entity Value is Null"<<endl;
        return;
    }

    if ( m_pLastObjectCreated!=NULL ) {
        m_bSigWaiting = true;
        ent->setAsPrimaryKey( static_cast<UMLUniqueConstraint*>(m_pLastObjectCreated ) );
        m_itemType = Uml::ot_EntityConstraint;
        reloadItemListBox();
    }

    // shift back
    m_itemType = Uml::ot_EntityConstraint;
}

void ConstraintListPage::slotNewForeignKeyConstraint()
{
    m_itemType = Uml::ot_ForeignKeyConstraint;
    ClassifierListPage::slotNewListItem();

    // shift back
    m_itemType = Uml::ot_EntityConstraint;
}

void ConstraintListPage::slotNewCheckConstraint()
{
    m_itemType = Uml::ot_CheckConstraint;
    ClassifierListPage::slotNewListItem();

    // shift back
    m_itemType = Uml::ot_EntityConstraint;
}

int ConstraintListPage::calculateNewIndex(Uml::Object_Type ot)
{
    // we want to show all Unique Constraints first , followed by ForeignKey Constraints
    UMLClassifierListItemList ucList, fkcList,  ccList;
    ucList =  m_pClassifier->getFilteredList(Uml::ot_UniqueConstraint);
    fkcList = m_pClassifier->getFilteredList(Uml::ot_ForeignKeyConstraint);
    ccList =  m_pClassifier->getFilteredList(Uml::ot_CheckConstraint);

    int ucCount,  fkcCount, ccCount;
    ucCount = ucList.count();
    fkcCount = fkcList.count();
    ccCount = ccList.count();

    int index = 0;

    if ( greaterThan( Uml::ot_UniqueConstraint, ot ) ) {
        index += ucCount;
    }

    if ( greaterThan( Uml::ot_ForeignKeyConstraint, ot ) ) {
        index += fkcCount;
    }

    if ( greaterThan( Uml::ot_CheckConstraint, ot ) ) {
        index += ccCount;
    }

    // we subtract 1 from the count as the new item is already in the list (m_List) and
    // hence contributes to the count we obtained
    index = index - 1;

    return index;
}

int ConstraintListPage::relativeIndexOf(QListWidgetItem* item)
{
    int actualIndex = ClassifierListPage::relativeIndexOf( item );

    int ucCount = m_pClassifier->getFilteredList( Uml::ot_UniqueConstraint ).count();
    int fkcCount = m_pClassifier->getFilteredList( Uml::ot_ForeignKeyConstraint ).count();
    //int ccCount = m_pClassifier->getFilteredList( Uml::ot_CheckConstraint ).count();

    //if ( m_itemType == Uml::ot_EntityConstraint )
    //    return actualIndex;

    int newIndex = actualIndex;

    if ( !greaterThan( m_itemType, Uml::ot_UniqueConstraint ) ) {
        newIndex -= ucCount;
    }

    if ( !greaterThan( m_itemType, Uml::ot_ForeignKeyConstraint ) ) {
        newIndex -= fkcCount;
    }

    return newIndex;
}

bool ConstraintListPage::greaterThan(Uml::Object_Type ct1,Uml::Object_Type ct2)
{
    // define ordering
    switch( ct1 ) {
       case Uml::ot_EntityConstraint:
       case Uml::ot_UniqueConstraint:
           // Unique Constraint greater than all others
            return true;
            break;
       case Uml:: ot_ForeignKeyConstraint:
           if ( ct2 != Uml::ot_UniqueConstraint )
             return true;
           else
               return false;
           break;
       case Uml::ot_CheckConstraint:
           if ( ct2 != Uml::ot_CheckConstraint )
               return false;
           else
               return true;
           break;
       default:
           return false;
    }
}

#include "constraintlistpage.moc"
