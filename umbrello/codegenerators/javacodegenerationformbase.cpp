#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './javacodegenerationformbase.ui'
**
** Created: Sun Aug 31 16:23:57 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "javacodegenerationformbase.h"

#include <qvariant.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a JavaCodeGenerationFormBase as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
JavaCodeGenerationFormBase::JavaCodeGenerationFormBase( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "Form1" );

    QWidget* privateLayoutWidget = new QWidget( this, "layout2" );
    privateLayoutWidget->setGeometry( QRect( 10, 20, 400, 300 ) );
    layout2 = new QVBoxLayout( privateLayoutWidget, 11, 6, "layout2"); 

    textLabel1_2 = new QLabel( privateLayoutWidget, "textLabel1_2" );
    layout2->addWidget( textLabel1_2 );

    groupBox1 = new QGroupBox( privateLayoutWidget, "groupBox1" );
    groupBox1->setColumnLayout(0, Qt::Vertical );
    groupBox1->layout()->setSpacing( 6 );
    groupBox1->layout()->setMargin( 11 );
    groupBox1Layout = new QHBoxLayout( groupBox1->layout() );
    groupBox1Layout->setAlignment( Qt::AlignTop );

    textLabel1 = new QLabel( groupBox1, "textLabel1" );
    groupBox1Layout->addWidget( textLabel1 );

    m_SelectCommentStyle = new QComboBox( FALSE, groupBox1, "m_SelectCommentStyle" );
    groupBox1Layout->addWidget( m_SelectCommentStyle );
    layout2->addWidget( groupBox1 );

    groupBox2 = new QGroupBox( privateLayoutWidget, "groupBox2" );
    groupBox2->setColumnLayout(0, Qt::Vertical );
    groupBox2->layout()->setSpacing( 6 );
    groupBox2->layout()->setMargin( 11 );
    groupBox2Layout = new QVBoxLayout( groupBox2->layout() );
    groupBox2Layout->setAlignment( Qt::AlignTop );

    m_generateConstructors = new QCheckBox( groupBox2, "m_generateConstructors" );
    m_generateConstructors->setChecked( FALSE );
    groupBox2Layout->addWidget( m_generateConstructors );

    m_generateAccessors = new QCheckBox( groupBox2, "m_generateAccessors" );
    m_generateAccessors->setChecked( TRUE );
    groupBox2Layout->addWidget( m_generateAccessors );
    layout2->addWidget( groupBox2 );

    groupBox3 = new QGroupBox( privateLayoutWidget, "groupBox3" );
    groupBox3->setColumnLayout(0, Qt::Vertical );
    groupBox3->layout()->setSpacing( 6 );
    groupBox3->layout()->setMargin( 11 );
    groupBox3Layout = new QVBoxLayout( groupBox3->layout() );
    groupBox3Layout->setAlignment( Qt::AlignTop );

    m_makeANTDocumentCheckBox = new QCheckBox( groupBox3, "m_makeANTDocumentCheckBox" );
    groupBox3Layout->addWidget( m_makeANTDocumentCheckBox );
    layout2->addWidget( groupBox3 );
    languageChange();
    resize( QSize(461, 443).expandedTo(minimumSizeHint()) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
JavaCodeGenerationFormBase::~JavaCodeGenerationFormBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void JavaCodeGenerationFormBase::languageChange()
{
    setCaption( tr2i18n( "Form1" ) );
    textLabel1_2->setText( tr2i18n( "<p align=\"center\">Java Code Generation</p>" ) );
    groupBox1->setTitle( tr2i18n( "Documentation" ) );
    textLabel1->setText( tr2i18n( "Style:" ) );
    m_SelectCommentStyle->clear();
    m_SelectCommentStyle->insertItem( tr2i18n( "Slash-Star (/**  */)" ) );
    m_SelectCommentStyle->insertItem( tr2i18n( "Slash-Slash (//)" ) );
    groupBox2->setTitle( tr2i18n( "Auto-Generate Methods" ) );
    m_generateConstructors->setText( tr2i18n( "Empty Constructors" ) );
    m_generateAccessors->setText( tr2i18n( "Accessor Methods" ) );
    groupBox3->setTitle( tr2i18n( "Project Generation" ) );
    m_makeANTDocumentCheckBox->setText( tr2i18n( "Create ANT build document" ) );
}

#include "javacodegenerationformbase.moc"
