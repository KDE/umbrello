/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "umlwidgetcolorpage.h"
#include "../optionstate.h"
#include "../umlview.h"
#include "../umlwidget.h"
#include <klocale.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include <kcolorbutton.h>

UMLWidgetColorPage::UMLWidgetColorPage( QWidget *pParent, UMLWidget *pWidget ) : QWidget( pParent )
{
    m_pUMLWidget = pWidget;
    m_options = 0;
    init();
    m_pLineColorB->setColor( pWidget->getLineColor() );
    m_pFillColorB->setColor( pWidget->getFillColour() );
    m_pUseFillColorCB->setChecked( pWidget -> getUseFillColour() );
}

UMLWidgetColorPage::UMLWidgetColorPage( QWidget * pParent, Settings::OptionState *options ) : QWidget( pParent )
{
    m_options = options;
    m_pUMLWidget = 0;
    init();
    m_pLineColorB->setColor( m_options->uiState.lineColor );
    m_pFillColorB->setColor( m_options->uiState.fillColor );
    m_pUseFillColorCB->setChecked( m_options->uiState.useFillColor );
}

void UMLWidgetColorPage::init()
{
    int margin = fontMetrics().height();

    //setup GUI
    QVBoxLayout * topLayout = new QVBoxLayout( this );
    topLayout -> setSpacing( 6 );

    m_pColorGB = new QGroupBox( i18n( "Color" ), this );
    topLayout -> addWidget( m_pColorGB );
    QGridLayout * colorLayout = new QGridLayout( m_pColorGB, 3, 3 );
    colorLayout -> setMargin( margin );

    m_pLineColorL = new QLabel( i18n( "&Line:" ), m_pColorGB );
    colorLayout -> addWidget( m_pLineColorL, 0, 0 );

    m_pLineColorB = new KColorButton( m_pColorGB );
    colorLayout -> addWidget( m_pLineColorB, 0, 1 );
    m_pLineColorL->setBuddy(m_pLineColorB);

    m_pLineDefaultB = new QPushButton( i18n( "&Default" ), m_pColorGB) ;
    colorLayout -> addWidget( m_pLineDefaultB, 0, 2 );

    m_pFillColorL = new QLabel( i18n( "&Fill:" ), m_pColorGB );
    colorLayout -> addWidget( m_pFillColorL, 1, 0 );

    m_pFillColorB = new KColorButton( m_pColorGB );
    colorLayout -> addWidget( m_pFillColorB, 1, 1 );
    m_pFillColorL->setBuddy(m_pFillColorB);

    m_pFillDefaultB = new QPushButton( i18n( "D&efault" ), m_pColorGB );
    colorLayout -> addWidget( m_pFillDefaultB, 1, 2 );

    m_pUseFillColorCB = new QCheckBox( i18n( "&Use fill" ), m_pColorGB );
    colorLayout -> setRowStretch( 2, 2 );
    colorLayout -> addWidget( m_pUseFillColorCB, 2, 0 );

    //connect button signals up
    connect( m_pLineDefaultB, SIGNAL( clicked() ), this, SLOT( slotLineButtonClicked() )) ;
    connect( m_pFillDefaultB, SIGNAL( clicked() ), this, SLOT( slotFillButtonClicked() ) );
}

UMLWidgetColorPage::~UMLWidgetColorPage() {}

void UMLWidgetColorPage::slotLineButtonClicked() {
    //  UMLView * pView = dynamic_cast<UMLView *>( m_pUMLWidget -> parent() );
    m_pLineColorB -> setColor( Settings::getOptionState().uiState.lineColor );
}

void UMLWidgetColorPage::slotFillButtonClicked() {
    //  UMLView * pView = dynamic_cast<UMLView *>( m_pUMLWidget -> parent() );
    m_pFillColorB -> setColor( Settings::getOptionState().uiState.fillColor );
}

void UMLWidgetColorPage::updateUMLWidget() {
    if(m_pUMLWidget)
    {
        m_pUMLWidget->setUseFillColour( m_pUseFillColorCB -> isChecked() );
        m_pUMLWidget->setLineColor( m_pLineColorB -> color() );
        m_pUMLWidget->setFillColour( m_pFillColorB -> color() );
    }
    else if(m_options)
    {
        m_options->uiState.useFillColor = m_pUseFillColorCB -> isChecked();
        m_options->uiState.lineColor = m_pLineColorB -> color();
        m_options->uiState.fillColor = m_pFillColorB -> color();
    }
}

#include "umlwidgetcolorpage.moc"
