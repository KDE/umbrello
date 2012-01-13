/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "umlwidgetcolorpage.h"

#include "optionstate.h"
#include "uml.h"
#include "umlview.h"
#include "umlwidget.h"

#include <klocale.h>
#include <kcolorbutton.h>

#include <QtGui/QLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>

/**
 *   Constructor - Observe a UMLWidget.
 */
UMLWidgetColorPage::UMLWidgetColorPage( QWidget *pParent, UMLWidget *pWidget ) : QWidget( pParent )
{
    m_pUMLWidget = pWidget;
    m_options = 0;
    init();
    m_pLineColorB->setColor( pWidget->lineColor() );
    m_pFillColorB->setColor( pWidget->getFillColor() );
    m_pUseFillColorCB->setChecked( pWidget->useFillColor() );

    if (!m_pUMLWidget) {  //  when we are on the diagram
        UMLView * view = UMLApp::app()->currentView();
        if (view) {
//:TODO:            UMLViewCanvas* canvas = dynamic_cast<UMLViewCanvas*>(view->canvas());
//:TODO:            if (canvas) {
//:TODO:                m_BackgroundColorB->setColor(canvas->backgroundColor());
//:TODO:                m_GridDotColorB->setColor(canvas->gridDotColor());
//:TODO:            }
        }
    }
}

/**
 *   Constructor - Observe an OptionState structure.
 */
UMLWidgetColorPage::UMLWidgetColorPage( QWidget * pParent, Settings::OptionState *options ) : QWidget( pParent )
{
    m_options = options;
    m_pUMLWidget = 0;
    init();
    m_pLineColorB->setColor( m_options->uiState.lineColor );
    m_pFillColorB->setColor( m_options->uiState.fillColor );
    m_pUseFillColorCB->setChecked( m_options->uiState.useFillColor );
    m_BackgroundColorB->setColor( m_options->uiState.backgroundColor );
    m_GridDotColorB->setColor( m_options->uiState.gridDotColor );
}

void UMLWidgetColorPage::init()
{
    int margin = fontMetrics().height();

    //setup GUI
    QVBoxLayout * topLayout = new QVBoxLayout( this );
    topLayout->setSpacing( 6 );

    m_pColorGB = new QGroupBox( i18nc("title of color group", "Color"), this );
    topLayout->addWidget( m_pColorGB );
    QGridLayout * colorLayout = new QGridLayout( m_pColorGB );
    colorLayout->setMargin( margin );

    m_pLineColorL = new QLabel( i18nc("line color", "&Line:"), m_pColorGB );
    colorLayout->addWidget( m_pLineColorL, 0, 0 );

    m_pLineColorB = new KColorButton( m_pColorGB );
    colorLayout->addWidget( m_pLineColorB, 0, 1 );
    m_pLineColorL->setBuddy(m_pLineColorB);

    m_pLineDefaultB = new QPushButton( i18nc("default line color button", "&Default"), m_pColorGB) ;
    colorLayout->addWidget( m_pLineDefaultB, 0, 2 );

    m_pFillColorL = new QLabel( i18n("&Fill:"), m_pColorGB );
    colorLayout->addWidget( m_pFillColorL, 1, 0 );

    m_pFillColorB = new KColorButton( m_pColorGB );
    colorLayout->addWidget( m_pFillColorB, 1, 1 );
    m_pFillColorL->setBuddy(m_pFillColorB);

    m_pFillDefaultB = new QPushButton( i18nc("default fill color button", "D&efault"), m_pColorGB );
    colorLayout->addWidget( m_pFillDefaultB, 1, 2 );

    m_pUseFillColorCB = new QCheckBox( i18n("&Use fill"), m_pColorGB );
    colorLayout->setRowStretch( 2, 2 );
    colorLayout->addWidget( m_pUseFillColorCB, 2, 0 );

    //connect button signals up
    connect(m_pLineDefaultB, SIGNAL(clicked()), this, SLOT(slotLineButtonClicked())) ;
    connect(m_pFillDefaultB, SIGNAL(clicked()), this, SLOT(slotFillButtonClicked()));

    if (!m_pUMLWidget) {  //  when we are on the diagram
        m_BackgroundColorL = new QLabel( i18nc("background color", "&Background:"), m_pColorGB );
        colorLayout->addWidget( m_BackgroundColorL, 3, 0 );

        m_BackgroundColorB = new KColorButton( m_pColorGB );
        colorLayout->addWidget( m_BackgroundColorB, 3, 1 );
        m_BackgroundColorL->setBuddy(m_BackgroundColorB);

        m_BackgroundDefaultB = new QPushButton( i18nc("default background color button", "De&fault"), m_pColorGB) ;
        colorLayout->addWidget( m_BackgroundDefaultB, 3, 2 );

        m_GridDotColorL = new QLabel( i18nc("grid dot color", "&Grid dot:"), m_pColorGB );
        colorLayout->addWidget( m_GridDotColorL, 4, 0 );

        m_GridDotColorB = new KColorButton( m_pColorGB );
        colorLayout->addWidget( m_GridDotColorB, 4, 1 );
        m_GridDotColorL->setBuddy(m_GridDotColorB);

        m_GridDotDefaultB = new QPushButton( i18nc("default grid dot color button", "Def&ault"), m_pColorGB) ;
        colorLayout->addWidget( m_GridDotDefaultB, 4, 2 );

        //connect button signals up
        connect(m_BackgroundDefaultB, SIGNAL(clicked()), this, SLOT(slotBackgroundButtonClicked()));
        connect(m_GridDotDefaultB,    SIGNAL(clicked()), this, SLOT(slotGridDotButtonClicked()));
    }
}

