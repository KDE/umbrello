/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2003-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#include "umlforeignkeyconstraintdialog.h"
#include "../attribute.h"
#include "../classifier.h"
#include "../classifierlistitem.h"
#include "../dialog_utils.h"
#include "../entityattribute.h"
#include "../enumliteral.h"
#include "../enum.h"
#include "../entity.h"
#include "../foreignkeyconstraint.h"
#include "../object_factory.h"
#include "../operation.h"
#include "../template.h"
#include "../uml.h"
#include "../umldoc.h"
#include "../umlentitylist.h"
#include "../uniqueconstraint.h"

#include <kdebug.h>
#include <kdialogbuttonbox.h>
#include <klocale.h>
#include <kvbox.h>
#include <kiconloader.h>
#include <kicon.h>
#include <kmessagebox.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>


UMLForeignKeyConstraintDialog::UMLForeignKeyConstraintDialog(QWidget* parent, UMLForeignKeyConstraint* pForeignKeyConstraint) : KPageDialog(parent) {

    setCaption(i18n("Foreign Key Setup") );
    setButtons( Help | Ok | Apply | Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );
    setFaceType( KPageDialog::List );

    m_pForeignKeyConstraint = pForeignKeyConstraint;
    m_doc = UMLApp::app()->getDocument();

    setupGeneralPage();
    setupColumnPage();

    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));

}

UMLForeignKeyConstraintDialog::~UMLForeignKeyConstraintDialog() {

}


void UMLForeignKeyConstraintDialog::slotAddPair() {
    // get the index of the selected local column and referenced column
    int indexL = m_ColumnWidgets.localColumnCB->currentIndex();
    int indexR = m_ColumnWidgets.referencedColumnCB->currentIndex();

    if ( indexL==-1 || indexR==-1 )
        return;

    // local entity attribute
    UMLEntityAttribute* localColumn = m_pLocalAttributeList.at( indexL );
    // referenced entity attribute
    UMLEntityAttribute* referencedColumn = m_pReferencedAttributeList.at( indexR );

    // remove from combo boxes
    m_ColumnWidgets.localColumnCB->removeItem( indexL );
    m_ColumnWidgets.referencedColumnCB->removeItem( indexR );

    // remove from local cache
    m_pLocalAttributeList.remove( indexL );
    m_pReferencedAttributeList.remove( indexR );

    // add to local cache of mapping
    QPair<UMLEntityAttribute*, UMLEntityAttribute*> pair =
        qMakePair( localColumn,referencedColumn );
    m_pAttributeMapList.append( pair );
    // update mapping view

    QTreeWidgetItem* mapping = new QTreeWidgetItem( m_ColumnWidgets.mappingTW );
    mapping->setText(0, localColumn->toString(Uml::st_SigNoVis) );
    mapping->setText(1, referencedColumn->toString(Uml::st_SigNoVis));

    m_ColumnWidgets.mappingTW->addTopLevelItem( mapping );

}

void UMLForeignKeyConstraintDialog::slotDeletePair(){

    // get the index of the selected pair in the view
    QTreeWidgetItem* twi = m_ColumnWidgets.mappingTW->currentItem();
    int indexP = m_ColumnWidgets.mappingTW->indexOfTopLevelItem( twi );

    if ( indexP==-1 )
        return;
    //find pair in local cache
    QPair<UMLEntityAttribute*, UMLEntityAttribute*> pair = m_pAttributeMapList.at( indexP );

    // remove them from the view and the list

    m_ColumnWidgets.mappingTW->takeTopLevelItem( indexP );
    m_pAttributeMapList.removeAt( indexP );

    // add the attributes to the local caches
    m_pLocalAttributeList.append( pair.first );
    m_pReferencedAttributeList.append( pair.second );

    // add them to the view ( combo boxes )
    kDebug()<< k_funcinfo << ( pair.first ) << ( pair.second ) << endl;
    m_ColumnWidgets.localColumnCB->addItem( ( pair.first )->toString(Uml::st_SigNoVis) );
    m_ColumnWidgets.referencedColumnCB->addItem( ( pair.second )->toString(Uml::st_SigNoVis) );

    foreach(pair, m_pAttributeMapList ) {
        kDebug()<<( pair.first )->getName()<<" "<< ( pair.first )->getBaseType()<<" "
                <<( pair.second )->getName()<<" "<< ( pair.second )->getBaseType()<<endl;
    }
}

