/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "associationpropertiesdialog.h"

// local includes
#include "associationwidget.h"
#include "debug_utils.h"
#include "icon_utils.h"
#include "objectwidget.h"
#include "uml.h"
#include "umlobject.h"
#include "umlview.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>
#if QT_VERSION < 0x050000
#include <kvbox.h>
#endif

// qt includes
#include <QFrame>
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
  : MultiPageDialogBase(parent),
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
    MultiPageDialogBase::apply();

    if (m_pAssoc) {
        applyFontPage(m_pAssoc);
    }
}

void AssociationPropertiesDialog::setupPages()
{
    setupGeneralPage(m_pAssoc);
    setupAssociationRolePage(m_pAssoc);
    setupStylePage(m_pAssoc);
    setupFontPage(m_pAssoc);
}

