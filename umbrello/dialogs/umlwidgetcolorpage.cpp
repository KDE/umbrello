/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "umlwidgetcolorpage.h"
#include "../uml.h"
#include "../umlwidget.h"
#include <klocale.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include <kcolorbutton.h>

UMLWidgetColorPage::UMLWidgetColorPage( QWidget * pParent, UMLWidget * pWidget ) : QWidget( pParent ) {
	m_pUMLWidget = pWidget;
	int margin = fontMetrics().height();

	//setup GUI
	QVBoxLayout * topLayout = new QVBoxLayout( this );
	topLayout -> setSpacing( 6 );

	m_pColorGB = new QGroupBox( i18n( "Color" ), this );
	topLayout -> addWidget( m_pColorGB );
	QGridLayout * colorLayout = new QGridLayout( m_pColorGB, 3, 3 );
	colorLayout -> setMargin( margin );

	m_pLineColorL = new QLabel( i18n( "Line:" ), m_pColorGB );
	colorLayout -> addWidget( m_pLineColorL, 0, 0 );

	m_pLineColorB = new KColorButton( pWidget -> getLineColor(), m_pColorGB );
	colorLayout -> addWidget( m_pLineColorB, 0, 1 );

	m_pLineDefaultB = new QPushButton( i18n( "&Default" ), m_pColorGB) ;
	colorLayout -> addWidget( m_pLineDefaultB, 0, 2 );

	m_pFillColorL = new QLabel( i18n( "Fill:" ), m_pColorGB );
	colorLayout -> addWidget( m_pFillColorL, 1, 0 );

	m_pFillColorB = new KColorButton( pWidget->getFillColour(), m_pColorGB );
	colorLayout -> addWidget( m_pFillColorB, 1, 1 );

	m_pFillDefaultB = new QPushButton( i18n( "D&efault" ), m_pColorGB );
	colorLayout -> addWidget( m_pFillDefaultB, 1, 2 );

	m_pUseFillColorCB = new QCheckBox( i18n( "&Use fill" ), m_pColorGB );
	colorLayout -> setRowStretch( 2, 2 );
	colorLayout -> addWidget( m_pUseFillColorCB, 2, 0 );
	m_pUseFillColorCB -> setChecked( pWidget -> getUseFillColor() );

	//connect button signals up
	connect( m_pLineDefaultB, SIGNAL( clicked() ), this, SLOT( slotLineButtonClicked() )) ;
	connect( m_pFillDefaultB, SIGNAL( clicked() ), this, SLOT( slotFillButtonClicked() ) );
}

UMLWidgetColorPage::~UMLWidgetColorPage() {}

void UMLWidgetColorPage::slotLineButtonClicked() {
	UMLView * pView = dynamic_cast<UMLView *>( m_pUMLWidget -> parent() );
	UMLApp * pApp = dynamic_cast<UMLApp *>( pView -> getDocument() -> parent() );
	m_pLineColorB -> setColor( pApp -> getOptionState().uiState.lineColor );
}

void UMLWidgetColorPage::slotFillButtonClicked() {
	UMLView * pView = dynamic_cast<UMLView *>( m_pUMLWidget -> parent() );
	UMLApp * pApp = dynamic_cast<UMLApp *>( pView -> getDocument() -> parent() );
	m_pFillColorB -> setColor( pApp -> getOptionState().uiState.fillColor );
}

void UMLWidgetColorPage::updateUMLWidget() {
	m_pUMLWidget->setUseFillColor( m_pUseFillColorCB -> isChecked() );
	m_pUMLWidget->setLineColour( m_pLineColorB -> color() );
	m_pUMLWidget->setFillColour( m_pFillColorB -> color() );
}

#include "umlwidgetcolorpage.moc"
