/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfontdialog.h>
#include <kdebug.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qtextedit.h>
#include <qspinbox.h>
#include <qvbox.h>

#include "umlviewdialog.h"
#include "../umlview.h"
#include "../classwidget.h"
#include "diagrampropertiespage.h"

UMLViewDialog::UMLViewDialog( QWidget * pParent, UMLView * pView ) : KDialogBase(IconList, i18n("Properties"), Ok | Apply | Cancel | Help,
        Ok, pParent, "_VIEWDLG_", true, true) {
	m_pView = pView;
	m_pTempWidget = new ClassWidget( m_pView );
	setupPages();
}

UMLViewDialog::~UMLViewDialog() {
	m_pTempWidget -> cleanup();
	delete m_pTempWidget;
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

	m_diagramProperties->showGrid->setChecked(m_pView -> getShowSnapGrid());
	m_diagramProperties->snapToGrid->setChecked(m_pView-> getSnapToGrid());
	m_diagramProperties->snapComponentSizeToGrid->setChecked(m_pView-> getSnapComponentSizeToGrid());

	m_diagramProperties->gridSpaceX->setValue( m_pView -> getSnapX());
	m_diagramProperties->gridSpaceY->setValue( m_pView -> getSnapY());
	m_diagramProperties->documentation->setText(m_pView -> getDoc());

}

void UMLViewDialog::setupClassPage() {
	if( m_pView -> getType() != Uml::dt_Class ) {
		return;
	}
	SettingsDlg::OptionState options = m_pView -> getOptionState();
	m_pTempWidget -> setShowAtts( options.classState.showAtts );
	m_pTempWidget -> setShowOps( options.classState.showOps );
	m_pTempWidget -> setShowPackage( options.classState.showPackage );
	m_pTempWidget -> setShowScope( options.classState.showScope );
	m_pTempWidget -> setShowStereotype( options.classState.showStereoType );
	m_pTempWidget -> setShowOpSigs( options.classState.showOpSig );
	m_pTempWidget -> setShowAttSigs( options.classState.showAttSig );
	QFrame * newPage = addPage( i18n("Display"), i18n("Classes Display Options"), DesktopIcon( "info") );
	QHBoxLayout * m_pOptionsLayout = new QHBoxLayout( newPage );
	m_pOptionsPage = new ClassOptionsPage( newPage, m_pTempWidget, m_pTempWidget->getBaseType() );
	m_pOptionsLayout -> addWidget( m_pOptionsPage );
}

void UMLViewDialog::setupColorPage() {
	QFrame * colorPage = addPage( i18n("Color"), i18n("Diagram Colors"), DesktopIcon( "colors") );
	QHBoxLayout * m_pColorLayout = new QHBoxLayout(colorPage);
	m_pColorPage = new UMLWidgetColorPage( colorPage, m_pTempWidget );
	m_pColorLayout -> addWidget(m_pColorPage);
}

void UMLViewDialog::setupFontPage() {
	QVBox * page = addVBoxPage( i18n("Font"), i18n("Font Settings"), DesktopIcon( "fonts")  );
	m_pChooser = new KFontChooser( (QWidget*)page, "font", false, QStringList(), false);
	m_pChooser -> setFont( m_pView -> getOptionState().uiState.font );
	m_pChooser->setSampleText(i18n("Umbrello font"));
}

void UMLViewDialog::applyPage( Page page ) {
	SettingsDlg::OptionState options = m_pView->getOptionState();
	Uml::Signature_Type sig;
	bool showSig = false;

	switch (page) {
		case General:
			{
			checkName();
			m_pView->setZoom( m_diagramProperties->zoom->value() );
			m_pView->setDoc( m_diagramProperties->documentation->text() );
			m_pView->setSnapX( m_diagramProperties->gridSpaceX->value() );
			m_pView->setSnapY( m_diagramProperties->gridSpaceY->value() );
			m_pView->setSnapToGrid( m_diagramProperties->snapToGrid->isChecked() );
			m_pView->setSnapComponentSizeToGrid( m_diagramProperties->snapComponentSizeToGrid->isChecked() );
			m_pView->setShowSnapGrid( m_diagramProperties->showGrid->isChecked() );
			break;
			}
		case Color:
			m_pColorPage->updateUMLWidget();
			m_pView->setUseFillColor( m_pTempWidget->getUseFillColor() );
			m_pView->setLineColor( m_pTempWidget->getLineColor() );
			m_pView->setFillColour( m_pTempWidget->getFillColour() );
			break;

		case Font:
			m_pView->setFont( m_pChooser->font() );
			break;

		case Class:
			if( m_pView->getType() != Uml::dt_Class ) {
				return;
			}
			m_pOptionsPage->updateUMLWidget();
			m_pView->setClassWidgetOptions( m_pOptionsPage );
			m_pOptionsPage->setWidget( m_pTempWidget );
			options.classState.showAtts = m_pTempWidget->getShowAtts();
			options.classState.showOps = m_pTempWidget->getShowOps();
			options.classState.showPackage = m_pTempWidget->getShowPackage();
			options.classState.showScope = m_pTempWidget->getShowScope();
			options.classState.showStereoType = m_pTempWidget->getShowStereotype();
			sig = m_pTempWidget->getShowOpSigs();
			showSig = !( sig == Uml::st_NoSig || sig == Uml::st_NoSigNoScope );
			options.classState.showOpSig = showSig;
			sig = m_pTempWidget->getShowAttSigs();
			showSig = !( sig == Uml::st_NoSig || sig == Uml::st_NoSigNoScope );
			options.classState.showAttSig = showSig;
			m_pView->setOptionState( options );
			break;
	}
}

void UMLViewDialog::checkName() {
	QString name = m_diagramProperties->diagramName-> text();
	UMLDoc * pDoc = m_pView -> getDocument();
	UMLView * pView = pDoc -> findView( m_pView -> getType(), name );
	if( name.length() == 0 ) {
		KMessageBox::sorry(this, i18n("The name you have entered is invalid."),
		                   i18n("Invalid Name."), false);
		m_diagramProperties->diagramName->setText( m_pView->getName() );
		return;
	}
	if( pView && pView != m_pView ) {
		KMessageBox::sorry(this, i18n("The name you have entered is not unique."),
		                   i18n("Name Not Unique."), false);
		m_diagramProperties->diagramName->setText( m_pView->getName() );
		return;
	}
	m_pView->setName( name );
}






#include "umlviewdialog.moc"
