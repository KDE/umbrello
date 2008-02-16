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
#include "statedialog.h"

//qt includes
#include <QtGui/QLabel>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <q3multilineedit.h>
#include <q3groupbox.h>
//kde includes
#include <kvbox.h>
#include <klineedit.h>
#include <klocale.h>
#include <kfontdialog.h>

//local includes
#include "../umlview.h"
#include "../statewidget.h"
#include "../dialog_utils.h"
#include "../icon_utils.h"


StateDialog::StateDialog( UMLView * pView, StateWidget * pWidget )
    : KPageDialog( pView )
{
    setCaption(i18n("Properties") );
    setButtons( Help | Default | Apply | Ok | Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );
    setFaceType( KPageDialog::List );

    m_pActivityPage = 0;
    m_pView = pView;
    m_pStateWidget = pWidget;
    m_bChangesMade = false;
    setupPages();
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

void StateDialog::slotOk()
{
    applyPage( pageGeneral );
    applyPage( pageFont );
    applyPage( pageActivity );
    applyPage( pageColor );
    accept();
}

void StateDialog::slotApply() {
    applyPage( currentPage() );
}

void StateDialog::setupPages()
{
    setupGeneralPage();
    if( m_pStateWidget -> getStateType() == StateWidget::Normal )
        setupActivityPage();
    setupColorPage();
    setupFontPage();
}

void StateDialog::applyPage( KPageWidgetItem*item )
{
    m_bChangesMade = true;
    if ( item == pageGeneral )
    {
        m_pStateWidget -> setName( m_GenPageWidgets.nameLE -> text() );
        m_pStateWidget -> setDoc( m_GenPageWidgets.docMLE -> text() );
    }
    else if ( item == pageActivity )
    {
        if( m_pActivityPage )
            m_pActivityPage -> updateActivities();
    }
    else if ( item == pageColor )
    {
        m_pColorPage -> updateUMLWidget();
    }
    else if ( item == pageFont )
    {
        m_pStateWidget -> setFont( m_pChooser -> font() );
    }
}

void StateDialog::setupGeneralPage()
{
    QString types[ ] = { i18n("Initial state"), i18n("State"), i18n("End state") };
    StateWidget::StateType type = m_pStateWidget -> getStateType();

    KVBox * page = new KVBox();
    pageGeneral = new KPageWidgetItem( page,i18n("General")  );
    pageGeneral->setHeader( i18n("General Properties") );
    pageGeneral->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_General) );
    addPage( pageGeneral );

    m_GenPageWidgets.generalGB = new Q3GroupBox( i18n( "Properties"), (QWidget *)page );

    QGridLayout * generalLayout = new QGridLayout( m_GenPageWidgets.generalGB );
    generalLayout -> setSpacing( spacingHint() );
    generalLayout -> setMargin(  fontMetrics().height()  );

    Dialog_Utils::makeLabeledEditField( m_GenPageWidgets.generalGB, generalLayout, 0,
                                    m_GenPageWidgets.typeL, i18n("State type:"),
                                    m_GenPageWidgets.typeLE, types[ (int)type ] );
    m_GenPageWidgets.typeLE -> setEnabled( false );

    Dialog_Utils::makeLabeledEditField( m_GenPageWidgets.generalGB, generalLayout, 1,
                                    m_GenPageWidgets.nameL, i18n("State name:"),
                                    m_GenPageWidgets.nameLE );

    m_GenPageWidgets.docGB = new Q3GroupBox( i18n( "Documentation"), (QWidget *)page );

    QHBoxLayout * docLayout = new QHBoxLayout( m_GenPageWidgets.docGB );
    docLayout -> setSpacing( spacingHint() );
    docLayout -> setMargin(  fontMetrics().height()  );

    m_GenPageWidgets.docMLE = new Q3MultiLineEdit( m_GenPageWidgets.docGB );
    m_GenPageWidgets.docMLE -> setText( m_pStateWidget -> getDoc() );
    docLayout -> addWidget( m_GenPageWidgets.docMLE );

    if( type != StateWidget::Normal ) {
        m_GenPageWidgets.nameLE -> setEnabled( false );
        m_GenPageWidgets.nameLE -> setText( "" );
    } else
        m_GenPageWidgets.nameLE -> setText( m_pStateWidget -> getName() );
}

void StateDialog::setupFontPage()
{
    if ( !m_pStateWidget )
        return;
    KVBox * page = new KVBox();
    pageFont = new KPageWidgetItem( page,i18n("Font")  );
    pageFont->setHeader( i18n("Font Settings") );
    pageFont->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Font) );
    addPage( pageFont );
    m_pChooser = new KFontChooser( (QWidget*)page, false, QStringList(), false);
    m_pChooser -> setFont( m_pStateWidget -> getFont() );
}

void StateDialog::setupColorPage()
{
    QFrame * colorPage = new QFrame();
    pageColor = new KPageWidgetItem( colorPage,i18n("Color")  );
    pageColor->setHeader( i18n("Widget Color") );
    pageColor->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Color) );
    addPage( pageColor );
    QHBoxLayout * m_pColorLayout = new QHBoxLayout(colorPage);
    m_pColorPage = new UMLWidgetColorPage( colorPage, m_pStateWidget );
    m_pColorLayout -> addWidget(m_pColorPage);
}

void StateDialog::setupActivityPage()
{
    QFrame * activityPage = new QFrame();
    pageActivity = new KPageWidgetItem( activityPage,i18n("Activities")  );
    pageActivity->setHeader( i18n("Activities") );
    pageActivity->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Activities) );
    addPage( pageActivity );

    QHBoxLayout * activityLayout = new QHBoxLayout( activityPage );
    m_pActivityPage = new ActivityPage( activityPage, m_pStateWidget );
    activityLayout -> addWidget( m_pActivityPage );
}


#include "statedialog.moc"
