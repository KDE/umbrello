/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlviewdialog.h"

// local includes
#include "classoptionspage.h"
#include "debug_utils.h"
#include "icon_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidgetstylepage.h"

// kde includes
#include <klocale.h>
#include <kmessagebox.h>
#include <kfontdialog.h>
#include <kvbox.h>

// qt includes
#include <QFrame>
#include <QHBoxLayout>

/**
 * Constructor.
 */
UMLViewDialog::UMLViewDialog(QWidget * pParent, UMLScene * pScene)
  : KPageDialog(pParent)
{
    setCaption( i18n("Properties") );
    setButtons( Ok | Apply | Cancel | Help );
    setDefaultButton( Ok );
    setModal( true );
    setFaceType( KPageDialog::List );
    showButtonSeparator( true );
    m_pScene = pScene;
    m_options = m_pScene->optionState();
    setupPages();
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

/**
 * Destructor.
 */
UMLViewDialog::~UMLViewDialog()
{
}

void UMLViewDialog::slotOk()
{
    applyPage( m_pageGeneralItem );
    applyPage( m_pageDisplayItem );
    applyPage( m_pageFontItem );
    applyPage( m_pageStyleItem );
    accept();
}

void UMLViewDialog::slotApply()
{
    applyPage( currentPage() );
}

/**
 * Sets up the dialog pages.
 */
void UMLViewDialog::setupPages()
{
    setupDiagramPropertiesPage();
    setupStylePage();
    setupFontPage();
    setupClassPage();
}

/**
 * Sets up the general Diagram Properties Page
 */
void UMLViewDialog::setupDiagramPropertiesPage()
{
    KVBox *page = new KVBox();
    m_pageGeneralItem = new KPageWidgetItem( page, i18nc("general settings page", "General") );
    m_pageGeneralItem->setHeader( i18n("General Settings") );
    m_pageGeneralItem->setIcon(Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_General) );
    addPage(m_pageGeneralItem);

    m_diagramProperties = new DiagramPropertiesPage(page);

    m_diagramProperties->ui_diagramName->setText( m_pScene->name() );
    m_diagramProperties->ui_zoom->setValue(m_pScene->activeView()->currentZoom());
    m_diagramProperties->ui_showOpSigs->setChecked( m_pScene->showOpSig() );

    m_diagramProperties->ui_checkBoxShowGrid->setChecked(m_pScene->isSnapGridVisible());
    m_diagramProperties->ui_snapToGrid->setChecked(m_pScene->snapToGrid());
    m_diagramProperties->ui_snapComponentSizeToGrid->setChecked(m_pScene->snapComponentSizeToGrid());

    m_diagramProperties->ui_gridSpaceX->setValue( m_pScene->snapX());
    m_diagramProperties->ui_gridSpaceY->setValue( m_pScene->snapY());
    m_diagramProperties->ui_lineWidth->setValue( m_pScene->lineWidth());
    m_diagramProperties->ui_documentation->setText(m_pScene->documentation());
}

/**
 * Sets up the Class page
 */
void UMLViewDialog::setupClassPage()
{
    if ( m_pScene->type() != Uml::DiagramType::Class ) {
        return;
    }
    QFrame * newPage = new QFrame();
    m_pageDisplayItem = new KPageWidgetItem( newPage, i18nc("classes display options page", "Display") );
    m_pageDisplayItem->setHeader( i18n("Classes Display Options") );
    m_pageDisplayItem->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Display) );
    addPage(m_pageDisplayItem);

    QHBoxLayout * pOptionsLayout = new QHBoxLayout( newPage );
    m_pOptionsPage = new ClassOptionsPage( newPage, &m_options );
    pOptionsLayout->addWidget( m_pOptionsPage );
}

/**
 * Sets up the style page.
 */
void UMLViewDialog::setupStylePage()
{
    QFrame * stylePage = new QFrame();
    m_pageStyleItem = new KPageWidgetItem( stylePage, i18nc("diagram style page", "Style") );
    m_pageStyleItem->setHeader( i18n("Diagram Style") );
    m_pageStyleItem->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Color) );
    addPage(m_pageStyleItem);

    QHBoxLayout * m_pStyleLayout = new QHBoxLayout(stylePage);
    m_pStylePage = new UMLWidgetStylePage( stylePage, &m_options );
    m_pStyleLayout->addWidget(m_pStylePage);
}

/**
 * Sets up font page.
 */
