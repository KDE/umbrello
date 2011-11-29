/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2003-2011                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/
#include "umluniqueconstraintdialog.h"

#include "attribute.h"
#include "classifierlistitem.h"
#include "classifier.h"
#include "debug_utils.h"
#include "entity.h"
#include "entityattribute.h"
#include "enumliteral.h"
#include "enum.h"
#include "object_factory.h"
#include "operation.h"
#include "template.h"
#include "uml.h"
#include "uniqueconstraint.h"
#include "umldoc.h"

#include <kcombobox.h>
#include <kdialogbuttonbox.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>

UMLUniqueConstraintDialog::UMLUniqueConstraintDialog(QWidget* parent, UMLUniqueConstraint* pUniqueConstraint) : KDialog(parent)
{
    setCaption( i18n("Unique Constraint Properties") );
    setButtons( Help | Ok | Apply | Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );

    m_pUniqueConstraint = pUniqueConstraint;
    m_doc = UMLApp::app()->document();

    setupDialog();
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

UMLUniqueConstraintDialog::~UMLUniqueConstraintDialog()
{
}

/**
 * Sets up the dialog.
 */
void UMLUniqueConstraintDialog::setupDialog()
{
    QFrame *frame = new QFrame( this );
    setMainWidget( frame );
    int margin = fontMetrics().height();

    // what do we need,
    // we need one label,  line edir
    // a list item box
    // a combo box, two push buttons, yeah that's it.
    // start
    //main layout contains the name fields, the column group box
    QVBoxLayout* mainLayout = new QVBoxLayout( frame );
    mainLayout->setSpacing(10);

    // layout to hold the name label and line edit
    QHBoxLayout* nameLayout = new QHBoxLayout();
    mainLayout->addItem( nameLayout );

    // name label
    m_pNameL = new QLabel( i18nc("name label", "Name"), this );
    nameLayout->addWidget( m_pNameL );
    // name lineEdit
    m_pNameLE = new KLineEdit( this );
    nameLayout->addWidget( m_pNameLE );

    // group box to hold the column details
    // top group box, contains a vertical layout with list box above and buttons below
    m_pAttributeListGB = new QGroupBox(i18n( "Attribute Details" ), frame );
    mainLayout->addWidget( m_pAttributeListGB );

    QVBoxLayout* listVBoxLayout = new QVBoxLayout( m_pAttributeListGB );
    listVBoxLayout->setMargin(margin);
    listVBoxLayout->setSpacing ( 10 );

    m_pAttributeListLW = new QListWidget(m_pAttributeListGB);
    listVBoxLayout->addWidget( m_pAttributeListLW );

    // Horizontal Layout to hold  attributes CB, the add, and remove buttons
    QHBoxLayout* comboButtonHBoxLayout = new QHBoxLayout();
    listVBoxLayout->addItem( comboButtonHBoxLayout );

    // the Combo Box containing the attributes
    m_pAttributeCB = new KComboBox( true, m_pAttributeListGB );
    m_pAttributeCB->setEditable( false );
    comboButtonHBoxLayout->addWidget( m_pAttributeCB );

    //the action buttons
    KDialogButtonBox* buttonBox = new KDialogButtonBox(m_pAttributeListGB);
    m_pAddPB = buttonBox->addButton( i18n( "&Add" ), KDialogButtonBox::ActionRole, this,
                          SLOT(slotAddAttribute()) );
    m_pRemovePB = buttonBox->addButton( i18n( "&Delete" ), KDialogButtonBox::ActionRole, this,
                          SLOT(slotDeleteAttribute()) );

    comboButtonHBoxLayout->addWidget( buttonBox );

    // We first insert all attributes to the combo box
    UMLEntity* ue = static_cast<UMLEntity*>( m_pUniqueConstraint->parent() );
    uDebug() << ue;
    if ( ue ) {
       UMLClassifierListItemList ual = ue->getFilteredList(UMLObject::ot_EntityAttribute);
       foreach( UMLClassifierListItem* att, ual ) {
           m_pEntityAttributeList.append( static_cast<UMLEntityAttribute*>( att ) );
           m_pAttributeCB->addItem( att->toString( Uml::SignatureType::SigNoVis ) );
       }
    }

    // Then we add the attributes in the constraint to the list box
    UMLEntityAttributeList eal = m_pUniqueConstraint->getEntityAttributeList();
    foreach( UMLEntityAttribute* att, eal ) {

        // add to local cache
        m_pConstraintAttributeList.append( att );
        // add to list box
        m_pAttributeListLW->addItem( att->toString( Uml::SignatureType::SigNoVis ) );

        int index = m_pEntityAttributeList.indexOf( att );
        m_pEntityAttributeList.removeAt( index );
        m_pAttributeCB->removeItem( index );
    }

    // set text of label
    m_pNameLE->setText( m_pUniqueConstraint->name() );

    // select firstItem
    if ( m_pAttributeListLW->count() != 0 ) {
        m_pAttributeListLW->setCurrentRow( 0 );
    }

    slotResetWidgetState();

    connect( m_pAttributeListLW, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotResetWidgetState()));
}

