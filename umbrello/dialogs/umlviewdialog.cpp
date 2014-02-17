/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "umlviewdialog.h"

// local includes
#include "classoptionspage.h"
#include "diagrampropertiespage.h"
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
    setCaption(i18n("Properties"));
    setButtons(Ok | Apply | Cancel | Help);
    setDefaultButton(Ok);
    setModal(true);
    setFaceType(KPageDialog::List);
    showButtonSeparator(true);
    m_pScene = pScene;
    m_options = m_pScene->optionState();
    setupPages();
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
}

/**
 * Destructor.
 */
UMLViewDialog::~UMLViewDialog()
{
}

void UMLViewDialog::slotOk()
{
    applyPage(m_pageDiagramItem);
    applyPage(m_pageDisplayItem);
    applyPage(m_pageFontItem);
    applyPage(m_pageStyleItem);
    accept();
}

void UMLViewDialog::slotApply()
{
    applyPage(currentPage());
}

/**
 * Sets up the dialog pages.
 */
void UMLViewDialog::setupPages()
{
    setupDiagramPropertiesPage();
    setupStylePage();
    setupFontPage();
    setupDisplayPage();
}

/**
 * Sets up the general Diagram Properties Page
 */
void UMLViewDialog::setupDiagramPropertiesPage()
{
    KVBox *page = new KVBox();
    m_pageDiagramItem = new KPageWidgetItem(page, i18nc("general settings page", "General"));
    m_pageDiagramItem->setHeader(i18n("General Settings"));
    m_pageDiagramItem->setIcon(Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_General));
    addPage(m_pageDiagramItem);

    m_diagramPropertiesPage = new DiagramPropertiesPage(page, m_pScene);
}

/**
 * Sets up the display page
 */
void UMLViewDialog::setupDisplayPage()
{
    m_pageDisplayItem = 0;
    // Display page currently only shows class-related display options that are
    // applicable for class- and sequence diagram
    if (m_pScene->type() != Uml::DiagramType::Class &&
        m_pScene->type() != Uml::DiagramType::Sequence) {
        return;
    }


    QFrame * newPage = new QFrame();
    m_pageDisplayItem = new KPageWidgetItem(newPage, i18nc("classes display options page", "Display"));
    m_pageDisplayItem->setHeader(i18n("Classes Display Options"));
    m_pageDisplayItem->setIcon(Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Display));
    addPage(m_pageDisplayItem);

    QHBoxLayout * pOptionsLayout = new QHBoxLayout(newPage);
    if (m_pScene->type() != Uml::DiagramType::Class) {
        m_pOptionsPage = new ClassOptionsPage(newPage, m_pScene);
    }
    else {
        m_pOptionsPage = new ClassOptionsPage(newPage, &m_options);
    }

    pOptionsLayout->addWidget(m_pOptionsPage);
}

/**
 * Sets up the style page.
 */
void UMLViewDialog::setupStylePage()
{
    QFrame * stylePage = new QFrame();
    m_pageStyleItem = new KPageWidgetItem(stylePage, i18nc("diagram style page", "Style"));
    m_pageStyleItem->setHeader(i18n("Diagram Style"));
    m_pageStyleItem->setIcon(Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Color));
    addPage(m_pageStyleItem);

    QHBoxLayout * m_pStyleLayout = new QHBoxLayout(stylePage);
    m_pStylePage = new UMLWidgetStylePage(stylePage, &m_options);
    m_pStyleLayout->addWidget(m_pStylePage);
}

/**
 * Sets up font page.
 */
void UMLViewDialog::setupFontPage()
{
    KVBox *page = new KVBox();
    m_pageFontItem = new KPageWidgetItem(page, i18n("Font"));
    m_pageFontItem->setHeader(i18n("Font Settings"));
    m_pageFontItem->setIcon(Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Font));
    addPage(m_pageFontItem);

    m_pChooser = new KFontChooser((QWidget*)page, KFontChooser::NoDisplayFlags, QStringList(), 0);
    m_pChooser->setFont(m_pScene->optionState().uiState.font);
}

/**
 * Applies the properties of the given page.
 */
void UMLViewDialog::applyPage(KPageWidgetItem *item)
{
    if (item == 0) {
        // Page not loaded in this dialog
        return;
    }
    else if (item == m_pageDiagramItem)
    {
        m_diagramPropertiesPage->apply();
    }
    else if (item == m_pageStyleItem)
    {
        uDebug() << "setting colors ";
        m_pStylePage->updateUMLWidget();
        m_pScene->setLineWidth(m_options.uiState.lineWidth);
        m_pScene->setUseFillColor(m_options.uiState.useFillColor);
        m_pScene->setTextColor(m_options.uiState.textColor);
        m_pScene->setLineColor(m_options.uiState.lineColor);
        m_pScene->setFillColor(m_options.uiState.fillColor);
        m_pScene->setBackgroundBrush(m_options.uiState.backgroundColor);
        m_pScene->setGridDotColor(m_options.uiState.gridDotColor);
        //:TODO: gridCrossColor, gridTextColor, gridTextFont, gridTextIsVisible
    }
    else if (item == m_pageFontItem)
    {
        uDebug() << "setting font " << m_pChooser->font().toString();
        m_pScene->setFont(m_pChooser->font(), true);
    }
    else if (item == m_pageDisplayItem)
    {
        m_pOptionsPage->apply();
        if (m_pScene->type() != Uml::DiagramType::Class) {
            return;
        }
        m_pScene->setClassWidgetOptions(m_pOptionsPage);
        //       sig = m_pTempWidget->getShowOpSigs();
        //       showSig = !(sig == Uml::st_NoSig || sig == Uml::st_NoSigNoVis);
        //       options.classState.showOpSig = showSig;
        //       sig = m_pTempWidget->getShowAttSigs();
        //       showSig = !(sig == Uml::st_NoSig || sig == Uml::st_NoSigNoVis);
        //       options.classState.showAttSig = showSig;
        m_pScene->setOptionState(m_options);
    }
}

#include "umlviewdialog.moc"
