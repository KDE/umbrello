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
#include "activitydialog.h"

//qt includes
#include <qlayout.h>

//kde includes
#include <kiconloader.h>
#include <klocale.h>

//local includes
#include "../umlview.h"
#include "../activitywidget.h"
#include "../dialog_utils.h"

ActivityDialog::ActivityDialog( UMLView * pView, ActivityWidget * pWidget )
        : KDialogBase(IconList, i18n("Properties"), Ok | Apply | Cancel | Help, Ok, pView, "_STATEDIALOG_", true, true) {
    m_pView = pView;
    m_pActivityWidget = pWidget;
    m_bChangesMade = false;
    setupPages();
}

void ActivityDialog::slotOk() {
    applyPage( GeneralPage );
    applyPage( ColorPage );
    applyPage( FontPage );
    accept();
}

void ActivityDialog::slotApply() {
    applyPage( (Page) activePageIndex() );
}

void ActivityDialog::setupPages() {
    setupGeneralPage();
    setupColorPage();
    setupFontPage();
}

void ActivityDialog::applyPage( Page page ) {
    m_bChangesMade = true;
    switch( page ) {
    case GeneralPage:
        m_pActivityWidget -> setName( m_GenPageWidgets.nameLE -> text() );
        m_pActivityWidget -> setDoc( m_GenPageWidgets.docMLE -> text() );
        break;

    case ColorPage:
        m_pColorPage -> updateUMLWidget();

    case FontPage:
        m_pActivityWidget -> setFont( m_pChooser -> font() );
        break;
    }//end switch
}

void ActivityDialog::setupGeneralPage() {
    QString types[ ] = { i18n("Initial activity"), i18n("Activity"), i18n("End activity"), i18n( "Branch/Merge"), i18n( "Fork/Join" ) };
    ActivityWidget::ActivityType type = m_pActivityWidget -> getActivityType();

    QVBox * page = addVBoxPage( i18n("General"), i18n("General Properties"), DesktopIcon( "misc") );
    m_GenPageWidgets.generalGB = new QGroupBox( i18n( "Properties"), (QWidget *)page );

    QGridLayout * generalLayout = new QGridLayout( m_GenPageWidgets.generalGB, 2, 2 );
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

    m_GenPageWidgets.docGB = new QGroupBox( i18n( "Documentation"), (QWidget *)page );

    QHBoxLayout * docLayout = new QHBoxLayout( m_GenPageWidgets.docGB );
    docLayout -> setSpacing( spacingHint() );
    docLayout -> setMargin(  fontMetrics().height()  );

    m_GenPageWidgets.docMLE = new QMultiLineEdit( m_GenPageWidgets.docGB );
    m_GenPageWidgets.docMLE -> setText( m_pActivityWidget -> getDoc() );
    docLayout -> addWidget( m_GenPageWidgets.docMLE );

    if( type != ActivityWidget::Normal ) {
        m_GenPageWidgets.nameLE -> setEnabled( false );
        m_GenPageWidgets.nameLE -> setText( "" );
    } else
        m_GenPageWidgets.nameLE -> setText( m_pActivityWidget -> getName() );
}

void ActivityDialog::setupFontPage() {
    QVBox * page = addVBoxPage( i18n("Font"), i18n("Font Settings"), DesktopIcon( "fonts")  );
    m_pChooser = new KFontChooser( (QWidget*)page, "font", false, QStringList(), false);
    m_pChooser -> setFont( m_pActivityWidget -> getFont() );
}

void ActivityDialog::setupColorPage() {
    QFrame * colorPage = addPage( i18n("Color"), i18n("Widget Colors"), DesktopIcon( "colors") );
    QHBoxLayout * m_pColorLayout = new QHBoxLayout(colorPage);
    m_pColorPage = new UMLWidgetColorPage( colorPage, m_pActivityWidget );
    m_pColorLayout -> addWidget(m_pColorPage);
}






#include "activitydialog.moc"