bool UMLForeignKeyConstraintDialog::apply(){

    // set the Referenced Entity
    QString entityName = m_GeneralWidgets.referencedEntityCB->currentText();
    UMLObject* uo = m_doc->findUMLObject( entityName , Uml::ot_Entity );

    UMLEntity* ue = static_cast<UMLEntity*>( uo );

    if ( ue==NULL ) {
        kDebug()<< k_funcinfo << " Couldn't find UML Entity with name "<< entityName <<endl;
        return false;
    }

    m_pForeignKeyConstraint->setReferencedEntity( ue );

    // set all the update and delete actions
    UMLForeignKeyConstraint::UpdateDeleteAction updateAction,deleteAction;
    updateAction = ( UMLForeignKeyConstraint::UpdateDeleteAction ) m_GeneralWidgets.updateActionCB->currentIndex();
    deleteAction = ( UMLForeignKeyConstraint::UpdateDeleteAction )  m_GeneralWidgets.deleteActionCB->currentIndex();
    m_pForeignKeyConstraint->setUpdateAction(updateAction);
    m_pForeignKeyConstraint->setDeleteAction(deleteAction);

    // remove all existing mappings first
    m_pForeignKeyConstraint->clearMappings();

    // add all mappings  present in local cache
    QPair<UMLEntityAttribute*, UMLEntityAttribute*> pair;
    foreach( pair, m_pAttributeMapList ) {
        if ( !m_pForeignKeyConstraint->addEntityAttributePair( pair.first, pair.second ) ) {
            return false;
        }
    }

    // set the name
    m_pForeignKeyConstraint->setName( m_GeneralWidgets.nameT->text() );

    return true;
}

void UMLForeignKeyConstraintDialog::setupGeneralPage() {
     //setup General page

    KVBox * page = new KVBox();
    pageGeneral = new KPageWidgetItem( page,i18n("General"));
    pageGeneral->setHeader( i18n("General Settings") );
    pageGeneral->setIcon( KIcon( DesktopIcon( "misc") ));
    addPage( pageGeneral );

    m_GeneralWidgets.generalGB = new Q3GroupBox( i18n( "General" ), page );

    QGridLayout* generalLayout = new QGridLayout( m_GeneralWidgets.generalGB );
    generalLayout -> setSpacing( spacingHint() );
    generalLayout -> setMargin( fontMetrics().height() );

    Dialog_Utils::makeLabeledEditField( m_GeneralWidgets.generalGB, generalLayout, 0,
                                        m_GeneralWidgets.nameL, i18n("Name"),
                                        m_GeneralWidgets.nameT);

    m_GeneralWidgets.referencedEntityL = new QLabel( i18n( "Referenced Entity" ), page );
    generalLayout->addWidget( m_GeneralWidgets.referencedEntityL, 1, 0 );

    m_GeneralWidgets.referencedEntityCB = new KComboBox( page );

    generalLayout->addWidget( m_GeneralWidgets.referencedEntityCB, 1, 1 );

    m_GeneralWidgets.actionGB = new Q3GroupBox( i18n( "Actions" ), page );

    QGridLayout* actionLayout = new QGridLayout( m_GeneralWidgets.actionGB );
    generalLayout -> setSpacing( spacingHint() );
    generalLayout -> setMargin( fontMetrics().height() );

    m_GeneralWidgets.onUpdateL = new QLabel( i18n( "On Update" ), page );
    actionLayout->addWidget( m_GeneralWidgets.onUpdateL, 0, 0 );

    m_GeneralWidgets.updateActionCB = new KComboBox( page );
    actionLayout->addWidget( m_GeneralWidgets.updateActionCB , 0, 1 );

    m_GeneralWidgets.onDeleteL = new QLabel( i18n( "On Delete" ), page );
    actionLayout->addWidget( m_GeneralWidgets.onDeleteL, 1, 0 );

    m_GeneralWidgets.deleteActionCB = new KComboBox( page );
    actionLayout->addWidget( m_GeneralWidgets.deleteActionCB , 1, 1 );


    // set the name
    m_GeneralWidgets.nameT->setText( m_pForeignKeyConstraint->getName() );

    // fill up the combo boxes

    // reference entity combo box
    UMLEntityList entList = m_doc->getEntities();

    foreach( UMLEntity* ent, entList ) {
        m_GeneralWidgets.referencedEntityCB->addItem( ent->getName() );
    }

    UMLEntity* referencedEntity = m_pForeignKeyConstraint->getReferencedEntity();

    int index;
    if ( referencedEntity != NULL ) {
       index = m_GeneralWidgets.referencedEntityCB->findText( referencedEntity->getName() );
       if ( index!=-1 )
          m_GeneralWidgets.referencedEntityCB->setCurrentIndex( index );
    }
    m_pReferencedEntityIndex = m_GeneralWidgets.referencedEntityCB->currentIndex();

    // action combo boxes

    // do not change order. It is according to enum specification in foreignkeyconstraint.h
    QStringList actions;
    actions<<i18n( "No Action" )<<i18n( "Restrict" )<<i18n( "Cascade" )<<i18n( "Set Null" )
           <<i18n( "Set Default" );

    foreach( QString act, actions ) {
        m_GeneralWidgets.updateActionCB->addItem( act );
        m_GeneralWidgets.deleteActionCB->addItem( act );
    }

    m_GeneralWidgets.updateActionCB->setCurrentIndex( m_pForeignKeyConstraint->getUpdateAction() );
    m_GeneralWidgets.deleteActionCB->setCurrentIndex( m_pForeignKeyConstraint->getDeleteAction() );

    connect( m_GeneralWidgets.referencedEntityCB, SIGNAL( activated( int ) ), this, SLOT( slotReferencedEntityChanged( int ) ) );

}

