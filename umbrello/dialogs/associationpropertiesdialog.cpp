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
#include "associationpropertiesdialog.h"

// local includes
#include "associationwidget.h"
#include "associationgeneralpage.h"
#include "associationrolepage.h"
#include "classgeneralpage.h"
#include "classpropertiesdialog.h"
#include "debug_utils.h"
#include "icon_utils.h"
#include "objectwidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlview.h"
#include "umlwidgetstylepage.h"

// kde includes
#include <KLocalizedString>
#include <kmessagebox.h>

// qt includes
#include <QFrame>
#include <QFontDialog>
#include <QLabel>
#include <QLayout>
#include <QHBoxLayout>

/**
 *  Sets up an Association Properties Dialog.
 *  @param  parent  The parent of the AssociationPropertiesDialog
 *  @param  assocWidget  The Association Widget to display properties of.
 *  @param  pageNum The page to show first.
 */
AssociationPropertiesDialog::AssociationPropertiesDialog (QWidget *parent, AssociationWidget * assocWidget, int pageNum)
  : DialogBase(parent),
    m_pGenPage(0),
    m_pRolePage(0),
    m_pAssoc(assocWidget)
{
    Q_UNUSED(pageNum)
    setCaption(i18n("Association Properties"));
    setupPages();

    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
}

/**
 *  Standard destructor.
 */
AssociationPropertiesDialog::~AssociationPropertiesDialog()
{
}

void AssociationPropertiesDialog::slotOk()
{
    slotApply();
    accept();
}

void AssociationPropertiesDialog::slotApply()
{
    if (m_pGenPage) {
        m_pGenPage->updateObject();
    }

    if (m_pRolePage) {
        m_pRolePage->updateObject();
    }

    if (m_pStylePage) {
        m_pStylePage->updateUMLWidget();
    }

    if (m_pAssoc) {
        uDebug() << "setting font " << m_pChooser->currentFont().toString();
        m_pAssoc->lwSetFont(m_pChooser->currentFont());
    }
}

void AssociationPropertiesDialog::setupPages()
{
    UMLDoc* umlDoc = UMLApp::app()->document();

    // general page
    QFrame *page = createPage(i18nc("general settings", "General"), i18n("General Settings"), Icon_Utils::it_Properties_General);
    QHBoxLayout *layout = new QHBoxLayout(page);
    m_pGenPage = new AssociationGeneralPage (umlDoc, page, m_pAssoc);
    layout->addWidget(m_pGenPage);

    // role page
    page = createPage(i18nc("role page name", "Roles"), i18n("Role Settings"), Icon_Utils::it_Properties_Roles);
    layout = new QHBoxLayout(page);
    m_pRolePage = new AssociationRolePage(umlDoc, page, m_pAssoc),
    layout->addWidget(m_pRolePage);

    // style page
    page = createPage(i18nc("style page name", "Style"), i18n("Role Style"), Icon_Utils::it_Properties_Color);
    layout = new QHBoxLayout(page);
    m_pStylePage = new UMLWidgetStylePage(page, m_pAssoc);
    layout->addWidget(m_pStylePage);

    // font page
    page = createPage(i18nc("font page name", "Font"), i18n("Font Settings"), Icon_Utils::it_Properties_Font);
    layout = new QHBoxLayout(page);
    m_pChooser = new QFontDialog();
    m_pChooser->setCurrentFont(m_pAssoc->font());
    m_pChooser->setOption(QFontDialog::NoButtons);
    layout->addWidget(m_pChooser);
}
