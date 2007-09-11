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
#include "umlviewdialog.h"

// qt/kde includes
#include <QFrame>
#include <QHBoxLayout>
#include <q3vbox.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <q3groupbox.h>
#include <q3textedit.h>
#include <qspinbox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfontdialog.h>
#include <kdebug.h>
#include <kicon.h>
#include <knuminput.h>
#include <kvbox.h>

// local includes
#include "../umlview.h"
#include "../umldoc.h"
#include "../uml.h"


UMLViewDialog::UMLViewDialog( QWidget * pParent, UMLView * pView )
    : KPageDialog(pParent) {
    setCaption( i18n("Properties") );
    setButtons( Ok | Apply | Cancel | Help );
    setDefaultButton( Ok );
    setModal( true );
    setFaceType( KPageDialog::List );
    showButtonSeparator( true );
    m_pView = pView;
    m_options = m_pView -> getOptionState();
    setupPages();
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));

}

UMLViewDialog::~UMLViewDialog() {
}

void UMLViewDialog::slotOk() {

    applyPage( pageGeneralItem );
    applyPage( pageDisplayItem );
    applyPage( pageFontItem );
    applyPage( pageColorItem );
    accept();
}

void UMLViewDialog::slotApply() {
    applyPage(currentPage() );
}

void UMLViewDialog::setupPages()
{
    setupDiagramPropertiesPage();
    setupColorPage();
    setupFontPage();
    setupClassPage();
}

void UMLViewDialog::setupDiagramPropertiesPage()
{
    KVBox *page = new KVBox();
    pageGeneralItem = new KPageWidgetItem( page, i18n("General") );
    pageGeneralItem->setHeader( i18n("General Settings") );
    pageGeneralItem->setIcon(KIcon(DesktopIcon( "misc") ));
    addPage( pageGeneralItem);

    m_diagramProperties = new DiagramPropertiesPage(page);

    m_diagramProperties->diagramName->setText( m_pView->getName() );
    m_diagramProperties->zoom->setValue(m_pView->currentZoom());
    m_diagramProperties->showOpSigs->setChecked( m_pView->getShowOpSig() );

    m_diagramProperties->showGrid->setChecked(m_pView -> getShowSnapGrid());
    m_diagramProperties->snapToGrid->setChecked(m_pView-> getSnapToGrid());
    m_diagramProperties->snapComponentSizeToGrid->setChecked(m_pView-> getSnapComponentSizeToGrid());

    m_diagramProperties->gridSpaceX->setValue( m_pView -> getSnapX());
    m_diagramProperties->gridSpaceY->setValue( m_pView -> getSnapY());
    m_diagramProperties->lineWidth->setValue( m_pView -> getLineWidth());
    m_diagramProperties->documentation->setText(m_pView -> getDoc());

}

void UMLViewDialog::setupClassPage() {
    if( m_pView -> getType() != Uml::dt_Class ) {
        return;
    }
    QFrame * newPage = new QFrame();
    pageDisplayItem = new KPageWidgetItem( newPage,i18n("Display") );
    pageDisplayItem->setHeader( i18n("Classes Display Options") );
    pageDisplayItem->setIcon( KIcon( DesktopIcon( "document-properties") ));
    addPage( pageDisplayItem);

    QHBoxLayout * pOptionsLayout = new QHBoxLayout( newPage );
    m_pOptionsPage = new ClassOptionsPage( newPage, &m_options );
    pOptionsLayout -> addWidget( m_pOptionsPage );
}

void UMLViewDialog::setupColorPage() {
    QFrame * colorPage = new QFrame();
    pageColorItem = new KPageWidgetItem( colorPage, i18n("Color") );
    pageColorItem->setHeader( i18n("Diagram Colors") );
    pageColorItem->setIcon(KIcon(  DesktopIcon( "colors")) );
    addPage( pageColorItem);

    QHBoxLayout * m_pColorLayout = new QHBoxLayout(colorPage);
    m_pColorPage = new UMLWidgetColorPage( colorPage, &m_options );
    m_pColorLayout -> addWidget(m_pColorPage);
}

void UMLViewDialog::setupFontPage() {
    KVBox *page = new KVBox();
    pageFontItem = new KPageWidgetItem( page, i18n("Font") );
    pageFontItem->setHeader( i18n("Font Settings") );
    pageFontItem->setIcon(KIcon(  DesktopIcon( "fonts") ));
    addPage( pageFontItem);

    m_pChooser = new KFontChooser( (QWidget*)page, false, QStringList(), false);
    m_pChooser -> setFont( m_pView -> getOptionState().uiState.font );
}

void UMLViewDialog::applyPage( KPageWidgetItem*item ) {

    if ( item==pageGeneralItem )
    {
        checkName();
        m_pView->setZoom( m_diagramProperties->zoom->value() );
        m_pView->setDoc( m_diagramProperties->documentation->text() );
        m_pView->setSnapX( m_diagramProperties->gridSpaceX->value() );
        m_pView->setSnapY( m_diagramProperties->gridSpaceY->value() );
        m_pView->setLineWidth( m_diagramProperties->lineWidth->value() );
        m_pView->setSnapToGrid( m_diagramProperties->snapToGrid->isChecked() );
        m_pView->setSnapComponentSizeToGrid( m_diagramProperties->snapComponentSizeToGrid->isChecked() );
        m_pView->setShowSnapGrid( m_diagramProperties->showGrid->isChecked() );
        m_pView->setShowOpSig( m_diagramProperties->showOpSigs->isChecked() );
    }
    else if ( item == pageColorItem )
    {
        m_pColorPage->updateUMLWidget();
        m_pView->setUseFillColor( m_options.uiState.useFillColor );
        m_pView->setLineColor( m_options.uiState.lineColor );
        m_pView->setFillColor( m_options.uiState.fillColor );
    }
    else if ( item == pageFontItem )
    {
        uDebug() << "setting font " << m_pChooser->font().toString() << endl;
        m_pView->setFont( m_pChooser->font(), true );
    }
    else if ( item == pageDisplayItem )
    {
        if( m_pView->getType() != Uml::dt_Class ) {
            return;
        }
        m_pOptionsPage->updateUMLWidget();
        m_pView->setClassWidgetOptions( m_pOptionsPage );
        //                      sig = m_pTempWidget->getShowOpSigs();
        //                      showSig = !( sig == Uml::st_NoSig || sig == Uml::st_NoSigNoVis );
        //                      options.classState.showOpSig = showSig;
        //                      sig = m_pTempWidget->getShowAttSigs();
        //                      showSig = !( sig == Uml::st_NoSig || sig == Uml::st_NoSigNoVis );
        //                      options.classState.showAttSig = showSig;
        m_pView->setOptionState( m_options );
    }
}

void UMLViewDialog::checkName() {
    QString name = m_diagramProperties->diagramName-> text();
    UMLDoc * pDoc = UMLApp::app()-> getDocument();
    UMLView * pView = pDoc -> findView( m_pView -> getType(), name );
    if( name.length() == 0 ) {
        KMessageBox::sorry(this, i18n("The name you have entered is invalid."),
                           i18n("Invalid Name"), false);
        m_diagramProperties->diagramName->setText( m_pView->getName() );
        return;
    }
    if( pView && pView != m_pView ) {
        KMessageBox::sorry(this, i18n("The name you have entered is not unique."),
                           i18n("Name Not Unique"), false);
        m_diagramProperties->diagramName->setText( m_pView->getName() );
        return;
    }
    m_pView->setName( name );
    pDoc->signalDiagramRenamed(m_pView);
}






#include "umlviewdialog.moc"
