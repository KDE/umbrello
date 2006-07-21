/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "settingsdlg.h"

// qt includes
#include <qlayout.h>
#include <qtooltip.h>
#include <kvbox.h>
//Added by qt3to4:
#include <QLabel>
#include <QGridLayout>
#include <kicon.h>
// kde includes
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kfiledialog.h>
// app includes
#include "codegenerationoptionspage.h"
#include "codevieweroptionspage.h"
#include "../dialog_utils.h"

SettingsDlg::SettingsDlg( QWidget * parent, Settings::OptionState *state )
        : KPageDialog( parent) {
    setCaption(i18n("Umbrello Setup") );
    setButtons( Help | Default | Apply | Ok | Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );
    setFaceType( KPageDialog::List );
    m_bChangesApplied = false;
    m_pOptionState = state;
    setHelp( "umbrello/index.html", QString::null );
    setupGeneralPage();
    setupFontPage();
    setupUIPage();
    setupClassPage();
    setupCodeGenPage();
    setupCodeViewerPage(state->codeViewerState);
}

SettingsDlg::~SettingsDlg() {}

void SettingsDlg::setupUIPage() {
    //setup UI page

    KVBox * page = new KVBox();
    pageUserInterface = new KPageWidgetItem( page,i18n("User Interface"));
    pageUserInterface->setHeader( i18n("User Interface Settings") );
    pageUserInterface->setIcon( KIcon(DesktopIcon( "window_list") ));
    addPage( pageUserInterface );


    m_UiWidgets.colorGB = new Q3GroupBox( i18n("Color"), page );
    QGridLayout * colorLayout = new QGridLayout( m_UiWidgets.colorGB );
    colorLayout -> setSpacing( spacingHint() );
    colorLayout -> setMargin( fontMetrics().height() );

    m_UiWidgets.lineColorL = new QLabel( i18n("Line color:"), m_UiWidgets.colorGB );
    colorLayout -> addWidget( m_UiWidgets.lineColorL, 0, 0 );

    m_UiWidgets.lineColorB = new KColorButton( m_pOptionState->uiState.lineColor ,m_UiWidgets.colorGB);
    //m_UiWidgets.lineColorB->setObjectName( m_UiWidgets.colorGB );
    colorLayout -> addWidget( m_UiWidgets.lineColorB, 0, 1 );

    m_UiWidgets.lineDefaultB = new QPushButton( i18n("D&efault Color"), m_UiWidgets.colorGB );
    colorLayout -> addWidget( m_UiWidgets.lineDefaultB, 0, 2 );

    m_UiWidgets.fillColorL = new QLabel( i18n("Fill color:"), m_UiWidgets.colorGB );
    colorLayout -> addWidget( m_UiWidgets.fillColorL, 1, 0 );

    m_UiWidgets.fillColorB = new KColorButton( m_pOptionState->uiState.fillColor,m_UiWidgets.colorGB  );
    //m_UiWidgets.fillColorB->setObjectName( m_UiWidgets.colorGB );
    colorLayout -> addWidget( m_UiWidgets.fillColorB, 1, 1 );

    m_UiWidgets.fillDefaultB = new QPushButton( i18n("De&fault Color"), m_UiWidgets.colorGB );
    colorLayout -> addWidget( m_UiWidgets.fillDefaultB, 1, 2 );


    m_UiWidgets.lineWidthL = new QLabel( i18n("Line width:"), m_UiWidgets.colorGB );
    colorLayout -> addWidget( m_UiWidgets.lineWidthL, 2, 0 );

    // Low-Limit: 0, High-Limit: 10, Step: 1, Initial-Val: m_pOptionState->uiState.lineWidth
    // Number-Base: 10 ( decimal ), Parent: m_UiWidgets.colorGB
    m_UiWidgets.lineWidthB = new KIntSpinBox( 0, 10, 1, m_pOptionState->uiState.lineWidth, m_UiWidgets.colorGB );
    colorLayout -> addWidget( m_UiWidgets.lineWidthB, 2, 1 );

    m_UiWidgets.lineWidthDefaultB = new QPushButton( i18n("D&efault Width"), m_UiWidgets.colorGB );
    colorLayout -> addWidget( m_UiWidgets.lineWidthDefaultB, 2, 2 );



    m_UiWidgets.useFillColorCB = new QCheckBox( i18n("&Use fill color"), m_UiWidgets.colorGB );
    colorLayout -> setRowStretch( 3, 2 );
    colorLayout -> addWidget( m_UiWidgets.useFillColorCB, 3, 0 );
    m_UiWidgets.useFillColorCB -> setChecked( m_pOptionState->uiState.useFillColor );

    //connect button signals up
    connect( m_UiWidgets.lineDefaultB, SIGNAL(clicked()), this, SLOT(slotLineBClicked()) );
    connect( m_UiWidgets.fillDefaultB, SIGNAL(clicked()), this, SLOT(slotFillBClicked()) );
}

