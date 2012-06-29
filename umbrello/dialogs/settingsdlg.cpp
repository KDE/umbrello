/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2011                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "settingsdlg.h"

// app includes
#include "autolayoutoptionpage.h"
#include "classoptionspage.h"
#include "codeimportoptionspage.h"
#include "codegenoptionspage.h"
#include "umlwidgetstylepage.h"
#include "codevieweroptionspage.h"
#include "generaloptionpage.h"
#include "dialog_utils.h"
#include "debug_utils.h"
#include "icon_utils.h"
#include "layoutgenerator.h"

#include <kfiledialog.h>
#include <kvbox.h>

//TODO don't do that, but it's better than hardcoded in the functions body
#define FILL_COLOR QColor(255, 255, 192) 
#define LINK_COLOR Qt::red
#define TEXT_COLOR Qt::black


SettingsDlg::SettingsDlg( QWidget * parent, Settings::OptionState *state )
        : KPageDialog( parent)
{
    setCaption(i18n("Umbrello Setup") );
    setButtons( Help | Default | Apply | Ok | Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );
    setFaceType( KPageDialog::List );
    m_bChangesApplied = false;
    m_pOptionState = state;
    setHelp( "umbrello/index.html", QString() );
    setupGeneralPage();
    setupFontPage();
    setupUIPage();
    setupClassPage();
    setupCodeImportPage();
    setupCodeGenPage();
    setupCodeViewerPage(state->codeViewerState);
    setupAutoLayoutPage();
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
    connect(this,SIGNAL(defaultClicked()),this,SLOT(slotDefault()));
}

SettingsDlg::~SettingsDlg()
{
}

void SettingsDlg::setupUIPage()
{
    // FIXME: merge with UMLWidgetStylePage
    //setup UI page
    QWidget * page = new QWidget();
    QVBoxLayout* uiPageLayout = new QVBoxLayout(page);
    
    pageUserInterface = new KPageWidgetItem( page,i18n("User Interface"));
    pageUserInterface->setHeader( i18n("User Interface Settings") );
    pageUserInterface->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_UserInterface) );
    addPage( pageUserInterface );

    m_UiWidgets.colorGB = new QGroupBox( i18nc("color group box", "Color"), page );
    QGridLayout * colorLayout = new QGridLayout( m_UiWidgets.colorGB );
    colorLayout->setSpacing( spacingHint() );
    colorLayout->setMargin( fontMetrics().height() );
    uiPageLayout->addWidget(m_UiWidgets.colorGB);
    
    uiPageLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    m_UiWidgets.textColorCB = new QCheckBox( i18n("Custom text color:"), m_UiWidgets.colorGB );
    colorLayout->addWidget( m_UiWidgets.textColorCB, 0, 0 );

    m_UiWidgets.textColorB = new KColorButton( m_pOptionState->uiState.textColor ,m_UiWidgets.colorGB);
    //m_UiWidgets.lineColorB->setObjectName( m_UiWidgets.colorGB );
    colorLayout->addWidget( m_UiWidgets.textColorB, 0, 1 );

    m_UiWidgets.lineColorCB = new QCheckBox( i18n("Custom line color:"), m_UiWidgets.colorGB );
    colorLayout->addWidget( m_UiWidgets.lineColorCB, 1, 0 );

    m_UiWidgets.lineColorB = new KColorButton( m_pOptionState->uiState.lineColor ,m_UiWidgets.colorGB);
    //m_UiWidgets.lineColorB->setObjectName( m_UiWidgets.colorGB );
    colorLayout->addWidget( m_UiWidgets.lineColorB, 1, 1 );

