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
#include <qvbox.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qtextedit.h>
#include <qspinbox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfontdialog.h>
#include <kdebug.h>
#include <knuminput.h>

// local includes
#include "../umlview.h"
#include "../umldoc.h"
#include "../uml.h"
#include "diagrampropertiespage.h"


UMLViewDialog::UMLViewDialog( QWidget * pParent, UMLView * pView ) : KDialogBase(IconList, i18n("Properties"), Ok | Apply | Cancel | Help,
        Ok, pParent, "_VIEWDLG_", true, true) {
    m_pView = pView;
    m_options = m_pView -> getOptionState();
    setupPages();
}

UMLViewDialog::~UMLViewDialog() {
}

void UMLViewDialog::slotOk() {
    applyPage( General );
    applyPage( Color );
    applyPage( Font );
    applyPage( Class );
    accept();
}

void UMLViewDialog::slotApply() {
    applyPage( (Page)activePageIndex() );
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
    QVBox *page = addVBoxPage( i18n("General"), i18n("General Settings"), DesktopIcon( "misc") );
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

    QFrame * newPage = addPage( i18n("Display"), i18n("Classes Display Options"), DesktopIcon( "info") );
    QHBoxLayout * m_pOptionsLayout = new QHBoxLayout( newPage );
    m_pOptionsPage = new ClassOptionsPage( newPage, &m_options );
    m_pOptionsLayout -> addWidget( m_pOptionsPage );
}

void UMLViewDialog::setupColorPage() {
    QFrame * colorPage = addPage( i18n("Color"), i18n("Diagram Colors"), DesktopIcon( "colors") );
    QHBoxLayout * m_pColorLayout = new QHBoxLayout(colorPage);
    m_pColorPage = new UMLWidgetColorPage( colorPage, &m_options );
    m_pColorLayout -> addWidget(m_pColorPage);
}

void UMLViewDialog::setupFontPage() {
    QVBox * page = addVBoxPage( i18n("Font"), i18n("Font Settings"), DesktopIcon( "fonts")  );
    m_pChooser = new KFontChooser( (QWidget*)page, "font", false, QStringList(), false);
    m_pChooser -> setFont( m_pView -> getOptionState().uiState.font );
}

void UMLViewDialog::applyPage( Page page ) {

    switch (page) {
    case General:
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
            break;
        }
    case Color:
        m_pColorPage->updateUMLWidget();
        m_pView->setUseFillColor( m_options.uiState.useFillColor );
        m_pView->setLineColor( m_options.uiState.lineColor );
        m_pView->setFillColor( m_options.uiState.fillColor );
        break;

    case Font:
        kDebug() << "UMLViewDialog::applyPage: setting font "
        << m_pChooser->font().toString() << endl;
        m_pView->setFont( m_pChooser->font(), true );
        break;

    case Class:
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
        break;
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
