/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "umlwidgetstylepage.h"

#include "optionstate.h"
#include "uml.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlviewcanvas.h"
#include "widgetbase.h"

#include <klocale.h>
#include <kcolorbutton.h>
#include <KIntSpinBox>

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
UMLWidgetStylePage::UMLWidgetStylePage( QWidget *pParent, WidgetBase *pWidget )
  : QWidget( pParent )
{
    m_pUMLWidget = pWidget;
    m_options = 0;
    init();
    m_pTextColorB->setColor( pWidget->textColor() );
    m_pLineColorB->setColor( pWidget->lineColor() );
    m_pFillColorB->setColor( pWidget->fillColor() );
    m_pUseFillColorCB->setChecked( pWidget->useFillColor() );
    m_lineWidthB->setValue( pWidget->lineWidth() );

    if (!m_pUMLWidget) {  //  when we are on the diagram
        UMLView * view = UMLApp::app()->currentView();
        if (view) {
            UMLViewCanvas* canvas = dynamic_cast<UMLViewCanvas*>(view->canvas());
            if (canvas) {
                m_BackgroundColorB->setColor(canvas->backgroundColor());
                m_GridDotColorB->setColor(canvas->gridDotColor());
            }
        }
    }
}

/**
 *   Constructor - Observe an OptionState structure.
 */
UMLWidgetStylePage::UMLWidgetStylePage( QWidget * pParent, Settings::OptionState *options )
  : QWidget( pParent )
{
    m_options = options;
    m_pUMLWidget = 0;
    init();
    m_pTextColorB->setColor( m_options->uiState.textColor );
    m_pLineColorB->setColor( m_options->uiState.lineColor );
    m_pFillColorB->setColor( m_options->uiState.fillColor );
    m_pUseFillColorCB->setChecked( m_options->uiState.useFillColor );
    m_GridDotColorB->setColor( m_options->uiState.gridDotColor );
    m_BackgroundColorB->setColor( m_options->uiState.backgroundColor );
    m_lineWidthB->setValue( m_options->uiState.lineWidth );
}

void UMLWidgetStylePage::init()
{
    int margin = fontMetrics().height();

    //setup GUI
    QVBoxLayout * topLayout = new QVBoxLayout( this );
    topLayout->setSpacing( 6 );
    int row = 0;

    m_pColorGB = new QGroupBox( i18nc("title of color group", "Color"), this );
    topLayout->addWidget( m_pColorGB );
    QGridLayout * colorLayout = new QGridLayout( m_pColorGB );
    colorLayout->setMargin( margin );

    m_pTextColorL = new QLabel( i18nc("text color", "&Text:"), m_pColorGB );
    colorLayout->addWidget( m_pTextColorL, row, 0 );

    m_pTextColorB = new KColorButton( m_pColorGB );
    colorLayout->addWidget( m_pTextColorB, row, 1 );
    m_pTextColorL->setBuddy(m_pTextColorB);

    m_pTextDefaultB = new QPushButton( i18nc("default text color button", "Defaul&t"), m_pColorGB) ;
    colorLayout->addWidget( m_pTextDefaultB, row, 2 );

    m_pLineColorL = new QLabel( i18nc("line color", "&Line:"), m_pColorGB );
    colorLayout->addWidget( m_pLineColorL, ++row, 0 );

    m_pLineColorB = new KColorButton( m_pColorGB );
    colorLayout->addWidget( m_pLineColorB, row, 1 );
    m_pLineColorL->setBuddy(m_pLineColorB);

    m_pLineDefaultB = new QPushButton( i18nc("default line color button", "&Default"), m_pColorGB) ;
    colorLayout->addWidget( m_pLineDefaultB, row, 2 );

    m_pFillColorL = new QLabel( i18n("&Fill:"), m_pColorGB );
    colorLayout->addWidget( m_pFillColorL, ++row, 0 );

    m_pFillColorB = new KColorButton( m_pColorGB );
    colorLayout->addWidget( m_pFillColorB, row, 1 );
    m_pFillColorL->setBuddy(m_pFillColorB);

    m_pFillDefaultB = new QPushButton( i18nc("default fill color button", "D&efault"), m_pColorGB );
    colorLayout->addWidget( m_pFillDefaultB, row, 2 );

    m_pUseFillColorCB = new QCheckBox( i18n("&Use fill"), m_pColorGB );
    colorLayout->setRowStretch( 3, 2 );
    colorLayout->addWidget( m_pUseFillColorCB, ++row, 0 );

    //connect button signals up
    connect(m_pTextDefaultB, SIGNAL(clicked()), this, SLOT(slotTextButtonClicked())) ;
    connect(m_pLineDefaultB, SIGNAL(clicked()), this, SLOT(slotLineButtonClicked())) ;
    connect(m_pFillDefaultB, SIGNAL(clicked()), this, SLOT(slotFillButtonClicked()));

    if (!m_pUMLWidget) {  //  when we are on the diagram
        m_BackgroundColorL = new QLabel( i18nc("background color", "&Background:"), m_pColorGB );
        colorLayout->addWidget( m_BackgroundColorL, ++row, 0 );

        m_BackgroundColorB = new KColorButton( m_pColorGB );
        colorLayout->addWidget( m_BackgroundColorB, row, 1 );
        m_BackgroundColorL->setBuddy(m_BackgroundColorB);

        m_BackgroundDefaultB = new QPushButton( i18nc("default background color button", "De&fault"), m_pColorGB) ;
        colorLayout->addWidget( m_BackgroundDefaultB, row, 2 );

        m_GridDotColorL = new QLabel( i18nc("grid dot color", "&Grid dot:"), m_pColorGB );
        colorLayout->addWidget( m_GridDotColorL, ++row, 0 );

        m_GridDotColorB = new KColorButton( m_pColorGB );
        colorLayout->addWidget( m_GridDotColorB, row, 1 );
        m_GridDotColorL->setBuddy(m_GridDotColorB);

        m_GridDotDefaultB = new QPushButton( i18nc("default grid dot color button", "Def&ault"), m_pColorGB) ;
        colorLayout->addWidget( m_GridDotDefaultB, row, 2 );

        //connect button signals up
        connect(m_BackgroundDefaultB, SIGNAL(clicked()), this, SLOT(slotBackgroundButtonClicked()));
        connect(m_GridDotDefaultB,    SIGNAL(clicked()), this, SLOT(slotGridDotButtonClicked()));
    }

    m_pStyleGB = new QGroupBox( i18nc("title of width group", "Width"), this );
    topLayout->addWidget( m_pStyleGB );
    QGridLayout *styleLayout = new QGridLayout( m_pStyleGB );
    styleLayout->setMargin( margin );

    m_lineWidthL = new QLabel( i18nc("line width", "Line &width:"), m_pStyleGB );
    styleLayout->addWidget( m_lineWidthL, ++row, 0 );

    m_lineWidthB = new KIntSpinBox( 0, 10, 1, 0, m_pStyleGB );
    styleLayout->addWidget( m_lineWidthB, row, 1 );

    m_lineWidthDefaultB = new QPushButton( i18nc("default line width button", "Defa&ult"), m_pStyleGB) ;
    styleLayout->addWidget( m_lineWidthDefaultB, row, 2 );

    //connect button signals up
    connect(m_lineWidthDefaultB, SIGNAL(clicked()), this, SLOT(slotLineWidthButtonClicked()));
}