//     m_UiWidgets.lineDefaultB = new QPushButton( i18n("D&efault Color"), m_UiWidgets.colorGB );
//     colorLayout->addWidget( m_UiWidgets.lineDefaultB, 0, 2 );

    m_UiWidgets.fillColorCB = new QCheckBox( i18n("Custom fill color:"), m_UiWidgets.colorGB );
    colorLayout->addWidget( m_UiWidgets.fillColorCB, 2, 0 );

    m_UiWidgets.fillColorB = new KColorButton( m_pOptionState->uiState.fillColor,m_UiWidgets.colorGB  );
    colorLayout->addWidget( m_UiWidgets.fillColorB, 2, 1 );

    m_UiWidgets.gridColorCB = new QCheckBox( i18n("Custom grid color:"), m_UiWidgets.colorGB );
    colorLayout->addWidget(m_UiWidgets.gridColorCB , 3, 0 );

    m_UiWidgets.gridColorB = new KColorButton( m_pOptionState->uiState.gridDotColor,m_UiWidgets.colorGB  );
    colorLayout->addWidget(m_UiWidgets.gridColorB , 3, 1 );

    m_UiWidgets.bgColorCB = new QCheckBox( i18n("Custom background color:"), m_UiWidgets.colorGB );
    colorLayout->addWidget(m_UiWidgets.bgColorCB , 4, 0 );

    m_UiWidgets.bgColorB = new KColorButton( m_pOptionState->uiState.backgroundColor,m_UiWidgets.colorGB  );
    colorLayout->addWidget(m_UiWidgets.bgColorB , 4, 1 );

    m_UiWidgets.lineWidthCB = new QCheckBox( i18n("Custom line width:"), m_UiWidgets.colorGB );
    colorLayout->addWidget( m_UiWidgets.lineWidthCB, 5, 0 );

    m_UiWidgets.lineWidthB = new KIntSpinBox( 0, 10, 1, m_pOptionState->uiState.lineWidth, m_UiWidgets.colorGB );
    colorLayout->addWidget( m_UiWidgets.lineWidthB, 5, 1 );
    
    m_UiWidgets.useFillColorCB = new QCheckBox( i18n("&Use fill color"), m_UiWidgets.colorGB );
    //colorLayout->setRowStretch( 3, 2 );
    colorLayout->addWidget( m_UiWidgets.useFillColorCB, 6, 0 );
    m_UiWidgets.useFillColorCB->setChecked( m_pOptionState->uiState.useFillColor );

    //connect button signals up
    connect( m_UiWidgets.textColorCB, SIGNAL(toggled(bool)), this, SLOT(slotTextCBChecked(bool)) );
    connect( m_UiWidgets.lineColorCB, SIGNAL(toggled(bool)), this, SLOT(slotLineCBChecked(bool)) );
    connect( m_UiWidgets.fillColorCB, SIGNAL(toggled(bool)), this, SLOT(slotFillCBChecked(bool)) );
    connect( m_UiWidgets.gridColorCB, SIGNAL(toggled(bool)), this, SLOT(slotGridCBChecked(bool)) );
    connect( m_UiWidgets.bgColorCB, SIGNAL(toggled(bool)), this, SLOT(slotBgCBChecked(bool)) );
    
    //TODO Once the new canvas is complete, so something better, it does not worth it for now
    if (m_UiWidgets.textColorB->color() == TEXT_COLOR) {
        m_UiWidgets.textColorCB->setChecked(false);
        m_UiWidgets.textColorB->setDisabled(true);
    }
    else {
        m_UiWidgets.textColorCB->setChecked(true);
    }

    if (m_UiWidgets.fillColorB->color() == FILL_COLOR) {
        m_UiWidgets.fillColorCB->setChecked(false);
        m_UiWidgets.fillColorB->setDisabled(true);
    }
    else {
        m_UiWidgets.fillColorCB->setChecked(true);
    }
    
    if (m_UiWidgets.lineColorB->color() == LINK_COLOR) {
        m_UiWidgets.lineColorCB->setChecked(false);
        m_UiWidgets.lineColorB->setDisabled(true);
    }
    else {
        m_UiWidgets.lineColorCB->setChecked(true);
    }
}

void SettingsDlg::setupGeneralPage()
{
    //setup General page
    KVBox * page = new KVBox();
    pageGeneral = new KPageWidgetItem( page, i18nc("general settings page", "General"));
    pageGeneral->setHeader( i18n("General Settings") );
    pageGeneral->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_General) );
    addPage( pageGeneral );
    m_pGeneralPage = new GeneralOptionPage(page);
}

