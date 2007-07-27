/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "assocpropdlg.h"

// qt/kde includes
#include <qlayout.h>
#include <qlabel.h>
#include <QFrame>
#include <QHBoxLayout>
#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kvbox.h>

// local includes
#include "assocgenpage.h"
#include "assocrolepage.h"
#include "classpropdlg.h"
#include "classgenpage.h"
#include "umlwidgetcolorpage.h"

#include "../umlobject.h"
#include "../umldoc.h"
#include "../objectwidget.h"
#include "../uml.h"
#include "../umlview.h"


AssocPropDlg::AssocPropDlg (QWidget *parent, AssociationWidget * assocWidget, int /*pageNum*/)
        : KPageDialog(parent)
{
    setCaption( i18n("Association Properties") );
    setButtons( Ok | Apply | Cancel | Help );
    setDefaultButton( Ok );
    setModal( true );
    setFaceType( KPageDialog::List );
    showButtonSeparator( true );
    init();
    m_pAssoc = assocWidget;

    m_pDoc = ((UMLApp *)parent) -> getDocument(); // needed?

    setupPages(assocWidget);
#ifdef __GNUC__
#warning "kde4: port it"
#endif
    //showPage(pageNum);
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

AssocPropDlg::~AssocPropDlg() { }

void AssocPropDlg::init ( )
{
    m_pAssoc = 0;
    m_pGenPage = 0;
    m_pRolePage = 0;
}

void AssocPropDlg::slotOk() {
    slotApply();
    KDialog::accept();
}

void AssocPropDlg::slotApply() {

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

    // general page
    QFrame *page = new QFrame();
    KPageWidgetItem *pageItem = new KPageWidgetItem( page, i18n("General"));
    pageItem->setHeader( i18n("General Settings") );
    pageItem->setIcon( KIcon(DesktopIcon( "misc") ));
    addPage( pageItem );
    QHBoxLayout *genLayout = new QHBoxLayout(page);
    page -> setMinimumSize(310, 330);
    m_pGenPage = new AssocGenPage (m_pDoc, page, assocWidget);
    genLayout -> addWidget(m_pGenPage);

    // role page
    QFrame *newPage = new QFrame();
    pageItem = new KPageWidgetItem( newPage, i18n("Roles"));
    pageItem->setHeader( i18n("Role Settings"));
    pageItem->setIcon( KIcon(DesktopIcon( "misc") ));
    addPage( pageItem );
    QHBoxLayout * roleLayout = new QHBoxLayout(newPage);
    // newPage -> setMinimumSize(310, 330);
    m_pRolePage = new AssocRolePage(m_pDoc, newPage, assocWidget);
    roleLayout -> addWidget(m_pRolePage);

    setupFontPage();

}

void AssocPropDlg::setupFontPage()
{
    if( !m_pAssoc)
        return;

    KVBox *page = new KVBox();
    KPageWidgetItem* pageItem = new KPageWidgetItem( page, i18n("Font"));
    pageItem->setHeader( i18n("Font Settings"));
    pageItem->setIcon( KIcon(DesktopIcon( "fonts") ));
    addPage( pageItem );

    m_pChooser = new KFontChooser( (QWidget*)page, false, QStringList(), false);
    m_pChooser->setFont( m_pAssoc->getFont());
    m_pChooser->setSampleText(i18n("Association font"));
}

#include "assocpropdlg.moc"