void SettingsDlg::setupGeneralPage() {
    //setup General page

    KVBox * page = new KVBox();
    pageGeneral = new KPageWidgetItem( page,i18n("General"));
    pageGeneral->setHeader( i18n("General Settings") );
    pageGeneral->setIcon( KIcon( DesktopIcon( "misc") ));
    addPage( pageGeneral );

    // Set up undo setting
    m_GeneralWidgets.miscGB = new Q3GroupBox( i18n("Miscellaneous"), page );

    QGridLayout * miscLayout = new QGridLayout( m_GeneralWidgets.miscGB );
    miscLayout -> setSpacing( spacingHint() );
    miscLayout -> setMargin( fontMetrics().height() );

    m_GeneralWidgets.undoCB = new QCheckBox( i18n("Enable undo"), m_GeneralWidgets.miscGB );
    m_GeneralWidgets.undoCB -> setChecked( m_pOptionState->generalState.undo );
    miscLayout -> addWidget( m_GeneralWidgets.undoCB, 0, 0 );

    m_GeneralWidgets.tabdiagramsCB = new QCheckBox( i18n("Use tabbed diagrams"), m_GeneralWidgets.miscGB );
    m_GeneralWidgets.tabdiagramsCB -> setChecked( m_pOptionState->generalState.tabdiagrams );
    miscLayout -> addWidget( m_GeneralWidgets.tabdiagramsCB, 0, 1 );

    m_GeneralWidgets.newcodegenCB = new QCheckBox( i18n("Use new C++/Java/Ruby generators"), m_GeneralWidgets.miscGB );
    m_GeneralWidgets.newcodegenCB -> setChecked( m_pOptionState->generalState.newcodegen );
    miscLayout -> addWidget( m_GeneralWidgets.newcodegenCB, 1, 0 );

    m_GeneralWidgets.angularLinesCB = new QCheckBox( i18n("Use angular association lines"), m_GeneralWidgets.miscGB );
    m_GeneralWidgets.angularLinesCB -> setChecked( m_pOptionState->generalState.angularlines );
    miscLayout -> addWidget( m_GeneralWidgets.angularLinesCB, 1, 1 );

    //setup autosave settings

    m_GeneralWidgets.autosaveGB = new Q3GroupBox( i18n("Autosave"), page );

    QGridLayout * autosaveLayout = new QGridLayout( m_GeneralWidgets.autosaveGB );
    autosaveLayout -> setSpacing( spacingHint() );
    autosaveLayout -> setMargin( fontMetrics().height() );

    m_GeneralWidgets.autosaveCB = new QCheckBox( i18n("E&nable autosave"), m_GeneralWidgets.autosaveGB );
    m_GeneralWidgets.autosaveCB -> setChecked( m_pOptionState->generalState.autosave );
    autosaveLayout -> addWidget( m_GeneralWidgets.autosaveCB, 0, 0 );

    m_GeneralWidgets.autosaveL = new QLabel( i18n("Select auto-save time interval (mins):"), m_GeneralWidgets.autosaveGB );
    autosaveLayout -> addWidget( m_GeneralWidgets.autosaveL, 1, 0 );

    m_GeneralWidgets.timeISB = new KIntSpinBox( 1, 600, 1, m_pOptionState->generalState.autosavetime, m_GeneralWidgets.autosaveGB );
    m_GeneralWidgets.timeISB -> setEnabled( m_pOptionState->generalState.autosave );
    autosaveLayout -> addWidget( m_GeneralWidgets.timeISB, 1, 1 );

    // 2004-05-17 Achim Spangler: Allow definition of Suffix for autosave
    // ( default: ".xmi" )
    Dialog_Utils::makeLabeledEditField( m_GeneralWidgets.autosaveGB, autosaveLayout, 2,
                                    m_GeneralWidgets.autosaveSuffixL, i18n("Set autosave suffix:"),
                                    m_GeneralWidgets.autosaveSuffixT, m_pOptionState->generalState.autosavesuffix );
    QString autoSaveSuffixToolTip = i18n( "<qt><p>The autosave file will be saved to ~/autosave.xmi if the autosaving occurs "
                                          "before you have manually saved the file.</p>"
                                          "<p>If you've already saved it, the autosave file will be saved in the same folder as the file "
                                          "and will be named like the file's name, followed by the suffix specified.</p>"
                                          "<p>If the suffix is equal to the suffix of the file you've saved, "
                                          "the autosave will overwrite your file automatically.</p></qt>" );
    QToolTip::add( m_GeneralWidgets.autosaveSuffixL, autoSaveSuffixToolTip );
    QToolTip::add( m_GeneralWidgets.autosaveSuffixT, autoSaveSuffixToolTip );

    //setup startup settings
    m_GeneralWidgets.startupGB = new Q3GroupBox( i18n("Startup"), page );

    QGridLayout * startupLayout = new QGridLayout( m_GeneralWidgets.startupGB );
    startupLayout -> setSpacing( spacingHint() );
    startupLayout -> setMargin( fontMetrics().height() );

    m_GeneralWidgets.logoCB = new QCheckBox( i18n("Sta&rtup logo"), m_GeneralWidgets.startupGB );
    m_GeneralWidgets.logoCB -> setChecked( m_pOptionState->generalState.logo );
    startupLayout -> addWidget( m_GeneralWidgets.logoCB, 0, 0 );

    m_GeneralWidgets.tipCB = new QCheckBox( i18n("&Tip of the day"), m_GeneralWidgets.startupGB );
    m_GeneralWidgets.tipCB -> setChecked( m_pOptionState->generalState.tip );
    startupLayout -> addWidget( m_GeneralWidgets.tipCB, 0, 1 );

    m_GeneralWidgets.loadlastCB = new QCheckBox( i18n("&Load last project"), m_GeneralWidgets.startupGB );
    m_GeneralWidgets.loadlastCB -> setChecked( m_pOptionState->generalState.loadlast );
    startupLayout -> addWidget( m_GeneralWidgets.loadlastCB, 1, 0 );

    m_GeneralWidgets.startL = new QLabel( i18n("Start new project with:"), m_GeneralWidgets.startupGB );
    startupLayout -> addWidget( m_GeneralWidgets.startL, 2, 0 );

    m_GeneralWidgets.diagramKB = new KComboBox( m_GeneralWidgets.startupGB );
    m_GeneralWidgets.diagramKB->setCompletionMode( KGlobalSettings::CompletionPopup );
    startupLayout -> addWidget( m_GeneralWidgets.diagramKB, 2, 1 );

    QString diagrams [] = { i18n("No Diagram"), i18n("Class Diagram"),
                            i18n("Use Case Diagram"), i18n("Sequence Diagram"),
                            i18n("Collaboration Diagram"), i18n("State Diagram"),
                            i18n("Activity Diagram"), i18n("Component Diagram"),
                            i18n("Deployment Diagram") };

    //start at 1 because we don't allow No Diagram any more
    for (int i=1; i<9; i++) {
        insertDiagram( diagrams[i] );
    }

    m_GeneralWidgets.diagramKB->setCurrentIndex( (int)m_pOptionState->generalState.diagram-1 );
    connect( m_GeneralWidgets.autosaveCB, SIGNAL(clicked()), this, SLOT(slotAutosaveCBClicked()) );
}

