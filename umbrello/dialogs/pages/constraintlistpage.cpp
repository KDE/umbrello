/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
#include "uml.h"  // only needed for log{Warn,Error}
#include "uniqueconstraint.h"

#if QT_VERSION < 0x050000
#include <kaction.h>
#include <kdialogbuttonbox.h>
#endif
#include <KLocalizedString>
#if QT_VERSION < 0x050000
#include <QMenu>
#include <kpushbutton.h>
#endif

#include <QApplication>
#include <QHBoxLayout>
#if QT_VERSION >= 0x050000
#include <QMenu>
#include <QPushButton>
#endif
#include <QVBoxLayout>

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
  : ClassifierListPage(parent, classifier, doc, type)
{
    setupActions();

#if QT_VERSION >= 0x050000
    buttonMenu = new QMenu(this);
#else
    buttonMenu = new QMenu(this);
#endif
    // add a button menu
    m_pNewClassifierListItemButton->setMenu(buttonMenu);
    buttonMenu->addAction(newPrimaryKeyConstraintAction);
    buttonMenu->addAction(newUniqueConstraintAction);
    buttonMenu->addAction(newForeignKeyConstraintAction);
    buttonMenu->addAction(newCheckConstraintAction);

    // because we order the list items. first the Unique Constraints and then the ForeignKey Constraints
    hideArrowButtons(true);
}

/**
 *  Standard destructor.
 */
ConstraintListPage::~ConstraintListPage()
{
}

void ConstraintListPage::setupActions()
{
#if QT_VERSION >= 0x050000
    newUniqueConstraintAction = new QAction(i18n("Unique Constraint..."), this);
#else
    newUniqueConstraintAction = new KAction(i18n("Unique Constraint..."), this);
#endif
    connect(newUniqueConstraintAction, SIGNAL(triggered(bool)),
             this, SLOT(slotNewUniqueConstraint()));

#if QT_VERSION >= 0x050000
    newPrimaryKeyConstraintAction = new QAction(i18n("Primary Key Constraint..."), this);
#else
    newPrimaryKeyConstraintAction = new KAction(i18n("Primary Key Constraint..."), this);
#endif
    connect(newPrimaryKeyConstraintAction, SIGNAL(triggered(bool)),
             this, SLOT(slotNewPrimaryKeyConstraint()));

#if QT_VERSION >= 0x050000
    newForeignKeyConstraintAction = new QAction(i18n("Foreign Key Constraint..."), this);
#else
    newForeignKeyConstraintAction = new KAction(i18n("Foreign Key Constraint..."), this);
#endif
    connect(newForeignKeyConstraintAction, SIGNAL(triggered(bool)),
             this, SLOT(slotNewForeignKeyConstraint()));

#if QT_VERSION >= 0x050000
    newCheckConstraintAction = new QAction(i18n("Check Constraint..."), this);
#else
    newCheckConstraintAction = new KAction(i18n("Check Constraint..."), this);
#endif
    connect(newCheckConstraintAction, SIGNAL(triggered(bool)),
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
    UMLEntity* ent = m_pClassifier->asUMLEntity();

    if (ent == 0) {
        logError0("ConstraintListPage::slotNewPrimaryKeyConstraint: Could not set Primary Key. "
                  "Entity Value is Null");
        return;
    }

    if (m_pLastObjectCreated != 0) {
        m_bSigWaiting = true;
        ent->setAsPrimaryKey(m_pLastObjectCreated->asUMLUniqueConstraint());
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
//  *  param ot The Object Type to be added
//  *  return The index
//  */
// int ConstraintListPage::calculateNewIndex(Uml::ObjectType ot)
// {
//     // we want to show all Unique Constraints first, followed by ForeignKey Constraints
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
//     if (greaterThan(UMLObject::ot_UniqueConstraint, ot)) {
//         index += ucCount;
//     }
// 
//     if (greaterThan(UMLObject::ot_ForeignKeyConstraint, ot)) {
//         index += fkcCount;
//     }
// 
//     if (greaterThan(UMLObject::ot_CheckConstraint, ot)) {
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
//     int actualIndex = ClassifierListPage::relativeIndexOf(item);
// 
//     int ucCount = m_pClassifier->getFilteredList(UMLObject::ot_UniqueConstraint).count();
//     int fkcCount = m_pClassifier->getFilteredList(UMLObject::ot_ForeignKeyConstraint).count();
//     //int ccCount = m_pClassifier->getFilteredList(UMLObject::ot_CheckConstraint).count();
// 
//     //if (m_itemType == UMLObject::ot_EntityConstraint)
//     //    return actualIndex;
// 
//     int newIndex = actualIndex;
// 
//     if (!greaterThan(m_itemType, UMLObject::ot_UniqueConstraint)) {
//         newIndex -= ucCount;
//     }
// 
//     if (!greaterThan(m_itemType, UMLObject::ot_ForeignKeyConstraint)) {
//         newIndex -= fkcCount;
//     }
// 
//     return newIndex;
// }

/**
 * Will return true if ct1 has a higher (top)  place in the list than ct2
 *
 * @param ct1 Constraint Type 1
 * @param ct2 Constraint Type 2
 * @return true if ct1 is to be shown above ct2 else false
 */
bool ConstraintListPage::greaterThan(UMLObject::ObjectType ct1, UMLObject::ObjectType ct2)
{
    // define ordering
    switch(ct1) {
       case UMLObject::ot_EntityConstraint:
       case UMLObject::ot_UniqueConstraint:
           // Unique Constraint greater than all others
            return true;
            break;
       case UMLObject::ot_ForeignKeyConstraint:
           if (ct2 != UMLObject::ot_UniqueConstraint)
               return true;
           else
               return false;
           break;
       case UMLObject::ot_CheckConstraint:
           if (ct2 != UMLObject::ot_CheckConstraint)
               return false;
           else
               return true;
           break;
       default:
           return false;
    }
}

/**
 * Get constraint list items for all types (unique, foreign-key
 * and check-constraints)
 */
UMLClassifierListItemList ConstraintListPage::getItemList()
{
    return m_pClassifier->getFilteredList(UMLObject::ot_EntityConstraint);
}

