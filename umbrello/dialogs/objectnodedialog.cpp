/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "objectnodedialog.h"

// local includes
#include "umlview.h"
#include "dialog_utils.h"
#include "icon_utils.h"
#include "objectnodewidget.h"

// kde includes
#include <kdebug.h>
#include <klineedit.h>
#include <klocale.h>
#include <ktextedit.h>
#include <kvbox.h>

// qt includes
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>

ObjectNodeDialog::ObjectNodeDialog( UMLView * pView, ObjectNodeWidget * pWidget )
        : KPageDialog(pView)
{
    setCaption( i18n("Properties") );
    setButtons( Ok | Apply | Cancel | Help );
    setDefaultButton( Ok );
    setModal( true );
    setFaceType( KPageDialog::List );
    showButtonSeparator( true );
    m_pView = pView;
    m_pObjectNodeWidget = pWidget;
    m_bChangesMade = false;
    setupPages();
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

/**
 * Entered when OK button pressed.
 */
void ObjectNodeDialog::slotOk()
{
    applyPage( pageItemColor );
    applyPage( pageItemFont );
    applyPage( pageItemGeneral );
    accept();
}

/**
 * Entered when Apply button pressed.
 */
void ObjectNodeDialog::slotApply()
{
    applyPage(currentPage());
}

void ObjectNodeDialog::slotShowState()
{
    m_GenPageWidgets.stateL->show();
    m_GenPageWidgets.stateLE->show();

    if (m_pObjectNodeWidget->getState() != NULL)
    {
        m_GenPageWidgets.stateLE->setText(m_pObjectNodeWidget->getState());
    }
}

void ObjectNodeDialog::slotHideState()
{
    m_GenPageWidgets.stateL->hide();
    m_GenPageWidgets.stateLE->hide();
}

/**
 * Sets up the pages of the dialog.
 */
void ObjectNodeDialog::setupPages()
{
    setupGeneralPage();
    setupColorPage();
    setupFontPage();
}

/**
 * Applies changes to the given page.
 */
void ObjectNodeDialog::applyPage( KPageWidgetItem *item )
{
    m_bChangesMade = true;
    if ( item == pageItemGeneral )
    {
        m_pObjectNodeWidget->setName( m_GenPageWidgets.nameLE->text() );
        m_pObjectNodeWidget->setDocumentation( m_GenPageWidgets.docMLE->toPlainText() );
        m_pObjectNodeWidget->setState( m_GenPageWidgets.stateLE->text() );

        ObjectNodeWidget::ObjectNodeType newType = ObjectNodeWidget::Normal;
        if ( m_GenPageWidgets.bufferRB->isChecked() )
            newType = ObjectNodeWidget::Buffer;
        else if ( m_GenPageWidgets.dataRB->isChecked() )
            newType = ObjectNodeWidget::Data;
        else if (m_GenPageWidgets.flowRB->isChecked() )
             newType = ObjectNodeWidget::Flow;

        m_pObjectNodeWidget->setObjectNodeType (newType);
    }
    else if ( item == pageItemFont )
    {
        m_pObjectNodeWidget->setFont( m_pChooser->font() );
    }
    else if ( item == pageItemColor )
    {
        m_pColorPage->updateUMLWidget();
    }
}

/**
 * Sets up the general page of the dialog.
 */
void ObjectNodeDialog::setupGeneralPage()
{
    QStringList types;
    types << i18n("Central Buffer") << i18n("Data Store") << i18n("ObjectFlow");
    ObjectNodeWidget::ObjectNodeType type = m_pObjectNodeWidget->getObjectNodeType();

    KVBox *page = new KVBox();
    pageItemGeneral = new KPageWidgetItem( page, i18n("General") );
    pageItemGeneral->setHeader(i18n("General Properties"));
    pageItemGeneral->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_General) );
    addPage( pageItemGeneral );

    m_GenPageWidgets.generalGB = new QGroupBox( i18nc("properties group title", "Properties"), (QWidget *)page );

    QGridLayout * generalLayout = new QGridLayout( m_GenPageWidgets.generalGB );
    generalLayout->setSpacing( spacingHint() );
    generalLayout->setMargin(  fontMetrics().height()  );

    QString objType;
    if (type < types.count()) {
        objType = types.at((int)type);
    }
    else {
        uWarning() << "type of ObjectNodeWidget is out of range! Value = " << type;
    }
    Dialog_Utils::makeLabeledEditField( m_GenPageWidgets.generalGB, generalLayout, 0,
                                    m_GenPageWidgets.typeL, i18n("Object Node type:"),
                                    m_GenPageWidgets.typeLE, objType );
    m_GenPageWidgets.typeLE->setEnabled( false );

    Dialog_Utils::makeLabeledEditField( m_GenPageWidgets.generalGB, generalLayout, 1,
                                    m_GenPageWidgets.nameL, i18n("Object Node name:"),
                                    m_GenPageWidgets.nameLE );

    Dialog_Utils::makeLabeledEditField( m_GenPageWidgets.generalGB, generalLayout, 2,
                                    m_GenPageWidgets.stateL, i18nc("enter state label", "State :"),
                                    m_GenPageWidgets.stateLE );
    m_GenPageWidgets.stateL->hide();
    m_GenPageWidgets.stateLE->hide();

    m_GenPageWidgets.bufferRB = new QRadioButton( i18n("&Central Buffer"),(QWidget *)page);
    generalLayout->addWidget( m_GenPageWidgets.bufferRB );

    m_GenPageWidgets.dataRB = new QRadioButton( i18n("&Data Store "),(QWidget *)page);
    generalLayout->addWidget( m_GenPageWidgets.dataRB );

    m_GenPageWidgets.flowRB = new QRadioButton( i18n("&Object Flow"),(QWidget *)page);
    generalLayout->addWidget( m_GenPageWidgets.flowRB );

    if (type == ObjectNodeWidget::Flow)
    {
        showState();
    }

    connect(m_GenPageWidgets.bufferRB,SIGNAL(clicked()),this,SLOT(slotHideState()));
    connect(m_GenPageWidgets.dataRB,SIGNAL(clicked()),this,SLOT(slotHideState()));
    connect(m_GenPageWidgets.flowRB,SIGNAL(clicked()),this,SLOT(slotShowState()));

    ObjectNodeWidget::ObjectNodeType newType = m_pObjectNodeWidget->getObjectNodeType() ;

    m_GenPageWidgets.bufferRB->setChecked(newType == ObjectNodeWidget::Buffer);
    m_GenPageWidgets.dataRB->setChecked (newType == ObjectNodeWidget::Data);
    m_GenPageWidgets.flowRB->setChecked (newType == ObjectNodeWidget::Flow);

    m_GenPageWidgets.docGB = new QGroupBox( i18n( "Documentation"), (QWidget *)page );

    QHBoxLayout * docLayout = new QHBoxLayout( m_GenPageWidgets.docGB );
    docLayout->setSpacing( spacingHint() );
    docLayout->setMargin(  fontMetrics().height()  );

    m_GenPageWidgets.docMLE = new KTextEdit( m_GenPageWidgets.docGB );
    m_GenPageWidgets.docMLE->setText( m_pObjectNodeWidget->documentation() );
    docLayout->addWidget( m_GenPageWidgets.docMLE );

    if (type != ObjectNodeWidget::Buffer && type != ObjectNodeWidget::Data && type != ObjectNodeWidget::Flow) {
        m_GenPageWidgets.nameLE->setEnabled( false );
        m_GenPageWidgets.nameLE->setText( "" );
    } else
        m_GenPageWidgets.nameLE->setText( m_pObjectNodeWidget->name() );
}

