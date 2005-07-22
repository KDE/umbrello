#include <kdialog.h>
#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './rubycodegenerationformbase.ui'
**
** Created: Fri Jul 22 06:36:57 2005
**      by: The User Interface Compiler ()
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "rubycodegenerationformbase.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a RubyCodeGenerationFormBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
RubyCodeGenerationFormBase::RubyCodeGenerationFormBase( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "Form1" );
    Form1Layout = new QGridLayout( this, 1, 1, 11, 6, "Form1Layout"); 

    layout6 = new QGridLayout( 0, 1, 1, 0, 6, "layout6"); 

    groupBox2 = new QGroupBox( this, "groupBox2" );
    groupBox2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)4, 0, 0, groupBox2->sizePolicy().hasHeightForWidth() ) );
    groupBox2->setColumnLayout(0, Qt::Vertical );
    groupBox2->layout()->setSpacing( 6 );
    groupBox2->layout()->setMargin( 11 );
    groupBox2Layout = new QGridLayout( groupBox2->layout() );
    groupBox2Layout->setAlignment( Qt::AlignTop );

    layout2 = new QGridLayout( 0, 1, 1, 0, 6, "layout2"); 

    m_generateConstructors = new QCheckBox( groupBox2, "m_generateConstructors" );
    m_generateConstructors->setChecked( FALSE );

    layout2->addWidget( m_generateConstructors, 0, 0 );

    m_generateAssocAccessors = new QCheckBox( groupBox2, "m_generateAssocAccessors" );
    m_generateAssocAccessors->setChecked( TRUE );

    layout2->addWidget( m_generateAssocAccessors, 2, 0 );

    m_generateAttribAccessors = new QCheckBox( groupBox2, "m_generateAttribAccessors" );
    m_generateAttribAccessors->setChecked( TRUE );

    layout2->addWidget( m_generateAttribAccessors, 1, 0 );

    groupBox2Layout->addLayout( layout2, 0, 0 );

    layout4 = new QGridLayout( 0, 1, 1, 0, 6, "layout4"); 

    m_accessorScopeCB = new QComboBox( FALSE, groupBox2, "m_accessorScopeCB" );

    layout4->addWidget( m_accessorScopeCB, 0, 1 );

    textLabel1_3 = new QLabel( groupBox2, "textLabel1_3" );

    layout4->addWidget( textLabel1_3, 0, 0 );

    groupBox2Layout->addLayout( layout4, 1, 0 );

    layout4_2 = new QGridLayout( 0, 1, 1, 0, 6, "layout4_2"); 

    textLabel1_3_2 = new QLabel( groupBox2, "textLabel1_3_2" );

    layout4_2->addWidget( textLabel1_3_2, 0, 0 );

    m_assocFieldScopeCB = new QComboBox( FALSE, groupBox2, "m_assocFieldScopeCB" );

    layout4_2->addWidget( m_assocFieldScopeCB, 0, 1 );

    groupBox2Layout->addLayout( layout4_2, 2, 0 );

    layout6->addWidget( groupBox2, 2, 0 );

    groupBox1 = new QGroupBox( this, "groupBox1" );
    groupBox1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)4, 0, 0, groupBox1->sizePolicy().hasHeightForWidth() ) );
    groupBox1->setColumnLayout(0, Qt::Vertical );
    groupBox1->layout()->setSpacing( 6 );
    groupBox1->layout()->setMargin( 11 );
    groupBox1Layout = new QHBoxLayout( groupBox1->layout() );
    groupBox1Layout->setAlignment( Qt::AlignTop );

    textLabel1 = new QLabel( groupBox1, "textLabel1" );
    groupBox1Layout->addWidget( textLabel1 );

    m_SelectCommentStyle = new QComboBox( FALSE, groupBox1, "m_SelectCommentStyle" );
    groupBox1Layout->addWidget( m_SelectCommentStyle );

    layout6->addWidget( groupBox1, 1, 0 );

    textLabel1_2 = new QLabel( this, "textLabel1_2" );
    textLabel1_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)4, 0, 0, textLabel1_2->sizePolicy().hasHeightForWidth() ) );

    layout6->addWidget( textLabel1_2, 0, 0 );

    Form1Layout->addLayout( layout6, 0, 0 );
    languageChange();
    resize( QSize(462, 376).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

/*
 *  Destroys the object and frees any allocated resources
 */
RubyCodeGenerationFormBase::~RubyCodeGenerationFormBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void RubyCodeGenerationFormBase::languageChange()
{
    setCaption( tr2i18n( "Form1" ) );
    groupBox2->setTitle( tr2i18n( "Auto-Generate Methods" ) );
    m_generateConstructors->setText( tr2i18n( "Empty constructor methods" ) );
    m_generateAssocAccessors->setText( tr2i18n( "Association accessor methods" ) );
    m_generateAttribAccessors->setText( tr2i18n( "Attribute accessor methods" ) );
    m_accessorScopeCB->clear();
    m_accessorScopeCB->insertItem( tr2i18n( "Public" ) );
    m_accessorScopeCB->insertItem( tr2i18n( "Private" ) );
    m_accessorScopeCB->insertItem( tr2i18n( "Protected" ) );
    m_accessorScopeCB->insertItem( tr2i18n( "From Parent Object" ) );
    m_accessorScopeCB->setCurrentItem( 3 );
    textLabel1_3->setText( tr2i18n( "Default attribute accessor scope:" ) );
    textLabel1_3_2->setText( tr2i18n( "Default association field scope:" ) );
    m_assocFieldScopeCB->clear();
    m_assocFieldScopeCB->insertItem( tr2i18n( "Public" ) );
    m_assocFieldScopeCB->insertItem( tr2i18n( "Private" ) );
    m_assocFieldScopeCB->insertItem( tr2i18n( "Protected" ) );
    m_assocFieldScopeCB->insertItem( tr2i18n( "From Parent Role" ) );
    m_assocFieldScopeCB->setCurrentItem( 3 );
    groupBox1->setTitle( tr2i18n( "Documentation" ) );
    textLabel1->setText( tr2i18n( "Style:" ) );
    m_SelectCommentStyle->clear();
    m_SelectCommentStyle->insertItem( tr2i18n( "Begin-End (=begin  =end)" ) );
    m_SelectCommentStyle->insertItem( tr2i18n( "Hash (#)" ) );
    textLabel1_2->setText( tr2i18n( "<p align=\"center\">Ruby Code Generation</p>" ) );
}

#include "rubycodegenerationformbase.moc"
