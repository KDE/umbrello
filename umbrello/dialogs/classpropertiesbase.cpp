/****************************************************************************
** Form implementation generated from reading ui file 'classpropertiesbase.ui'
**
** Created: Mon Mar 10 23:43:50 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "classpropertiesbase.h"

#include <qvariant.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtextedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a ClassPropertiesBase as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
ClassPropertiesBase::ClassPropertiesBase( QWidget* parent, const char* name, WFlags fl )
    : DialogPage( parent, name, fl )
{
    if ( !name )
	setName( "ClassPropertiesBase" );
    ClassPropertiesBaseLayout = new QVBoxLayout( this, 11, 6, "ClassPropertiesBaseLayout"); 

    layout4 = new QGridLayout( 0, 1, 1, 0, 6, "layout4"); 

    m_packageName = new QLineEdit( this, "m_packageName" );

    layout4->addWidget( m_packageName, 2, 1 );

    textLabel3 = new QLabel( this, "textLabel3" );

    layout4->addWidget( textLabel3, 2, 0 );

    m_stereotype = new QLineEdit( this, "m_stereotype" );

    layout4->addWidget( m_stereotype, 1, 1 );

    textLabel2 = new QLabel( this, "textLabel2" );

    layout4->addWidget( textLabel2, 1, 0 );

    textLabel1 = new QLabel( this, "textLabel1" );

    layout4->addWidget( textLabel1, 0, 0 );

    m_className = new QLineEdit( this, "m_className" );

    layout4->addWidget( m_className, 0, 1 );
    ClassPropertiesBaseLayout->addLayout( layout4 );

    layout6 = new QHBoxLayout( 0, 0, 6, "layout6"); 

    m_abstract = new QCheckBox( this, "m_abstract" );
    layout6->addWidget( m_abstract );
    QSpacerItem* spacer = new QSpacerItem( 231, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout6->addItem( spacer );
    ClassPropertiesBaseLayout->addLayout( layout6 );

    groupBox3 = new QGroupBox( this, "groupBox3" );
    groupBox3->setColumnLayout(0, Qt::Vertical );
    groupBox3->layout()->setSpacing( 6 );
    groupBox3->layout()->setMargin( 11 );
    groupBox3Layout = new QHBoxLayout( groupBox3->layout() );
    groupBox3Layout->setAlignment( Qt::AlignTop );

    m_public = new QRadioButton( groupBox3, "m_public" );
    groupBox3Layout->addWidget( m_public );

    m_protected = new QRadioButton( groupBox3, "m_protected" );
    groupBox3Layout->addWidget( m_protected );

    m_private = new QRadioButton( groupBox3, "m_private" );
    groupBox3Layout->addWidget( m_private );
    ClassPropertiesBaseLayout->addWidget( groupBox3 );

    groupBox2 = new QGroupBox( this, "groupBox2" );
    groupBox2->setColumnLayout(0, Qt::Vertical );
    groupBox2->layout()->setSpacing( 6 );
    groupBox2->layout()->setMargin( 11 );
    groupBox2Layout = new QVBoxLayout( groupBox2->layout() );
    groupBox2Layout->setAlignment( Qt::AlignTop );

    m_documentation = new QTextEdit( groupBox2, "m_documentation" );
    groupBox2Layout->addWidget( m_documentation );
    ClassPropertiesBaseLayout->addWidget( groupBox2 );
    languageChange();
    resize( QSize(338, 351).expandedTo(minimumSizeHint()) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
ClassPropertiesBase::~ClassPropertiesBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ClassPropertiesBase::languageChange()
{
    setCaption( tr( "Class properties" ) );
    textLabel3->setText( tr( "Package name:" ) );
    textLabel2->setText( tr( "Stereotype:" ) );
    textLabel1->setText( tr( "Class name:" ) );
    m_abstract->setText( tr( "Abstract class" ) );
    groupBox3->setTitle( tr( "Visibility" ) );
    m_public->setText( tr( "Public" ) );
    m_protected->setText( tr( "Protected" ) );
    m_private->setText( tr( "Private" ) );
    groupBox2->setTitle( tr( "Documentation" ) );
}