void UMLForeignKeyConstraintDialog::setupColumnPage() {

    //setup Columns page
    KVBox * page = new KVBox();
    pageColumn = new KPageWidgetItem( page,i18n("Columns"));
    pageColumn->setHeader( i18n("Columns") );
    pageColumn->setIcon( KIcon( DesktopIcon( "misc") ));
    addPage( pageColumn );


    m_ColumnWidgets.mappingTW = new QTreeWidget( page );

    QStringList headers;
    headers<<i18n( "Local" )<<i18n( "Referenced" );
    m_ColumnWidgets.mappingTW->setHeaderLabels( headers );

    QWidget* columns = new QWidget( page );
    QGridLayout* columnsLayout = new QGridLayout( columns );

    m_ColumnWidgets.localColumnL = new QLabel( i18n( "Local Column" ), page );
    columnsLayout->addWidget( m_ColumnWidgets.localColumnL, 0, 0 );

    m_ColumnWidgets.localColumnCB = new KComboBox( page );
    columnsLayout->addWidget( m_ColumnWidgets.localColumnCB, 0, 1 );

    m_ColumnWidgets.referencedColumnL = new QLabel(i18n( "Referenecd Column" ), page );
    columnsLayout->addWidget( m_ColumnWidgets.referencedColumnL, 1, 0 );

    m_ColumnWidgets.referencedColumnCB = new KComboBox( page );
    columnsLayout->addWidget( m_ColumnWidgets.referencedColumnCB, 1, 1 );

    KDialogButtonBox* buttonBox = new KDialogButtonBox( page );
    buttonBox->addButton( i18n( "&Add" ), KDialogButtonBox::ActionRole, this,
                          SLOT(slotAddPair()) );
    buttonBox->addButton( i18n( "&Delete" ), KDialogButtonBox::ActionRole, this,
                          SLOT(slotDeletePair()) );

    columnsLayout->addWidget( buttonBox , 2, 1 );

    // fill the column boxes and their local cache.
    refillLocalAttributeCB();
    refillReferencedAttributeCB();

    QMap<UMLEntityAttribute*, UMLEntityAttribute*>::iterator i;
    QMap<UMLEntityAttribute*,UMLEntityAttribute*>  map = m_pForeignKeyConstraint->getEntityAttributePairs();
    for (i = map.begin(); i!= map.end() ; ++i) {

        UMLEntityAttribute* localColumn, *referencedColumn;

        localColumn = const_cast<UMLEntityAttribute*>( i.key() );
        referencedColumn = const_cast<UMLEntityAttribute*>( i.value() );

        // remove these columns from local cache
        int indexL = m_pLocalAttributeList.findRef( localColumn );
        int indexR = m_pReferencedAttributeList.findRef( referencedColumn );

        m_pLocalAttributeList.remove( indexL );
        m_pReferencedAttributeList.remove( indexR );

        // remove them from combo boxes
        // the conditions may never be violated . Just for safety though
        if ( indexL>=0 && indexL < (m_ColumnWidgets.localColumnCB )->count() )
            m_ColumnWidgets.localColumnCB->removeItem( indexL );
        if ( indexR>=0 && indexR < (m_ColumnWidgets.referencedColumnCB )->count() )
            m_ColumnWidgets.referencedColumnCB->removeItem( indexR );


        // add to local cache
        m_pAttributeMapList.append( qMakePair( localColumn, referencedColumn ) );

        // add to view
        QTreeWidgetItem* mapping = new QTreeWidgetItem( m_ColumnWidgets.mappingTW );
        mapping->setText(0, localColumn->toString(Uml::st_SigNoVis) );
        mapping->setText(1, referencedColumn->toString(Uml::st_SigNoVis));

        m_ColumnWidgets.mappingTW->insertTopLevelItem( 0, mapping );

    }



}