/**
 *   Destructor.
 */
UMLWidgetColorPage::~UMLWidgetColorPage()
{
}

/**
 *   Sets the default line color when default line button
 *   clicked.
 */
void UMLWidgetColorPage::slotLineButtonClicked()
{
    m_pLineColorB->setColor( Settings::optionState().uiState.lineColor );
}

/**
 *   Sets the default fill color when default fill button
 *   clicked.
 */
void UMLWidgetColorPage::slotFillButtonClicked()
{
    m_pFillColorB->setColor( Settings::optionState().uiState.fillColor );
}

/**
 *   Sets the default fill color when default fill button
 *   clicked.
 */
void UMLWidgetColorPage::slotBackgroundButtonClicked()
{
    m_BackgroundColorB->setColor( Settings::optionState().uiState.backgroundColor );
}

/**
 *   Sets the default fill color when default fill button
 *   clicked.
 */
void UMLWidgetColorPage::slotGridDotButtonClicked()
{
    m_GridDotColorB->setColor( Settings::optionState().uiState.gridDotColor );
}

/**
 *   Updates the @ref UMLWidget with the dialog properties.
 */
void UMLWidgetColorPage::updateUMLWidget()
{
    if (m_pUMLWidget) {
        m_pUMLWidget->setUseFillColor( m_pUseFillColorCB->isChecked() );
        m_pUMLWidget->setLineColor( m_pLineColorB->color() );
        m_pUMLWidget->setFillColor( m_pFillColorB->color() );
    }
    else if (m_options) {
        m_options->uiState.useFillColor = m_pUseFillColorCB->isChecked();
        m_options->uiState.lineColor = m_pLineColorB->color();
        m_options->uiState.fillColor = m_pFillColorB->color();
        m_options->uiState.backgroundColor = m_BackgroundColorB->color();
        m_options->uiState.gridDotColor = m_GridDotColorB->color();
    }

    if (!m_pUMLWidget) {  // when we are on the diagram
        UMLView * view = UMLApp::app()->currentView();
        if (view) {
//:TODO:            UMLViewCanvas* canvas = dynamic_cast<UMLViewCanvas*>(view->canvas());
//:TODO:            if (canvas) {
//:TODO:                canvas->setColors(m_BackgroundColorB->color(), m_GridDotColorB->color());
//:TODO:            }
        }
    }
}

#include "umlwidgetcolorpage.moc"
