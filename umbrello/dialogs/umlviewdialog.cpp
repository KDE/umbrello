/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "umlviewdialog.h"
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qtextedit.h>
#include <qspinbox.h>
//kde includes
#include <kfontdialog.h>


UMLViewDialog::UMLViewDialog( QWidget * pParent, UMLView * pView ) : KDialogBase(IconList, i18n("Properties"), Ok | Apply | Cancel | Help,
        Ok, pParent, "_VIEWDLG_", true, true) {
	m_pView = pView;
	m_pTempWidget = new ConceptWidget( m_pView );
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

void UMLViewDialog::setupPages() {
	//setup the general page
	QFrame *page = addPage( i18n("General"), i18n("General settings"), DesktopIcon( "misc") );
	QVBoxLayout * topLayout = new QVBoxLayout(page);
	m_pValuesGB = new QGroupBox( i18n( "Diagram settings" ), page );

	QGridLayout * valueLayout = new QGridLayout( m_pValuesGB, 3, 4 );
	valueLayout -> setSpacing( 10 );
	valueLayout -> setMargin( fontMetrics().height() );

	m_pNameL = new QLabel( i18n( "Name" ), m_pValuesGB);
	valueLayout -> addWidget( m_pNameL, 0, 0 );
	valueLayout -> setColStretch( 0, 1);

	m_pNameLE = new QLineEdit( m_pView->getName(), m_pValuesGB );
	valueLayout -> addWidget( m_pNameLE, 0, 2 );
	valueLayout -> setColStretch( 2, 1);

	m_pShowSnapCB = new QCheckBox( i18n( "Show grid" ), m_pValuesGB );
	m_pShowSnapCB -> setChecked( m_pView -> getShowSnapGrid() );
	valueLayout -> addWidget( m_pShowSnapCB, 1, 0 );

	m_pSnapToGridCB = new QCheckBox( i18n( "Snap to grid" ), m_pValuesGB );
	m_pSnapToGridCB -> setChecked( m_pView -> getSnapToGrid() );
	valueLayout -> addWidget( m_pSnapToGridCB, 1, 2 );

	m_pSpinXL = new QLabel( i18n( "X value" ), m_pValuesGB );
	valueLayout -> addWidget( m_pSpinXL, 2, 0 );

	m_pSnapXSB = new QSpinBox( 2, 50, 1, m_pValuesGB );
	m_pSnapXSB -> setValue( m_pView -> getSnapX() );
	valueLayout -> addWidget( m_pSnapXSB, 2, 1 );

	m_pSpinYL = new QLabel( i18n( "Y value" ), m_pValuesGB );
	valueLayout -> addWidget( m_pSpinYL, 2, 2 );

	m_pSnapYSB = new QSpinBox( 2, 50, 1, m_pValuesGB );
	m_pSnapYSB -> setValue( m_pView -> getSnapY() );
	valueLayout -> addWidget( m_pSnapYSB, 2, 3 );

	topLayout -> addWidget( m_pValuesGB );

	m_pDocGB = new QGroupBox( i18n( "Documentation" ), page );
	QHBoxLayout * docLayout = new QHBoxLayout( m_pDocGB );
	docLayout -> setSpacing( 10 );
	docLayout -> setMargin( fontMetrics().height() );
	m_pDocTE = new QTextEdit( m_pDocGB );
	m_pDocTE -> setText( m_pView -> getDoc() );
	docLayout -> addWidget( m_pDocTE );
	topLayout -> addWidget( m_pDocGB );
	//setup the rest of the pages
	setupColorPage();
	setupFontPage();
	setupClassPage();
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
	m_pOptionsPage = new ClassOptionsPage( newPage, m_pTempWidget );
	m_pOptionsLayout -> addWidget( m_pOptionsPage );
}

void UMLViewDialog::setupColorPage() {
	QFrame * colorPage = addPage( i18n("Colour"), i18n("Diagram Colours"), DesktopIcon( "colors") );
	QHBoxLayout * m_pColorLayout = new QHBoxLayout(colorPage);
	m_pColorPage = new UMLWidgetColorPage( colorPage, m_pTempWidget );
	m_pColorLayout -> addWidget(m_pColorPage);
}

void UMLViewDialog::setupFontPage() {
	QVBox * page = addVBoxPage( i18n("Font"), i18n("Font settings"), DesktopIcon( "fonts")  );
	m_pChooser = new KFontChooser( (QWidget*)page, "font", false, QStringList(), false);
	m_pChooser -> setFont( m_pView -> getOptionState().uiState.font );
	m_pChooser->setSampleText("Umbrello font");
}

void UMLViewDialog::applyPage( Page page ) {
	SettingsDlg::OptionState options = m_pView->getOptionState();
	Uml::Signature_Type sig;
	bool showSig = false;

	switch (page) {
		case General:
			checkName();
			m_pView -> setDoc( m_pDocTE->text() );
			m_pView -> setSnapX( m_pSnapXSB->value() );
			m_pView -> setSnapY( m_pSnapYSB->value() );
			m_pView -> setSnapToGrid( m_pSnapToGridCB->isChecked() );
			m_pView -> setShowSnapGrid( m_pShowSnapCB->isChecked() );
			break;

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
			m_pView->setConceptWidgetOptions( m_pOptionsPage );
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
	QString name = m_pNameLE -> text();
	UMLDoc * pDoc = m_pView -> getDocument();
	UMLView * pView = pDoc -> findView( m_pView -> getType(), name );
	if( name.length() == 0 ) {
		KMessageBox::sorry(this, i18n("The name you have entered is invalid."),
		                   i18n("Invalid name."), false);
		m_pNameLE->setText( m_pView->getName() );
		return;
	}
	if( pView && pView != m_pView ) {
		KMessageBox::sorry(this, i18n("The name you have entered is not unique."),
		                   i18n("Name not unique."), false);
		m_pNameLE->setText( m_pView->getName() );
		return;
	}
	m_pView->setName( name );
}






#include "umlviewdialog.moc"