/**
 * Adds attribute to the list.
 */
void UMLUniqueConstraintDialog::slotAddAttribute()
{
    int index = m_pAttributeCB->currentIndex();

    if ( index == -1 ) {
        return;
    }

    // get reference
    UMLEntityAttribute* entAtt = m_pEntityAttributeList.at(index);

    //find and remove from list
    index = m_pEntityAttributeList.indexOf( entAtt );
    m_pEntityAttributeList.removeAt( index );

    // remove from combo box
    m_pAttributeCB->removeItem( index );

    // add to local cache
    m_pConstraintAttributeList.append( entAtt );

    // add to list box
    int count = m_pAttributeListLW->count();
    m_pAttributeListLW->insertItem( count, entAtt->toString( Uml::SignatureType::SigNoVis ) );

    slotResetWidgetState();
}

/**
 * Deletes an attribute from the list.
 */
void UMLUniqueConstraintDialog::slotDeleteAttribute()
{
    int index = m_pAttributeListLW->currentRow();

    if ( index == -1 ) {
        return;
    }

    // get reference
    UMLEntityAttribute* entAtt = m_pConstraintAttributeList.at(index );

    //remove from constraint
    m_pConstraintAttributeList.removeAt( index );
    // remove from list box
    m_pAttributeListLW->takeItem( index );

    // add to list
    m_pEntityAttributeList.append( entAtt );
    // add to combo box
    int count = m_pAttributeCB->count();
    m_pAttributeCB->insertItem( count, entAtt->toString(Uml::SignatureType::SigNoVis ) );

    slotResetWidgetState();
}

/**
 * Used when the Apply Button is clicked.
 */
void UMLUniqueConstraintDialog::slotApply()
{
    apply();
}

/**
 * Used when the OK button is clicked.  Calls apply().
 */
void UMLUniqueConstraintDialog::slotOk()
{
    if ( apply() ) {
        accept();
    }
}

/**
 * Apply changes.
 */
bool UMLUniqueConstraintDialog::apply()
{
    QString name = m_pNameLE->text();
    if ( name.length() == 0 ) {
        KMessageBox::error(this, i18n("You have entered an invalid constraint name."),
                           i18n("Constraint Name Invalid"), 0);
        m_pNameLE->setText( m_pUniqueConstraint->name() );
        return false;
    }

    // clear the old list
    m_pUniqueConstraint->clearAttributeList();

    // fill it with contents of local cache
    foreach(UMLEntityAttribute* att, m_pConstraintAttributeList ) {
        m_pUniqueConstraint->addEntityAttribute( att );
    }

    // set name
    m_pUniqueConstraint->setName( name );

    return true;
}

/**
 * Enable or Disable the widgets.
 */
void UMLUniqueConstraintDialog::slotResetWidgetState()
{
    m_pAttributeCB->setEnabled( true );
    m_pAddPB->setEnabled( true );
    m_pRemovePB->setEnabled( true );

    // get index of selected Attribute in List Box
    int index = m_pAttributeListLW->currentRow();
    // if index is not invalid ( -1 ), then activate the Remove Button
    if ( index == -1 ) {
        m_pRemovePB->setEnabled( false );
    }

    // check for number of items in ComboBox
    int count = m_pAttributeCB->count();
    // if count is 0 disable Combo Box and Add Button
    if ( count == 0 ) {
        m_pAttributeCB->setEnabled( false );
        m_pAddPB->setEnabled( false );
    }
}

#include "umluniqueconstraintdialog.moc"