/**
* Inserts @p type into the type-combobox as well as its completion object.
*/
void SettingsDlg::insertDiagram( const QString& type, int index )
{
    m_GeneralWidgets.diagramKB->insertItem( type, index );
    m_GeneralWidgets.diagramKB->completionObject()->addItem( type );
}

void SettingsDlg::setupClassPage() {
    //setup class settings page
    KVBox * page = new KVBox();
    pageClass = new KPageWidgetItem( page,i18n("Class"));
    pageClass->setHeader( i18n("Class Settings") );
    pageGeneral->setIcon( KIcon( DesktopIcon( "edit") ));
    addPage( pageClass );

    m_ClassWidgets.visibilityGB = new Q3GroupBox( i18n("Visibility"), page );

    QGridLayout * visibilityLayout = new QGridLayout( m_ClassWidgets.visibilityGB );
    visibilityLayout -> setSpacing( spacingHint() );
    visibilityLayout -> setMargin(  fontMetrics().height()  );

    m_ClassWidgets.showVisibilityCB = new QCheckBox(i18n("Show &visibility"), m_ClassWidgets.visibilityGB);
    m_ClassWidgets.showVisibilityCB -> setChecked(  m_pOptionState->classState.showVisibility );
    visibilityLayout -> addWidget( m_ClassWidgets.showVisibilityCB, 0, 0 );

    m_ClassWidgets.showAttsCB = new QCheckBox( i18n("Show attributes"), m_ClassWidgets.visibilityGB );
    m_ClassWidgets.showAttsCB -> setChecked(  m_pOptionState->classState.showAtts );
    visibilityLayout -> addWidget( m_ClassWidgets.showAttsCB, 0, 1 );

    m_ClassWidgets.showOpsCB = new QCheckBox( i18n("Show operations"), m_ClassWidgets.visibilityGB );
    m_ClassWidgets.showOpsCB -> setChecked(  m_pOptionState->classState.showOps );
    visibilityLayout -> addWidget( m_ClassWidgets.showOpsCB, 1, 0 );

    m_ClassWidgets.showStereotypeCB = new QCheckBox( i18n("Show stereot&ype"), m_ClassWidgets.visibilityGB );
    m_ClassWidgets.showStereotypeCB -> setChecked(  m_pOptionState->classState.showStereoType );
    visibilityLayout -> addWidget( m_ClassWidgets.showStereotypeCB, 1, 1 );

    m_ClassWidgets.showAttSigCB = new QCheckBox(i18n("Show attribute signature"), m_ClassWidgets.visibilityGB);
    m_ClassWidgets.showAttSigCB -> setChecked(   m_pOptionState->classState.showAttSig );
    visibilityLayout -> addWidget( m_ClassWidgets.showAttSigCB, 2, 0 );


    m_ClassWidgets.showPackageCB = new QCheckBox(i18n("Show package"), m_ClassWidgets.visibilityGB);
    m_ClassWidgets.showPackageCB -> setChecked(  m_pOptionState->classState.showPackage );
    visibilityLayout -> addWidget( m_ClassWidgets.showPackageCB, 2, 1 );

    m_ClassWidgets.showOpSigCB = new QCheckBox( i18n("Show operation signature"), m_ClassWidgets.visibilityGB );
    m_ClassWidgets.showOpSigCB -> setChecked(  m_pOptionState->classState.showOpSig );
    visibilityLayout -> addWidget( m_ClassWidgets.showOpSigCB, 3, 0 );
    visibilityLayout -> setRowStretch( 3, 1 );

    m_ClassWidgets.scopeGB = new Q3GroupBox( i18n("Starting Scope"), page );
    QGridLayout * scopeLayout = new QGridLayout( m_ClassWidgets.scopeGB );
    scopeLayout -> setSpacing( spacingHint() );
    scopeLayout -> setMargin(  fontMetrics().height()  );

    m_ClassWidgets.attributeLabel = new QLabel( i18n("Default attribute scope:"), m_ClassWidgets.scopeGB);
    scopeLayout -> addWidget( m_ClassWidgets.attributeLabel, 0, 0 );

    m_ClassWidgets.operationLabel = new QLabel( i18n("Default operation scope:"), m_ClassWidgets.scopeGB);
    scopeLayout -> addWidget( m_ClassWidgets.operationLabel, 1, 0 );

    m_ClassWidgets.m_pAttribScopeCB = new KComboBox(m_ClassWidgets.scopeGB);
    insertAttribScope( tr2i18n( "Public" ) );
    insertAttribScope( tr2i18n( "Private" ) );
    insertAttribScope( tr2i18n( "Protected" ) );
    m_ClassWidgets.m_pAttribScopeCB->setCurrentIndex((m_pOptionState->classState.defaultAttributeScope - 200));
    m_ClassWidgets.m_pAttribScopeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
    scopeLayout -> addWidget( m_ClassWidgets.m_pAttribScopeCB, 0, 1 );

    m_ClassWidgets.m_pOperationScopeCB = new KComboBox(m_ClassWidgets.scopeGB);
    insertOperationScope( tr2i18n( "Public" ) );
    insertOperationScope( tr2i18n( "Private" ) );
    insertOperationScope( tr2i18n( "Protected" ) );
    m_ClassWidgets.m_pOperationScopeCB->setCurrentIndex((m_pOptionState->classState.defaultOperationScope - 200));
    m_ClassWidgets.m_pOperationScopeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
    scopeLayout -> addWidget( m_ClassWidgets.m_pOperationScopeCB, 1, 1 );

}
/**
* Inserts @p type into the type-combobox as well as its completion object.
*/
void SettingsDlg::insertAttribScope( const QString& type, int index )
{
    m_ClassWidgets.m_pAttribScopeCB->insertItem( type, index );
    m_ClassWidgets.m_pAttribScopeCB->completionObject()->addItem( type );
}
/**
* Inserts @p type into the type-combobox as well as its completion object.
*/
void SettingsDlg::insertOperationScope( const QString& type, int index )
{
    m_ClassWidgets.m_pOperationScopeCB->insertItem( type, index );
    m_ClassWidgets.m_pOperationScopeCB->completionObject()->addItem( type );
}