void SettingsDlg::setupClassPage()
{
    //setup class settings page
    KVBox * page = new KVBox();
    pageClass = new KPageWidgetItem( page,i18nc("class settings page", "Class"));
    pageClass->setHeader( i18n("Class Settings") );
    pageClass->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Class) );
    addPage( pageClass );
    m_pClassPage = new ClassOptionsPage(page, m_pOptionState, false);
}

void SettingsDlg::setupCodeImportPage()
{
    //setup code importer settings page
    KVBox * page = new KVBox();
    pageCodeImport = new KPageWidgetItem( page,i18n("Code Importer") );
    pageCodeImport->setHeader( i18n("Code Import Settings") );
    pageCodeImport->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_CodeImport) );
    addPage( pageCodeImport );
    m_pCodeImportPage = new CodeImportOptionsPage(page);
}

void SettingsDlg::setupCodeGenPage()
{
    //setup code generation settings page
    KVBox * page = new KVBox();
    pageCodeGen = new KPageWidgetItem( page,i18n("Code Generation") );
    pageCodeGen->setHeader( i18n("Code Generation Settings") );
    pageCodeGen->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_CodeGeneration) );
    addPage( pageCodeGen );
    m_pCodeGenPage = new CodeGenOptionsPage(page);
    connect( m_pCodeGenPage, SIGNAL(languageChanged()), this, SLOT(slotApply()) );
}

void SettingsDlg::setupCodeViewerPage(Settings::CodeViewerState options)
{
    //setup code generation settings page
    KVBox * page = new KVBox();
    pageCodeViewer = new KPageWidgetItem( page,i18n("Code Viewer")  );
    pageCodeViewer->setHeader( i18n("Code Viewer Settings") );
    pageCodeViewer->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_CodeViewer) );
    addPage( pageCodeViewer );
    m_pCodeViewerPage = new CodeViewerOptionsPage(options, page);
}

void SettingsDlg::setupFontPage()
{
    KVBox * page = new KVBox();
    pageFont = new KPageWidgetItem( page,i18n("Font")  );
    pageFont->setHeader( i18n("Font Settings") );
    pageFont->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Font) );
    addPage( pageFont );
    m_FontWidgets.chooser = new KFontChooser( page,  KFontChooser::NoDisplayFlags, QStringList(), 0);
    m_FontWidgets.chooser->setFont( m_pOptionState->uiState.font );
}

void SettingsDlg::setupAutoLayoutPage()
{
    KVBox * page = new KVBox();
    pageAutoLayout = new KPageWidgetItem( page,i18n("Auto Layout")  );
    pageAutoLayout->setHeader( i18n("Auto Layout Settings") );
    pageAutoLayout->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_AutoLayout) );
    addPage( pageAutoLayout );
    m_pAutoLayoutPage = new AutoLayoutOptionPage( page );
}

void SettingsDlg::slotApply()
{
    applyPage( currentPage() );
    //do no emit signal applyClicked in the slot slotApply->infinite loop
    //emit applyClicked();
}

void SettingsDlg::slotOk()
{
    applyPage( pageClass);
    applyPage( pageGeneral);
    applyPage( pageUserInterface );
    applyPage( pageCodeViewer);
    applyPage( pageCodeImport );
    applyPage( pageCodeGen );
    applyPage( pageFont );
    applyPage( pageAutoLayout );
    accept();
}

