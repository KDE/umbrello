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
#include "codeimportoptionspage.h"
#include "codegenoptionspage.h"
#include "umlwidgetstylepage.h"
#include "codevieweroptionspage.h"
#include "dialog_utils.h"
#include "debug_utils.h"
#include "icon_utils.h"

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

    // Set up undo setting
    m_GeneralWidgets.miscGB = new QGroupBox( i18nc("miscellaneous group box", "Miscellaneous"), page );

    QGridLayout * miscLayout = new QGridLayout( m_GeneralWidgets.miscGB );
    miscLayout->setSpacing( spacingHint() );
    miscLayout->setMargin( fontMetrics().height() );

    m_GeneralWidgets.undoCB = new QCheckBox( i18n("Enable undo"), m_GeneralWidgets.miscGB );
    m_GeneralWidgets.undoCB->setChecked( m_pOptionState->generalState.undo );
    miscLayout->addWidget( m_GeneralWidgets.undoCB, 0, 0 );

    m_GeneralWidgets.tabdiagramsCB = new QCheckBox( i18n("Use tabbed diagrams"), m_GeneralWidgets.miscGB );
    m_GeneralWidgets.tabdiagramsCB->setChecked( m_pOptionState->generalState.tabdiagrams );
    miscLayout->addWidget( m_GeneralWidgets.tabdiagramsCB, 0, 1 );

    m_GeneralWidgets.newcodegenCB = new QCheckBox( i18n("Use new C++/Java/Ruby generators"), m_GeneralWidgets.miscGB );
    m_GeneralWidgets.newcodegenCB->setChecked( m_pOptionState->generalState.newcodegen );
    miscLayout->addWidget( m_GeneralWidgets.newcodegenCB, 1, 0 );

    m_GeneralWidgets.angularLinesCB = new QCheckBox( i18n("Use angular association lines"), m_GeneralWidgets.miscGB );
    m_GeneralWidgets.angularLinesCB->setChecked( m_pOptionState->generalState.angularlines );
    miscLayout->addWidget( m_GeneralWidgets.angularLinesCB, 1, 1 );

    m_GeneralWidgets.footerPrintingCB = new QCheckBox( i18n("Turn on footer and page numbers when printing"), m_GeneralWidgets.miscGB );
    m_GeneralWidgets.footerPrintingCB->setChecked( m_pOptionState->generalState.footerPrinting );
    miscLayout->addWidget( m_GeneralWidgets.footerPrintingCB, 2, 0 );

    //setup autosave settings
    m_GeneralWidgets.autosaveGB = new QGroupBox( i18n("Autosave"), page );

    QGridLayout * autosaveLayout = new QGridLayout( m_GeneralWidgets.autosaveGB );
    autosaveLayout->setSpacing( spacingHint() );
    autosaveLayout->setMargin( fontMetrics().height() );

    m_GeneralWidgets.autosaveCB = new QCheckBox( i18n("E&nable autosave"), m_GeneralWidgets.autosaveGB );
    m_GeneralWidgets.autosaveCB->setChecked( m_pOptionState->generalState.autosave );
    autosaveLayout->addWidget( m_GeneralWidgets.autosaveCB, 0, 0 );

    m_GeneralWidgets.autosaveL = new QLabel( i18n("Select auto-save time interval (mins):"), m_GeneralWidgets.autosaveGB );
    autosaveLayout->addWidget( m_GeneralWidgets.autosaveL, 1, 0 );

    m_GeneralWidgets.timeISB = new KIntSpinBox( 1, 600, 1, m_pOptionState->generalState.autosavetime, m_GeneralWidgets.autosaveGB );
    m_GeneralWidgets.timeISB->setEnabled( m_pOptionState->generalState.autosave );
    autosaveLayout->addWidget( m_GeneralWidgets.timeISB, 1, 1 );

    // Allow definition of Suffix for autosave ( default: ".xmi" )
    Dialog_Utils::makeLabeledEditField( m_GeneralWidgets.autosaveGB, autosaveLayout, 2,
                                    m_GeneralWidgets.autosaveSuffixL, i18n("Set autosave suffix:"),
                                    m_GeneralWidgets.autosaveSuffixT, m_pOptionState->generalState.autosavesuffix );
    QString autoSaveSuffixToolTip = i18n( "<qt><p>The autosave file will be saved to ~/autosave.xmi if the autosaving occurs "
                                          "before you have manually saved the file.</p>"
                                          "<p>If you have already saved it, the autosave file will be saved in the same folder as the file "
                                          "and will be named like the file's name, followed by the suffix specified.</p>"
                                          "<p>If the suffix is equal to the suffix of the file you have saved, "
                                          "the autosave will overwrite your file automatically.</p></qt>" );
    m_GeneralWidgets.autosaveSuffixL->setToolTip( autoSaveSuffixToolTip );
    m_GeneralWidgets.autosaveSuffixT->setToolTip( autoSaveSuffixToolTip );

    //setup startup settings
    m_GeneralWidgets.startupGB = new QGroupBox( i18n("Startup"), page );

    QGridLayout * startupLayout = new QGridLayout( m_GeneralWidgets.startupGB );
    startupLayout->setSpacing( spacingHint() );
    startupLayout->setMargin( fontMetrics().height() );

    m_GeneralWidgets.loadlastCB = new QCheckBox( i18n("&Load last project"), m_GeneralWidgets.startupGB );
    m_GeneralWidgets.loadlastCB->setChecked( m_pOptionState->generalState.loadlast );
    startupLayout->addWidget( m_GeneralWidgets.loadlastCB, 0, 0 );

    m_GeneralWidgets.startL = new QLabel( i18n("Start new project with:"), m_GeneralWidgets.startupGB );
    startupLayout->addWidget( m_GeneralWidgets.startL, 1, 0 );

    m_GeneralWidgets.diagramKB = new KComboBox( m_GeneralWidgets.startupGB );
    m_GeneralWidgets.diagramKB->setCompletionMode( KGlobalSettings::CompletionPopup );
    startupLayout->addWidget( m_GeneralWidgets.diagramKB, 1, 1 );

    // start at 1 because we don't allow No Diagram any more
    // diagramNo 1 is Uml::DiagramType::Class
    // digaramNo 9 is Uml::DiagramType::EntityRelationship
    for (int diagramNo = 1; diagramNo < 10; ++diagramNo) {
        Uml::DiagramType dt = Uml::DiagramType(Uml::DiagramType::Value(diagramNo));
        insertDiagram( dt.toString(), diagramNo - 1  );
    }

    m_GeneralWidgets.diagramKB->setCurrentIndex( (int)m_pOptionState->generalState.diagram-1 );
    connect( m_GeneralWidgets.autosaveCB, SIGNAL(clicked()), this, SLOT(slotAutosaveCBClicked()) );

    m_GeneralWidgets.defaultLanguageL = new QLabel( i18n( "Default Language :" ), m_GeneralWidgets.startupGB );
    startupLayout->addWidget( m_GeneralWidgets.defaultLanguageL, 2, 0 );

    m_GeneralWidgets.languageKB = new KComboBox( m_GeneralWidgets.startupGB );
    m_GeneralWidgets.languageKB->setCompletionMode( KGlobalSettings::CompletionPopup );
    startupLayout->addWidget( m_GeneralWidgets.languageKB, 2, 1 );

    int indexCounter = 0;
    while (indexCounter < Uml::ProgrammingLanguage::Reserved) {
        QString language = Uml::ProgrammingLanguage::toString(Uml::ProgrammingLanguage::Value(indexCounter));
        m_GeneralWidgets.languageKB->insertItem(indexCounter, language);
        indexCounter++;
    }
    m_GeneralWidgets.languageKB->setCurrentIndex(m_pOptionState->generalState.defaultLanguage);
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 */
void SettingsDlg::insertDiagram( const QString& type, int index )
{
    m_GeneralWidgets.diagramKB->insertItem( index, type );
    m_GeneralWidgets.diagramKB->completionObject()->addItem( type );
}

