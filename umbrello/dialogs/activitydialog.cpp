/*
 *  copyright (C) 2002-2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "activitydialog.h"

//qt includes
#include <qlayout.h>
//Added by qt3to4:
#include <kicon.h>
#include <QFrame>
#include <QHBoxLayout>
#include <QGridLayout>
#include <kvbox.h>
//kde includes
#include <kiconloader.h>
#include <klocale.h>

//local includes
#include "../umlview.h"
#include "../activitywidget.h"
#include "../dialog_utils.h"

ActivityDialog::ActivityDialog( UMLView * pView, ActivityWidget * pWidget )
        : KPageDialog(pView) {
    setCaption( i18n("Properties") );
    setButtons( Ok | Apply | Cancel | Help );
    setDefaultButton( Ok );
    setModal( true );
    setFaceType( KPageDialog::List );
    showButtonSeparator( true );
    m_pView = pView;
    m_pActivityWidget = pWidget;
    m_bChangesMade = false;
    setupPages();
}

void ActivityDialog::slotOk() {
    applyPage( pageItemColor );
    applyPage( pageItemFont );
    applyPage( pageItemGeneral );
    accept();
}

void ActivityDialog::slotApply() {
    applyPage(currentPage());
}

void ActivityDialog::setupPages() {
    setupGeneralPage();
    setupColorPage();
    setupFontPage();
}

void ActivityDialog::applyPage( KPageWidgetItem *item ) {
    m_bChangesMade = true;
    if ( item == pageItemGeneral )
    {
        m_pActivityWidget -> setName( m_GenPageWidgets.nameLE -> text() );
        m_pActivityWidget -> setDoc( m_GenPageWidgets.docMLE -> text() );
    }
    else if ( item == pageItemFont )
    {
        m_pActivityWidget -> setFont( m_pChooser -> font() );
    }
    else if ( item == pageItemColor )
    {
        m_pColorPage -> updateUMLWidget();
    }
}

void ActivityDialog::setupGeneralPage() {
    QString types[ ] = { i18n("Initial activity"), i18n("Activity"), i18n("End activity"), i18n( "Branch/Merge"), i18n( "Fork/Join" ) };
    ActivityWidget::ActivityType type = m_pActivityWidget -> getActivityType();


    KVBox *page = new KVBox();
    pageItemGeneral = new KPageWidgetItem( page, i18n("General") );
    pageItemGeneral->setHeader(i18n("General Properties"));
    pageItemGeneral->setIcon( KIcon(DesktopIcon( "misc")) );
    addPage( pageItemGeneral );

    m_GenPageWidgets.generalGB = new Q3GroupBox( i18n( "Properties"), (QWidget *)page );

    QGridLayout * generalLayout = new QGridLayout( m_GenPageWidgets.generalGB );
    generalLayout -> setSpacing( spacingHint() );
    generalLayout -> setMargin(  fontMetrics().height()  );

    QString actType ( types[ (int)type ] );
    Dialog_Utils::makeLabeledEditField( m_GenPageWidgets.generalGB, generalLayout, 0,
                                    m_GenPageWidgets.typeL, i18n("Activity type:"),
                                    m_GenPageWidgets.typeLE, actType );
    m_GenPageWidgets.typeLE -> setEnabled( false );

    Dialog_Utils::makeLabeledEditField( m_GenPageWidgets.generalGB, generalLayout, 1,
                                    m_GenPageWidgets.nameL, i18n("Activity name:"),
                                    m_GenPageWidgets.nameLE );

    m_GenPageWidgets.docGB = new Q3GroupBox( i18n( "Documentation"), (QWidget *)page );

    QHBoxLayout * docLayout = new QHBoxLayout( m_GenPageWidgets.docGB );
    docLayout -> setSpacing( spacingHint() );
    docLayout -> setMargin(  fontMetrics().height()  );

    m_GenPageWidgets.docMLE = new Q3MultiLineEdit( m_GenPageWidgets.docGB );
    m_GenPageWidgets.docMLE -> setText( m_pActivityWidget -> getDoc() );
    docLayout -> addWidget( m_GenPageWidgets.docMLE );

    if( type != ActivityWidget::Normal ) {
        m_GenPageWidgets.nameLE -> setEnabled( false );
        m_GenPageWidgets.nameLE -> setText( "" );
    } else
        m_GenPageWidgets.nameLE -> setText( m_pActivityWidget -> getName() );
}

void ActivityDialog::setupFontPage() {
    KVBox *page = new KVBox();
    pageItemFont = new KPageWidgetItem( page, i18n("Font") );
    pageItemFont->setHeader( i18n("Font Settings") );
    pageItemFont->setIcon( KIcon(DesktopIcon( "fonts") ));
    addPage( pageItemFont );
    m_pChooser = new KFontChooser( (QWidget*)page, false, QStringList(), false);
    m_pChooser -> setFont( m_pActivityWidget -> getFont() );
}

void ActivityDialog::setupColorPage() {
    QFrame *colorPage = new QFrame();
    pageItemColor = new KPageWidgetItem( colorPage, i18n("Color") );
    pageItemColor->setHeader( i18n("Widget Colors") );
    pageItemColor->setIcon( KIcon(DesktopIcon( "colors") ));
    addPage( pageItemColor );
    QHBoxLayout * m_pColorLayout = new QHBoxLayout(colorPage);
    m_pColorPage = new UMLWidgetColorPage( colorPage, m_pActivityWidget );
    m_pColorLayout -> addWidget(m_pColorPage);
}

#include "activitydialog.moc"