void SettingsDlg::setupCodeGenPage() {
    //setup code generation settings page
    KVBox * page = new KVBox();
    pageCodeGen = new KPageWidgetItem( page,i18n("Code Generation") );
    pageCodeGen->setHeader( i18n("Code Generation Settings") );
    pageCodeGen->setIcon( KIcon( DesktopIcon( "source") ));
    addPage( pageCodeGen );
    m_pCodeGenPage = new CodeGenerationOptionsPage(page);
    connect( m_pCodeGenPage, SIGNAL(languageChanged()), this, SLOT(slotApply()) );
}

void SettingsDlg::setupCodeViewerPage(Settings::CodeViewerState options) {
    //setup code generation settings page
    KVBox * page = new KVBox();
    pageCodeViewer = new KPageWidgetItem( page,i18n("Code Viewer")  );
    pageCodeViewer->setHeader( i18n("Code Viewer Settings") );
    pageCodeViewer->setIcon( KIcon( DesktopIcon( "source") ) );
    addPage( pageCodeViewer );
    m_pCodeViewerPage = new CodeViewerOptionsPage(options, page);
}

void SettingsDlg::setupFontPage() {
    KVBox * page = new KVBox();
    pageFont = new KPageWidgetItem( page,i18n("Font")  );
    pageFont->setHeader( i18n("Font Settings") );
    pageFont->setIcon( KIcon( DesktopIcon( "fonts") ) );
    addPage( pageFont );
    m_FontWidgets.chooser = new KFontChooser( page,  false, QStringList(), false);
    m_FontWidgets.chooser->setFont( m_pOptionState->uiState.font );

}