void SettingsDlg::setupClassPage()
{
    //setup class settings page
    KVBox * page = new KVBox();
    pageClass = new KPageWidgetItem( page,i18nc("class settings page", "Class"));
    pageClass->setHeader( i18n("Class Settings") );
    pageClass->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Class) );
    addPage( pageClass );

    m_ClassWidgets.visibilityGB = new QGroupBox( i18n("Visibility"), page );

    QGridLayout * visibilityLayout = new QGridLayout( m_ClassWidgets.visibilityGB );
    visibilityLayout->setSpacing( spacingHint() );
    visibilityLayout->setMargin(  fontMetrics().height()  );

    m_ClassWidgets.showVisibilityCB = new QCheckBox(i18n("Show &visibility"), m_ClassWidgets.visibilityGB);
    m_ClassWidgets.showVisibilityCB->setChecked(  m_pOptionState->classState.showVisibility );
    visibilityLayout->addWidget( m_ClassWidgets.showVisibilityCB, 0, 0 );

    m_ClassWidgets.showAttsCB = new QCheckBox( i18n("Show attributes"), m_ClassWidgets.visibilityGB );
    m_ClassWidgets.showAttsCB->setChecked(  m_pOptionState->classState.showAtts );
    visibilityLayout->addWidget( m_ClassWidgets.showAttsCB, 0, 1 );

    m_ClassWidgets.showOpsCB = new QCheckBox( i18n("Show operations"), m_ClassWidgets.visibilityGB );
    m_ClassWidgets.showOpsCB->setChecked(  m_pOptionState->classState.showOps );
    visibilityLayout->addWidget( m_ClassWidgets.showOpsCB, 1, 0 );

    m_ClassWidgets.showStereotypeCB = new QCheckBox( i18n("Show stereot&ype"), m_ClassWidgets.visibilityGB );
    m_ClassWidgets.showStereotypeCB->setChecked(  m_pOptionState->classState.showStereoType );
    visibilityLayout->addWidget( m_ClassWidgets.showStereotypeCB, 1, 1 );

    m_ClassWidgets.showAttSigCB = new QCheckBox(i18n("Show attribute signature"), m_ClassWidgets.visibilityGB);
    m_ClassWidgets.showAttSigCB->setChecked(   m_pOptionState->classState.showAttSig );
    visibilityLayout->addWidget( m_ClassWidgets.showAttSigCB, 2, 0 );

    m_ClassWidgets.showPackageCB = new QCheckBox(i18n("Show package"), m_ClassWidgets.visibilityGB);
    m_ClassWidgets.showPackageCB->setChecked(  m_pOptionState->classState.showPackage );
    visibilityLayout->addWidget( m_ClassWidgets.showPackageCB, 2, 1 );

    m_ClassWidgets.showOpSigCB = new QCheckBox( i18n("Show operation signature"), m_ClassWidgets.visibilityGB );
    m_ClassWidgets.showOpSigCB->setChecked(  m_pOptionState->classState.showOpSig );
    visibilityLayout->addWidget( m_ClassWidgets.showOpSigCB, 3, 0 );

    m_ClassWidgets.showPublicOnlyCB = new QCheckBox( i18n("Show Public Only"), m_ClassWidgets.visibilityGB );
    m_ClassWidgets.showPublicOnlyCB->setChecked(  m_pOptionState->classState.showPublicOnly );
    visibilityLayout->addWidget( m_ClassWidgets.showPublicOnlyCB, 3, 1 );
    visibilityLayout->setRowStretch( 3, 1 );

    m_ClassWidgets.scopeGB = new QGroupBox( i18n("Starting Scope"), page );
    QGridLayout * scopeLayout = new QGridLayout( m_ClassWidgets.scopeGB );
    scopeLayout->setSpacing( spacingHint() );
    scopeLayout->setMargin(  fontMetrics().height()  );

    m_ClassWidgets.attributeLabel = new QLabel( i18n("Default attribute scope:"), m_ClassWidgets.scopeGB);
    scopeLayout->addWidget( m_ClassWidgets.attributeLabel, 0, 0 );

    m_ClassWidgets.operationLabel = new QLabel( i18n("Default operation scope:"), m_ClassWidgets.scopeGB);
    scopeLayout->addWidget( m_ClassWidgets.operationLabel, 1, 0 );

    m_ClassWidgets.m_pAttribScopeCB = new KComboBox(m_ClassWidgets.scopeGB);
    insertAttribScope( tr2i18n( "Public" ) );
    insertAttribScope( tr2i18n( "Private" ) );
    insertAttribScope( tr2i18n( "Protected" ) );
    m_ClassWidgets.m_pAttribScopeCB->setCurrentIndex(m_pOptionState->classState.defaultAttributeScope);
    m_ClassWidgets.m_pAttribScopeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
    scopeLayout->addWidget( m_ClassWidgets.m_pAttribScopeCB, 0, 1 );

    m_ClassWidgets.m_pOperationScopeCB = new KComboBox(m_ClassWidgets.scopeGB);
    insertOperationScope( tr2i18n( "Public" ) );
    insertOperationScope( tr2i18n( "Private" ) );
    insertOperationScope( tr2i18n( "Protected" ) );
    m_ClassWidgets.m_pOperationScopeCB->setCurrentIndex(m_pOptionState->classState.defaultOperationScope);
    m_ClassWidgets.m_pOperationScopeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
    scopeLayout->addWidget( m_ClassWidgets.m_pOperationScopeCB, 1, 1 );
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 */
void SettingsDlg::insertAttribScope( const QString& type, int index )
{
    m_ClassWidgets.m_pAttribScopeCB->insertItem( index, type );
    m_ClassWidgets.m_pAttribScopeCB->completionObject()->addItem( type );
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 */
void SettingsDlg::insertOperationScope( const QString& type, int index )
{
    m_ClassWidgets.m_pOperationScopeCB->insertItem( index, type );
    m_ClassWidgets.m_pOperationScopeCB->completionObject()->addItem( type );
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
    accept();
}

void SettingsDlg::slotDefault()
{
    // Defaults hard coded.  Make sure that this is alright.
    // If defaults are set anywhere else, like in setting up config file, make sure the same.
    KPageWidgetItem *current = currentPage();
    if ( current ==  pageGeneral )
    {
        m_GeneralWidgets.autosaveCB->setChecked( false );
        m_GeneralWidgets.timeISB->setValue( 5 );
        m_GeneralWidgets.timeISB->setEnabled( true );
        m_GeneralWidgets.loadlastCB->setChecked( true );
        m_GeneralWidgets.diagramKB->setCurrentIndex( 0 );
        m_GeneralWidgets.languageKB->setCurrentIndex( Uml::ProgrammingLanguage::Cpp );
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
        m_ClassWidgets.showVisibilityCB->setChecked( false );
        m_ClassWidgets.showAttsCB->setChecked( true );
        m_ClassWidgets.showOpsCB->setChecked( true );
        m_ClassWidgets.showStereotypeCB->setChecked( false );
        m_ClassWidgets.showAttSigCB->setChecked( false );
        m_ClassWidgets.showOpSigCB->setChecked( false );
        m_ClassWidgets.showPackageCB->setChecked( false );
        m_ClassWidgets.m_pAttribScopeCB->setCurrentIndex(1); // Private
        m_ClassWidgets.m_pOperationScopeCB->setCurrentIndex(0); // Public
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
}

void SettingsDlg::applyPage( KPageWidgetItem*item )
{
    m_bChangesApplied = true;
    if ( item == pageGeneral )
    {
        m_pOptionState->generalState.undo = m_GeneralWidgets.undoCB->isChecked();
        m_pOptionState->generalState.tabdiagrams = m_GeneralWidgets.tabdiagramsCB->isChecked();
        m_pOptionState->generalState.newcodegen = m_GeneralWidgets.newcodegenCB->isChecked();
        m_pOptionState->generalState.angularlines = m_GeneralWidgets.angularLinesCB->isChecked();
        m_pOptionState->generalState.footerPrinting = m_GeneralWidgets.footerPrintingCB->isChecked();
        m_pOptionState->generalState.autosave = m_GeneralWidgets.autosaveCB->isChecked();
        m_pOptionState->generalState.autosavetime = m_GeneralWidgets.timeISB->value();
        // retrieve Suffix setting from dialog entry
        m_pOptionState->generalState.autosavesuffix = m_GeneralWidgets.autosaveSuffixT->text();
        m_pOptionState->generalState.loadlast = m_GeneralWidgets.loadlastCB->isChecked();
        m_pOptionState->generalState.diagram  = Uml::DiagramType::Value(m_GeneralWidgets.diagramKB->currentIndex() + 1);
        m_pOptionState->generalState.defaultLanguage = Uml::ProgrammingLanguage::Value( m_GeneralWidgets.languageKB->currentIndex());
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
        m_pOptionState->classState.showVisibility = m_ClassWidgets.showVisibilityCB->isChecked();
        m_pOptionState->classState.showAtts = m_ClassWidgets.showAttsCB->isChecked();
        m_pOptionState->classState.showOps = m_ClassWidgets.showOpsCB->isChecked();
        m_pOptionState->classState.showStereoType = m_ClassWidgets.showStereotypeCB->isChecked();
        m_pOptionState->classState.showAttSig = m_ClassWidgets.showAttSigCB->isChecked();
        m_pOptionState->classState.showOpSig = m_ClassWidgets.showOpSigCB->isChecked();
        m_pOptionState->classState.showPackage = m_ClassWidgets.showPackageCB->isChecked();
        m_pOptionState->classState.defaultAttributeScope = (Uml::Visibility::Value) m_ClassWidgets.m_pAttribScopeCB->currentIndex();
        m_pOptionState->classState.defaultOperationScope = (Uml::Visibility::Value) m_ClassWidgets.m_pOperationScopeCB->currentIndex();
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

void SettingsDlg::slotAutosaveCBClicked()
{
    m_GeneralWidgets.timeISB->setEnabled( m_GeneralWidgets.autosaveCB->isChecked() );
}

QString SettingsDlg::getCodeGenerationLanguage()
{
    return m_pCodeGenPage->getLanguage();
}

#include "settingsdlg.moc"