/**
 *   Destructor.
 */
UMLWidgetStylePage::~UMLWidgetStylePage()
{
}

/**
 *   Sets the default text color when default text button
 *   clicked.
 */
void UMLWidgetStylePage::slotTextButtonClicked()
{
    m_pTextColorB->setColor( Settings::optionState().uiState.textColor );
}

/**
 *   Sets the default line color when default line button
 *   clicked.
 */
void UMLWidgetStylePage::slotLineButtonClicked()
{
    m_pLineColorB->setColor( Settings::optionState().uiState.lineColor );
}

/**
 *   Sets the default fill color when default fill button
 *   clicked.
 */
void UMLWidgetStylePage::slotFillButtonClicked()
{
    m_pFillColorB->setColor( Settings::optionState().uiState.fillColor );
}

/**
 *   Sets the default fill color when default fill button
 *   clicked.
 */
void UMLWidgetStylePage::slotBackgroundButtonClicked()
{
    m_BackgroundColorB->setColor( Settings::optionState().uiState.backgroundColor );
}

/**
 *   Sets the default fill color when default fill button
 *   clicked.
 */
void UMLWidgetStylePage::slotGridDotButtonClicked()
{
    m_GridDotColorB->setColor( Settings::optionState().uiState.gridDotColor );
}

/**
 *   Sets the default line color when default line button
 *   clicked.
 */
void UMLWidgetStylePage::slotLineWidthButtonClicked()
{
    m_lineWidthB->setValue( Settings::optionState().uiState.lineWidth );
}

/**
 *   Updates the @ref UMLWidget with the dialog properties.
 */
void UMLWidgetStylePage::updateUMLWidget()
{
    if (m_pUMLWidget) {
        m_pUMLWidget->setUseFillColor( m_pUseFillColorCB->isChecked() );
        m_pUMLWidget->setTextColor( m_pTextColorB->color() );
        m_pUMLWidget->setLineColor( m_pLineColorB->color() );
        m_pUMLWidget->setFillColor( m_pFillColorB->color() );
        m_pUMLWidget->setLineWidth( m_lineWidthB->value() );
    }
    else if (m_options) {
        m_options->uiState.useFillColor = m_pUseFillColorCB->isChecked();
        m_options->uiState.textColor = m_pTextColorB->color();
        m_options->uiState.lineColor = m_pLineColorB->color();
        m_options->uiState.fillColor = m_pFillColorB->color();
        m_options->uiState.backgroundColor = m_BackgroundColorB->color();
        m_options->uiState.gridDotColor = m_GridDotColorB->color();
        m_options->uiState.lineWidth = m_lineWidthB->value();
        UMLApp::app()->currentView()->umlScene()->setOptionState(*m_options);
    }

    if (!m_pUMLWidget) {  // when we are on the diagram
        UMLView * view = UMLApp::app()->currentView();
        if (view) {
            UMLViewCanvas* canvas = dynamic_cast<UMLViewCanvas*>(view->canvas());
            if (canvas) {
                canvas->setColors(m_BackgroundColorB->color(), m_GridDotColorB->color());
            }
        }
    }
}

#include "umlwidgetstylepage.moc"
