/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "objectnodedialog.h"

//qt includes
#include <QtGui/QLayout>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QRadioButton>
//kde includes
#include <kvbox.h>
#include <klocale.h>

//local includes
#include "../umlview.h"
#include "../dialog_utils.h"
#include "../icon_utils.h"
#include "../objectnodewidget.h"


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

void ObjectNodeDialog::slotOk()
{
    applyPage( pageItemColor );
    applyPage( pageItemFont );
    applyPage( pageItemGeneral );
    accept();
}

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

void ObjectNodeDialog::setupPages()
{
    setupGeneralPage();
    setupColorPage();
    setupFontPage();
}

void ObjectNodeDialog::applyPage( KPageWidgetItem *item )
{
    m_bChangesMade = true;
    if ( item == pageItemGeneral )
    {
        m_pObjectNodeWidget -> setName( m_GenPageWidgets.nameLE -> text() );
        m_pObjectNodeWidget -> setDoc( m_GenPageWidgets.docMLE -> text() );
        m_pObjectNodeWidget -> setState( m_GenPageWidgets.stateLE -> text() );

        ObjectNodeWidget::ObjectNodeType newType = ObjectNodeWidget::Normal;
        if ( m_GenPageWidgets.BufferRB->isChecked() )
            newType = ObjectNodeWidget::Buffer;
        else if ( m_GenPageWidgets.DataRB->isChecked() )
            newType = ObjectNodeWidget::Data;
        else if (m_GenPageWidgets.FlowRB->isChecked() )
             newType = ObjectNodeWidget::Flow;

        m_pObjectNodeWidget->setObjectNodeType (newType);

    }
    else if ( item == pageItemFont )
    {
        m_pObjectNodeWidget -> setFont( m_pChooser -> font() );
    }
    else if ( item == pageItemColor )
    {
        m_pColorPage -> updateUMLWidget();
    }
}


void ObjectNodeDialog::setupGeneralPage()
{
    QString types[ ] = { i18n("Central Buffer"), i18n("Data Store"), i18n("ObjectFlow")};
    ObjectNodeWidget::ObjectNodeType type = m_pObjectNodeWidget -> getObjectNodeType();

    KVBox *page = new KVBox();
    pageItemGeneral = new KPageWidgetItem( page, i18n("General") );
    pageItemGeneral->setHeader(i18n("General Properties"));
    pageItemGeneral->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_General) );
    addPage( pageItemGeneral );

    m_GenPageWidgets.generalGB = new Q3GroupBox( i18n( "Properties"), (QWidget *)page );

    QGridLayout * generalLayout = new QGridLayout( m_GenPageWidgets.generalGB );
    generalLayout -> setSpacing( spacingHint() );
    generalLayout -> setMargin(  fontMetrics().height()  );

    QString objType ( types[ (int)type ] );
    Dialog_Utils::makeLabeledEditField( m_GenPageWidgets.generalGB, generalLayout, 0,
                                    m_GenPageWidgets.typeL, i18n("Object Node type:"),
                                    m_GenPageWidgets.typeLE, objType );
    m_GenPageWidgets.typeLE -> setEnabled( false );

    Dialog_Utils::makeLabeledEditField( m_GenPageWidgets.generalGB, generalLayout, 1,
                                    m_GenPageWidgets.nameL, i18n("Object Node name:"),
                                    m_GenPageWidgets.nameLE );

    Dialog_Utils::makeLabeledEditField( m_GenPageWidgets.generalGB, generalLayout, 2,
                                    m_GenPageWidgets.stateL, i18n("State :"),
                                    m_GenPageWidgets.stateLE );
    m_GenPageWidgets.stateL->hide();
    m_GenPageWidgets.stateLE->hide();

    m_GenPageWidgets.BufferRB = new QRadioButton( i18n("&Central Buffer"),(QWidget *)page);
    generalLayout -> addWidget( m_GenPageWidgets.BufferRB );

    m_GenPageWidgets.DataRB = new QRadioButton( i18n("&Data Store "),(QWidget *)page);
    generalLayout -> addWidget( m_GenPageWidgets.DataRB );

    m_GenPageWidgets.FlowRB = new QRadioButton( i18n("&Object Flow"),(QWidget *)page);
    generalLayout -> addWidget( m_GenPageWidgets.FlowRB );

    if (type == ObjectNodeWidget::Flow)
    {
        showState();
    }

    connect(m_GenPageWidgets.BufferRB,SIGNAL(clicked()),this,SLOT(slotHideState()));
    connect(m_GenPageWidgets.DataRB,SIGNAL(clicked()),this,SLOT(slotHideState()));
    connect(m_GenPageWidgets.FlowRB,SIGNAL(clicked()),this,SLOT(slotShowState()));

    ObjectNodeWidget::ObjectNodeType newType = m_pObjectNodeWidget -> getObjectNodeType() ;

    m_GenPageWidgets.BufferRB->setChecked(newType == ObjectNodeWidget::Buffer);

    m_GenPageWidgets.DataRB->setChecked (newType == ObjectNodeWidget::Data);

    m_GenPageWidgets.FlowRB->setChecked (newType == ObjectNodeWidget::Flow);

    m_GenPageWidgets.docGB = new Q3GroupBox( i18n( "Documentation"), (QWidget *)page );

    QHBoxLayout * docLayout = new QHBoxLayout( m_GenPageWidgets.docGB );
    docLayout -> setSpacing( spacingHint() );
    docLayout -> setMargin(  fontMetrics().height()  );

    m_GenPageWidgets.docMLE = new Q3MultiLineEdit( m_GenPageWidgets.docGB );
    m_GenPageWidgets.docMLE -> setText( m_pObjectNodeWidget -> getDoc() );
    docLayout -> addWidget( m_GenPageWidgets.docMLE );

    if( type != ObjectNodeWidget::Buffer && type != ObjectNodeWidget::Data && type != ObjectNodeWidget::Flow) {
        m_GenPageWidgets.nameLE -> setEnabled( false );
        m_GenPageWidgets.nameLE -> setText( "" );
    } else
        m_GenPageWidgets.nameLE -> setText( m_pObjectNodeWidget -> getName() );
}

void ObjectNodeDialog::setupFontPage()
{
    KVBox *page = new KVBox();
    pageItemFont = new KPageWidgetItem( page, i18n("Font") );
    pageItemFont->setHeader( i18n("Font Settings") );
    pageItemFont->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Font) );
    addPage( pageItemFont );
    m_pChooser = new KFontChooser( (QWidget*)page, false, QStringList(), false);
    m_pChooser -> setFont( m_pObjectNodeWidget -> getFont() );
}

void ObjectNodeDialog::showState()
{
    m_GenPageWidgets.stateL->show();
    m_GenPageWidgets.stateLE->show();

    if (m_pObjectNodeWidget->getState() != NULL)
    {
        m_GenPageWidgets.stateLE->setText(m_pObjectNodeWidget->getState());
    }
}

void ObjectNodeDialog::setupColorPage()
{
    QFrame *colorPage = new QFrame();
    pageItemColor = new KPageWidgetItem( colorPage, i18n("Color") );
    pageItemColor->setHeader( i18n("Widget Colors") );
    pageItemColor->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Color) );
    addPage( pageItemColor );
    QHBoxLayout * m_pColorLayout = new QHBoxLayout(colorPage);
    m_pColorPage = new UMLWidgetColorPage( colorPage, m_pObjectNodeWidget );
    m_pColorLayout -> addWidget(m_pColorPage);
}

#include "objectnodedialog.moc"
