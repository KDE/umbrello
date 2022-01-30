/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
#include <kfontchooser.h>
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QFrame>
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
    m_pageFontItem = setupFontPage(m_pScene->optionState().uiState.font);
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

    m_pOptionsPage = new ClassOptionsPage(0, m_pScene);
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
        logDebug0("UMLViewDialog::applyPage setting colors");
        m_pStylePage->apply();
    }
    else if (item == m_pageFontItem)
    {
        applyFontPage(m_pScene);
    }
    else if (item == m_pageDisplayItem)
    {
        m_pOptionsPage->apply();
    }
}
