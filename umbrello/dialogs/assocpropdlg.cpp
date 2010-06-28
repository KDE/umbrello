/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "assocpropdlg.h"

// local includes
#include "assocgenpage.h"
#include "assocrolepage.h"
#include "classpropdlg.h"
#include "classgenpage.h"
#include "umlwidgetcolorpage.h"
#include "umlobject.h"
#include "umldoc.h"
#include "objectwidget.h"
#include "uml.h"
#include "umlview.h"
#include "icon_utils.h"

// kde includes
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kvbox.h>

// qt includes
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>

AssocPropDlg::AssocPropDlg (QWidget *parent, AssociationWidget * assocWidget, int pageNum)
        : KPageDialog(parent)
{
    Q_UNUSED(pageNum);
    setCaption( i18n("Association Properties") );
    setButtons( Ok | Apply | Cancel | Help );
    setDefaultButton( Ok );
    setModal( true );
    setFaceType( KPageDialog::List );
    showButtonSeparator( true );

    m_pGenPage = 0;
    m_pRolePage = 0;
    m_pAssoc = assocWidget;

    setupPages(assocWidget);

    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

AssocPropDlg::~AssocPropDlg()
{
}

void AssocPropDlg::slotOk()
{
    slotApply();
    KDialog::accept();
}

void AssocPropDlg::slotApply()
{
    if (m_pGenPage) {
        m_pGenPage->updateObject();
    }

    if (m_pRolePage) {
        m_pRolePage->updateObject();
    }

    if (m_pAssoc) {
        m_pAssoc->lwSetFont( m_pChooser->font() );
    }
}

// void AssocPropDlg::setupPages (UMLObject * c)
void AssocPropDlg::setupPages (AssociationWidget *assocWidget)
{
    UMLDoc* umlDoc = UMLApp::app()->document();

    // general page
    QFrame *page = new QFrame();
    KPageWidgetItem *pageItem = new KPageWidgetItem( page, i18nc("general settings", "General"));
    pageItem->setHeader( i18n("General Settings") );
    pageItem->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_General) );
    addPage( pageItem );
    QHBoxLayout *genLayout = new QHBoxLayout(page);
    page->setMinimumSize(310, 330);
    m_pGenPage = new AssocGenPage (umlDoc, page, assocWidget);
    genLayout->addWidget(m_pGenPage);

    // role page
    QFrame *newPage = new QFrame();
    pageItem = new KPageWidgetItem( newPage, i18n("Roles"));
    pageItem->setHeader( i18n("Role Settings"));
    pageItem->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Roles) );
    addPage( pageItem );
    QHBoxLayout * roleLayout = new QHBoxLayout(newPage);
    // newPage->setMinimumSize(310, 330);
    m_pRolePage = new AssocRolePage(umlDoc, newPage, assocWidget);
    roleLayout->addWidget(m_pRolePage);

    setupFontPage();
}

void AssocPropDlg::setupFontPage()
{
    if ( !m_pAssoc) {
        return;
    }

    KVBox *page = new KVBox();
    KPageWidgetItem* pageItem = new KPageWidgetItem( page, i18n("Font"));
    pageItem->setHeader( i18n("Font Settings"));
    pageItem->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Font) );
    addPage( pageItem );

    m_pChooser = new KFontChooser( (QWidget*)page, false, QStringList(), false);
    m_pChooser->setFont( m_pAssoc->getFont());
    m_pChooser->setSampleText(i18n("Association font"));
}

#include "assocpropdlg.moc"