void SettingsDlg::slotDefault()
{
    // Defaults hard coded.  Make sure that this is alright.
    // If defaults are set anywhere else, like in setting up config file, make sure the same.
    KPageWidgetItem *current = currentPage();
    if ( current ==  pageGeneral )
    {
        m_pGeneralPage->setDefaults();
    }
    else if ( current == pageFont )
    {
        m_FontWidgets.chooser->setFont( parentWidget()->font() );
    }
    else if ( current == pageUserInterface )
    {
        m_UiWidgets.useFillColorCB->setChecked( true );
        m_UiWidgets.textColorB->setColor( TEXT_COLOR );
        m_UiWidgets.fillColorB->setColor( FILL_COLOR );
        m_UiWidgets.lineColorB->setColor( LINK_COLOR );
        m_UiWidgets.lineWidthB->setValue( 0 );
    }
    else if ( current == pageClass )
    {
        m_pClassPage->setDefaults();
    }
    else if (  current == pageCodeImport )
    {
        m_pCodeImportPage->setDefaults();
    }
    else if (  current == pageCodeGen )
    {
    }
    else if ( current == pageCodeViewer )
    {
    }
    else if ( current == pageAutoLayout )
    {
        m_pAutoLayoutPage->setDefaults();
    }
}

void SettingsDlg::applyPage( KPageWidgetItem*item )
{
    m_bChangesApplied = true;
    if ( item == pageGeneral )
    {
        m_pGeneralPage->apply();
    }
    else if ( item == pageFont )
    {
        m_pOptionState->uiState.font = m_FontWidgets.chooser->font();
    }
    else if ( item == pageUserInterface )
    {
        m_pOptionState->uiState.useFillColor = m_UiWidgets.useFillColorCB->isChecked();
        m_pOptionState->uiState.fillColor = m_UiWidgets.fillColorB->color();
        m_pOptionState->uiState.textColor = m_UiWidgets.textColorB->color();
        m_pOptionState->uiState.lineColor = m_UiWidgets.lineColorB->color();
        m_pOptionState->uiState.lineWidth = m_UiWidgets.lineWidthB->value();
        m_pOptionState->uiState.backgroundColor = m_UiWidgets.bgColorB->color();
        m_pOptionState->uiState.gridDotColor = m_UiWidgets.gridColorB->color();
    }
    else if ( item == pageClass )
    {
        m_pClassPage->apply();
    }
    else if ( item == pageCodeImport )
    {
        m_pCodeImportPage->apply();
    }
    else if ( item == pageCodeGen )
    {
        m_pCodeGenPage->apply();
    }
    else if ( item == pageCodeViewer )
    {
        m_pCodeViewerPage->apply();
        m_pOptionState->codeViewerState = m_pCodeViewerPage->getOptions();
    }
    else if ( item == pageAutoLayout )
    {
        m_pAutoLayoutPage->apply();
    }
}

void SettingsDlg::slotTextCBChecked(bool value)
{
    if (value == false) {
        m_UiWidgets.textColorB->setColor( TEXT_COLOR );
        m_UiWidgets.textColorB->setDisabled(true);
    }
    else {
        m_UiWidgets.textColorB->setDisabled(false);
    }
}

void SettingsDlg::slotLineCBChecked(bool value)
{
    if (value == false) {
        m_UiWidgets.lineColorB->setColor( LINK_COLOR );
        m_UiWidgets.lineColorB->setDisabled(true);
    }
    else {
        m_UiWidgets.lineColorB->setDisabled(false);
    }
}

void SettingsDlg::slotFillCBChecked(bool value)
{
    if (value == false) {
        m_UiWidgets.fillColorB->setColor( FILL_COLOR );
        m_UiWidgets.fillColorB->setDisabled(true);
    }
    else {
        m_UiWidgets.fillColorB->setDisabled(false);
    }
}

void SettingsDlg::slotGridCBChecked(bool value)
{
    if (value == false) {
        QPalette palette;
        m_UiWidgets.gridColorB->setColor( palette.alternateBase().color() );
        m_UiWidgets.gridColorB->setDisabled(true);
    }
    else {
        m_UiWidgets.gridColorB->setDisabled(false);
    }
}

void SettingsDlg::slotBgCBChecked(bool value)
{
    if (value == false) {
        QPalette palette;
        m_UiWidgets.bgColorB->setColor( palette.base().color() );
        m_UiWidgets.bgColorB->setDisabled(true);
    }
    else {
        m_UiWidgets.bgColorB->setDisabled(false);
    }
}

QString SettingsDlg::getCodeGenerationLanguage()
{
    return m_pCodeGenPage->getLanguage();
}

#include "settingsdlg.moc"
