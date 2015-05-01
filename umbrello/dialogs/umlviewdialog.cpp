/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
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
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QFrame>
#include <QFontDialog>
#include <QHBoxLayout>

DEBUG_REGISTER(UMLViewDialog)

/**
 * Constructor.
 */
UMLViewDialog::UMLViewDialog(QWidget * pParent, UMLScene * pScene)
  : MultiPageDialogBase(pParent),
    m_pOptionsPage(0)
{
    setCaption(i18n("Properties"));
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

void UMLViewDialog::apply()
{
    slotOk();
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
    m_diagramPropertiesPage = new DiagramPropertiesPage(0, m_pScene);
    m_pageDiagramItem = createPage(i18nc("general settings page", "General"), i18n("General Settings"),
                                   Icon_Utils::it_Properties_General, m_diagramPropertiesPage);
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

    if (m_pScene->type() != Uml::DiagramType::Class) {
        m_pOptionsPage = new ClassOptionsPage(0, m_pScene);
    }
    else {
        m_pOptionsPage = new ClassOptionsPage(0, &m_options);
    }
    m_pageDisplayItem = createPage(i18nc("classes display options page", "Display"), i18n("Classes Display Options"),
                                   Icon_Utils::it_Properties_Display, m_pOptionsPage);
}

/**
 * Sets up the style page.
 */
void UMLViewDialog::setupStylePage()
{
    m_pStylePage = new UMLWidgetStylePage(0, m_pScene);
    m_pageStyleItem = createPage(i18nc("diagram style page", "Style"), i18n("Diagram Style"),
                                 Icon_Utils::it_Properties_Color, m_pStylePage);
}

/**
 * Sets up font page.
 */
void UMLViewDialog::setupFontPage()
{
    m_pChooser = new QFontDialog();
    m_pChooser->setCurrentFont(m_pScene->optionState().uiState.font);
    m_pChooser->setOption(QFontDialog::NoButtons);
    m_pageFontItem = createPage(i18n("Font"), i18n("Font Settings"),
                                Icon_Utils::it_Properties_Font, m_pChooser);
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
        m_pStylePage->apply();
    }
    else if (item == m_pageFontItem)
    {
        uDebug() << "setting font " << m_pChooser->currentFont().toString();
        m_pScene->setFont(m_pChooser->currentFont(), true);
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
