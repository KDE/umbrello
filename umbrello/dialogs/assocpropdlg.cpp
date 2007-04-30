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

#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kdebug.h>

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


AssocPropDlg::AssocPropDlg (QWidget *parent, AssociationWidget * assocWidget, int pageNum)
        : KDialogBase(IconList, i18n("Association Properties"), Ok | Apply | Cancel | Help,
                      Ok, parent, "_ASSOCPROPDLG_", true, true)
{
    init();
    m_pAssoc = assocWidget;

    m_pDoc = ((UMLApp *)parent) -> getDocument(); // needed?

    setupPages(assocWidget);
    showPage(pageNum);
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
    KDialogBase::accept();
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
    QFrame *page = addPage( i18n("General"), i18n("General Settings"), DesktopIcon( "misc") );
    QHBoxLayout *genLayout = new QHBoxLayout(page);
    page -> setMinimumSize(310, 330);
    m_pGenPage = new AssocGenPage (m_pDoc, page, assocWidget);
    genLayout -> addWidget(m_pGenPage);

    // role page
    QFrame * newPage = addPage( i18n("Roles"), i18n("Role Settings"), DesktopIcon( "misc") );
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

    QVBox *page = addVBoxPage( i18n("Font"), i18n("Font Settings"), DesktopIcon( "fonts"));
    m_pChooser = new KFontChooser( (QWidget*)page, "font", false, QStringList(), false);
    m_pChooser->setFont( m_pAssoc->getFont());
    m_pChooser->setSampleText(i18n("Association font"));
}

#include "assocpropdlg.moc"