void SettingsDlg::slotApply() {
    applyPage( currentPage() );
    emit applyClicked();
}

void SettingsDlg::slotOk() {
    KPageWidgetItem*pageCodeViewer,*pageFont,*pageCodeGen,*pageUserInterface,* pageGeneral,*pageClass;

    applyPage( pageClass);
    applyPage( pageGeneral);
    applyPage( pageUserInterface );
    applyPage( pageCodeViewer);
    applyPage( pageCodeGen );
    applyPage( pageFont );
    accept();
}


void SettingsDlg::slotDefault() {
    /*
       Defaults hard coded.  Make sure that this is alright.
       If defaults are set anywhere else, like in setting up config file, make sure the same.
    */
    KPageWidgetItem *current = currentPage();
    if ( current ==  pageGeneral )
    {
        m_GeneralWidgets.autosaveCB -> setChecked( false );
        m_GeneralWidgets.timeISB -> setValue( 5 );
        m_GeneralWidgets.timeISB->setEnabled( true );
        m_GeneralWidgets.logoCB -> setChecked( true );
        m_GeneralWidgets.tipCB -> setChecked( true );
        m_GeneralWidgets.loadlastCB -> setChecked( true );
        m_GeneralWidgets.diagramKB -> setCurrentItem( 0 );
    }
    else if ( current == pageFont )
    {
        m_FontWidgets.chooser -> setFont( parentWidget() -> font() );
    }
    else if ( current == pageUserInterface )
    {
        m_UiWidgets.useFillColorCB -> setChecked( true );
        m_UiWidgets.fillColorB -> setColor( QColor( 255, 255, 192 ) );
        m_UiWidgets.lineColorB -> setColor( Qt::red );
        m_UiWidgets.lineWidthB -> setValue( 0 );
    }
    else if ( current == pageClass )
    {
        m_ClassWidgets.showVisibilityCB -> setChecked( false );
        m_ClassWidgets.showAttsCB -> setChecked( true );
        m_ClassWidgets.showOpsCB -> setChecked( true );
        m_ClassWidgets.showStereotypeCB -> setChecked( false );
        m_ClassWidgets.showAttSigCB -> setChecked( false );
        m_ClassWidgets.showOpSigCB -> setChecked( false );
        m_ClassWidgets.showPackageCB -> setChecked( false );
        m_ClassWidgets.m_pAttribScopeCB->setCurrentIndex(1); // Private
        m_ClassWidgets.m_pOperationScopeCB->setCurrentItem(0); // Public
    }
    else if (  current == pageCodeGen || current == pageCodeViewer )
    {
    }
}

