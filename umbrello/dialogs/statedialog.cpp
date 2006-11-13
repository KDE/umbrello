/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "statedialog.h"

//qt includes
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qgroupbox.h>

//kde includes
#include <kiconloader.h>
#include <klocale.h>
#include <kfontdialog.h>

//local includes
#include "../umlview.h"
#include "../statewidget.h"
#include "../dialog_utils.h"

StateDialog::StateDialog( UMLView * pView, StateWidget * pWidget )
        : KDialogBase(IconList, i18n("Properties"), Ok | Apply | Cancel | Help, Ok, pView, "_STATEDIALOG_", true, true) {
    m_pActivityPage = 0;
    m_pView = pView;
    m_pStateWidget = pWidget;
    m_bChangesMade = false;
    setupPages();
}

void StateDialog::slotOk() {
    applyPage( GeneralPage );
    applyPage( Activity_Page );
    applyPage( ColorPage );
    applyPage( FontPage );
    accept();
}

void StateDialog::slotApply() {
    applyPage( (Page) activePageIndex() );
}

void StateDialog::setupPages() {
    setupGeneralPage();
    if( m_pStateWidget -> getStateType() == StateWidget::Normal )
        setupActivityPage();
    setupColorPage();
    setupFontPage();
}

void StateDialog::applyPage( Page page ) {
    m_bChangesMade = true;
    switch( page ) {
    case GeneralPage:
        m_pStateWidget -> setName( m_GenPageWidgets.nameLE -> text() );
        m_pStateWidget -> setDoc( m_GenPageWidgets.docMLE -> text() );
        break;

    case Activity_Page:
        if( m_pActivityPage )
            m_pActivityPage -> updateActivities();
        break;

    case ColorPage:
        m_pColorPage -> updateUMLWidget();
        break;

    case FontPage:
        m_pStateWidget -> setFont( m_pChooser -> font() );
        break;
    }//end switch
}

void StateDialog::setupGeneralPage() {
    QString types[ ] = { i18n("Initial state"), i18n("State"), i18n("End state") };
    StateWidget::StateType type = m_pStateWidget -> getStateType();

    QVBox * page = addVBoxPage( i18n("General"), i18n("General Properties"), DesktopIcon( "misc") );
    m_GenPageWidgets.generalGB = new QGroupBox( i18n( "Properties"), (QWidget *)page );

    QGridLayout * generalLayout = new QGridLayout( m_GenPageWidgets.generalGB, 2, 2 );
    generalLayout -> setSpacing( spacingHint() );
    generalLayout -> setMargin(  fontMetrics().height()  );

    Dialog_Utils::makeLabeledEditField( m_GenPageWidgets.generalGB, generalLayout, 0,
                                    m_GenPageWidgets.typeL, i18n("State type:"),
                                    m_GenPageWidgets.typeLE, types[ (int)type ] );
    m_GenPageWidgets.typeLE -> setEnabled( false );

    Dialog_Utils::makeLabeledEditField( m_GenPageWidgets.generalGB, generalLayout, 1,
                                    m_GenPageWidgets.nameL, i18n("State name:"),
                                    m_GenPageWidgets.nameLE );

    m_GenPageWidgets.docGB = new QGroupBox( i18n( "Documentation"), (QWidget *)page );

    QHBoxLayout * docLayout = new QHBoxLayout( m_GenPageWidgets.docGB );
    docLayout -> setSpacing( spacingHint() );
    docLayout -> setMargin(  fontMetrics().height()  );

    m_GenPageWidgets.docMLE = new QMultiLineEdit( m_GenPageWidgets.docGB );
    m_GenPageWidgets.docMLE -> setText( m_pStateWidget -> getDoc() );
    docLayout -> addWidget( m_GenPageWidgets.docMLE );

    if( type != StateWidget::Normal ) {
        m_GenPageWidgets.nameLE -> setEnabled( false );
        m_GenPageWidgets.nameLE -> setText( "" );
    } else
        m_GenPageWidgets.nameLE -> setText( m_pStateWidget -> getName() );
}

void StateDialog::setupFontPage() {
    if ( !m_pStateWidget )
        return;
    QVBox * page = addVBoxPage( i18n("Font"), i18n("Font Settings"), DesktopIcon( "fonts")  );
    m_pChooser = new KFontChooser( (QWidget*)page, "font", false, QStringList(), false);
    m_pChooser -> setFont( m_pStateWidget -> getFont() );
}

void StateDialog::setupColorPage() {
    QFrame * colorPage = addPage( i18n("Color"), i18n("Widget Color"), DesktopIcon( "colors") );
    QHBoxLayout * m_pColorLayout = new QHBoxLayout(colorPage);
    m_pColorPage = new UMLWidgetColorPage( colorPage, m_pStateWidget );
    m_pColorLayout -> addWidget(m_pColorPage);
}

void StateDialog::setupActivityPage() {
    QFrame * activityPage = addPage( i18n("Activities"), i18n("Activities"), DesktopIcon( "misc") );
    QHBoxLayout * activityLayout = new QHBoxLayout( activityPage );
    m_pActivityPage = new ActivityPage( activityPage, m_pStateWidget );
    activityLayout -> addWidget( m_pActivityPage );
}





#include "statedialog.moc"