/**
 * Sets up the font selection page.
 */
void ObjectNodeDialog::setupFontPage()
{
    KVBox *page = new KVBox();
    pageItemFont = new KPageWidgetItem( page, i18n("Font") );
    pageItemFont->setHeader( i18n("Font Settings") );
    pageItemFont->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Font) );
    addPage( pageItemFont );
    m_pChooser = new KFontChooser( (QWidget*)page, false, QStringList(), false);
    m_pChooser->setFont( m_pObjectNodeWidget->font() );
}

/**
 * Show the State entry text.
 */
void ObjectNodeDialog::showState()
{
    m_GenPageWidgets.stateL->show();
    m_GenPageWidgets.stateLE->show();

    if (m_pObjectNodeWidget->getState() != NULL)
    {
        m_GenPageWidgets.stateLE->setText(m_pObjectNodeWidget->getState());
    }
}

/**
 * Sets up the color page.
 */
void ObjectNodeDialog::setupColorPage()
{
    QFrame *colorPage = new QFrame();
    pageItemColor = new KPageWidgetItem( colorPage, i18nc("color page title", "Color") );
    pageItemColor->setHeader( i18n("Widget Colors") );
    pageItemColor->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Color) );
    addPage( pageItemColor );
    QHBoxLayout * m_pColorLayout = new QHBoxLayout(colorPage);
    m_pColorPage = new UMLWidgetColorPage( colorPage, m_pObjectNodeWidget );
    m_pColorLayout->addWidget(m_pColorPage);
}

#include "objectnodedialog.moc"