void SettingsDlg::applyPage( KPageWidgetItem*item ) {
    m_bChangesApplied = true;
    if ( item == pageGeneral )
    {
        m_pOptionState->generalState.undo = m_GeneralWidgets.undoCB -> isChecked();
        m_pOptionState->generalState.tabdiagrams = m_GeneralWidgets.tabdiagramsCB->isChecked();
        m_pOptionState->generalState.newcodegen = m_GeneralWidgets.newcodegenCB->isChecked();
        m_pOptionState->generalState.angularlines = m_GeneralWidgets.angularLinesCB->isChecked();
        m_pOptionState->generalState.autosave = m_GeneralWidgets.autosaveCB -> isChecked();
        m_pOptionState->generalState.autosavetime = m_GeneralWidgets.timeISB -> value();
        // 2004-05-17 Achim Spangler: retrieve Suffix setting from dialog entry
        m_pOptionState->generalState.autosavesuffix = m_GeneralWidgets.autosaveSuffixT -> text();
        m_pOptionState->generalState.logo = m_GeneralWidgets.logoCB -> isChecked();
        m_pOptionState->generalState.tip = m_GeneralWidgets.tipCB -> isChecked();
        m_pOptionState->generalState.loadlast = m_GeneralWidgets.loadlastCB -> isChecked();
        m_pOptionState->generalState.diagram  = (Settings::Diagram)(m_GeneralWidgets.diagramKB->currentItem() + 1);
    }
    else if ( item == pageFont )
    {
        m_pOptionState->uiState.font = m_FontWidgets.chooser -> font();
    }
    else if ( item == pageUserInterface )
    {
        m_pOptionState->uiState.useFillColor = m_UiWidgets.useFillColorCB -> isChecked();
        m_pOptionState->uiState.fillColor = m_UiWidgets.fillColorB -> color();
        m_pOptionState->uiState.lineColor = m_UiWidgets.lineColorB -> color();
        m_pOptionState->uiState.lineWidth = m_UiWidgets.lineWidthB -> value();
    }
    else if ( item == pageClass )
    {
        m_pOptionState->classState.showVisibility = m_ClassWidgets.showVisibilityCB -> isChecked();
        m_pOptionState->classState.showAtts = m_ClassWidgets.showAttsCB -> isChecked();
        m_pOptionState->classState.showOps = m_ClassWidgets.showOpsCB -> isChecked();
        m_pOptionState->classState.showStereoType = m_ClassWidgets.showStereotypeCB -> isChecked();
        m_pOptionState->classState.showAttSig = m_ClassWidgets.showAttSigCB -> isChecked();
        m_pOptionState->classState.showOpSig = m_ClassWidgets.showOpSigCB -> isChecked();
        m_pOptionState->classState.showPackage = m_ClassWidgets.showPackageCB -> isChecked();
            m_pOptionState->classState.defaultAttributeScope = (Uml::Visibility::Value) (m_ClassWidgets.m_pAttribScopeCB->currentItem() + 200);
            m_pOptionState->classState.defaultOperationScope = (Uml::Visibility::Value) (m_ClassWidgets.m_pOperationScopeCB->currentItem() + 200);
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

void SettingsDlg::slotLineBClicked() {
    m_UiWidgets.lineColorB -> setColor( Qt::red );
}

void SettingsDlg::slotFillBClicked() {
    m_UiWidgets.fillColorB -> setColor( QColor(255, 255, 192) );
}

void SettingsDlg::slotAutosaveCBClicked() {
    m_GeneralWidgets.timeISB -> setEnabled( m_GeneralWidgets.autosaveCB -> isChecked() );
}

QString SettingsDlg::getCodeGenerationLanguage() {
    return m_pCodeGenPage->getCodeGenerationLanguage();
}

#include "settingsdlg.moc"