void UMLViewDialog::setupFontPage()
{
    KVBox *page = new KVBox();
    m_pageFontItem = new KPageWidgetItem( page, i18n("Font") );
    m_pageFontItem->setHeader( i18n("Font Settings") );
    m_pageFontItem->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Font) );
    addPage(m_pageFontItem);

    m_pChooser = new KFontChooser( (QWidget*)page, KFontChooser::NoDisplayFlags, QStringList(), 0);
    m_pChooser->setFont( m_pScene->optionState().uiState.font );
}

/**
 * Applies the properties of the given page.
 */
void UMLViewDialog::applyPage(KPageWidgetItem *item)
{
    if ( item == m_pageGeneralItem )
    {
        checkName();
//:TODO:        m_pScene->setZoom( m_diagramProperties->ui_zoom->value() );
        m_pScene->setDocumentation( m_diagramProperties->ui_documentation->toPlainText() );
        m_pScene->setSnapSpacing( m_diagramProperties->ui_gridSpaceX->value(),
                                  m_diagramProperties->ui_gridSpaceY->value() );
        m_pScene->setLineWidth( m_diagramProperties->ui_lineWidth->value() );
        m_pScene->setSnapToGrid( m_diagramProperties->ui_snapToGrid->isChecked() );
        m_pScene->setSnapComponentSizeToGrid( m_diagramProperties->ui_snapComponentSizeToGrid->isChecked() );
        m_pScene->setSnapGridVisible( m_diagramProperties->ui_checkBoxShowGrid->isChecked() );
        m_pScene->setShowOpSig( m_diagramProperties->ui_showOpSigs->isChecked() );
    }
    else if ( item == m_pageStyleItem )
    {
        uDebug() << "setting colors ";
        m_pStylePage->updateUMLWidget();
        m_pScene->setLineWidth(m_options.uiState.lineWidth);
        m_pScene->setUseFillColor(m_options.uiState.useFillColor);
        m_pScene->setTextColor(m_options.uiState.textColor);
        m_pScene->setLineColor(m_options.uiState.lineColor);
        m_pScene->setBrush(m_options.uiState.fillColor);
        m_pScene->setBackgroundBrush(m_options.uiState.backgroundColor);
        m_pScene->setGridDotColor(m_options.uiState.gridDotColor);
        //:TODO: gridCrossColor, gridTextColor, gridTextFont, gridTextIsVisible
    }
    else if ( item == m_pageFontItem )
    {
        uDebug() << "setting font " << m_pChooser->font().toString();
        m_pScene->setFont( m_pChooser->font(), true );
    }
    else if ( item == m_pageDisplayItem )
    {
        if ( m_pScene->type() != Uml::DiagramType::Class ) {
            return;
        }
        m_pOptionsPage->apply();
        m_pScene->setClassWidgetOptions( m_pOptionsPage );
        //       sig = m_pTempWidget->getShowOpSigs();
        //       showSig = !( sig == Uml::st_NoSig || sig == Uml::st_NoSigNoVis );
        //       options.classState.showOpSig = showSig;
        //       sig = m_pTempWidget->getShowAttSigs();
        //       showSig = !( sig == Uml::st_NoSig || sig == Uml::st_NoSigNoVis );
        //       options.classState.showAttSig = showSig;
        m_pScene->setOptionState( m_options );
    }
}

/**
 * Checks whether the name is unique and sets it if it is.
 */
void UMLViewDialog::checkName()
{
    QString newName = m_diagramProperties->ui_diagramName->text();
    if ( newName.length() == 0 ) {
        KMessageBox::sorry(this, i18n("The name you have entered is invalid."),
                           i18n("Invalid Name"), 0);
        m_diagramProperties->ui_diagramName->setText( m_pScene->name() );
        return;
    }

    if (newName != m_pScene->name()) {
        UMLDoc* doc = UMLApp::app()->document();
        UMLView* view = doc->findView(m_pScene->type(), newName);
        if (view) {
            KMessageBox::sorry(this, i18n("The name you have entered is not unique."),
                               i18n("Name Not Unique"), 0);
            m_diagramProperties->ui_diagramName->setText( m_pScene->name() );
        }
        else {
            // uDebug() << "Cannot find view with name " << newName;
            m_pScene->setName( newName );
            doc->signalDiagramRenamed(m_pScene->activeView());
        }
    }
}

#include "umlviewdialog.moc"