void UMLForeignKeyConstraintDialog::slotApply(){
    apply();
}

void UMLForeignKeyConstraintDialog::slotOk(){
    if ( apply() ) {
        accept();
    }

}


void UMLForeignKeyConstraintDialog::slotReferencedEntityChanged(int index){

    if ( index == m_pReferencedEntityIndex )
        return;

    if ( !m_pAttributeMapList.empty() ) {
        int result = KMessageBox::questionYesNo( this, i18n( "You are attempting to change the Referenced Entity of this ForeignKey Constraint. Any unapplied changes to the mappings between local and referenced entities will be lost. Are you sure you want to continue ?" ) );

        if ( result!=KMessageBox::Yes ) {
            // revert back to old index
            m_GeneralWidgets.referencedEntityCB->setCurrentIndex( m_pReferencedEntityIndex );
            return;
        }

    }

    // set the referenced entity index to current index
    m_pReferencedEntityIndex = index;
    m_ColumnWidgets.mappingTW->clear();
    refillReferencedAttributeCB();
    refillLocalAttributeCB();
}



void UMLForeignKeyConstraintDialog::refillReferencedAttributeCB(){
    m_pReferencedAttributeList.clear();
    m_ColumnWidgets.referencedColumnCB->clear();
    // fill the combo boxes


    UMLObject* uo = m_doc->findUMLObject( m_GeneralWidgets.referencedEntityCB->currentText(),
                                       Uml::ot_Entity );

    UMLEntity* ue = static_cast<UMLEntity*>( uo );

    if ( ue ) {
       UMLClassifierListItemList ual = ue->getFilteredList(Uml::ot_EntityAttribute);
       foreach( UMLClassifierListItem* att, ual ) {
           m_pReferencedAttributeList.append( static_cast<UMLEntityAttribute*>( att ) );
           m_ColumnWidgets.referencedColumnCB->addItem( att->toString( Uml::st_SigNoVis ) );
       }
    }
}

void UMLForeignKeyConstraintDialog::refillLocalAttributeCB(){
    m_pLocalAttributeList.clear();
    m_ColumnWidgets.localColumnCB->clear();
    // fill the combo boxes
    UMLEntity* ue = static_cast<UMLEntity*>( m_pForeignKeyConstraint->parent() );

    if ( ue ) {
       UMLClassifierListItemList ual = ue->getFilteredList(Uml::ot_EntityAttribute);
       foreach( UMLClassifierListItem* att, ual ) {
           m_pLocalAttributeList.append( static_cast<UMLEntityAttribute*>( att ) );
           m_ColumnWidgets.localColumnCB->addItem( att->toString( Uml::st_SigNoVis ) );
       }
    }

}

#include "umlforeignkeyconstraintdialog.moc"

