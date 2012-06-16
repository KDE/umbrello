/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "generaloptionpage.h"

// local includes
#include "dialog_utils.h"
#include "optionstate.h"

// kde includes
#include <KComboBox>
#include <KDialog>
#include <KIntSpinBox>
#include <KLineEdit>

// qt includes
#include <KLocale>
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QGridLayout>

/**
 * Constructor.general
 * @param parent   the parent (wizard) of this wizard page
 */
GeneralOptionPage::GeneralOptionPage(QWidget* parent)
  : QWidget(parent)
{
    Settings::OptionState &optionState = Settings::optionState();
    int spacingHint = static_cast<KDialog*>(parent)->spacingHint();
    // Set up undo setting
    m_GeneralWidgets.miscGB = new QGroupBox( i18nc("miscellaneous group box", "Miscellaneous"), parent );

    QGridLayout * miscLayout = new QGridLayout( m_GeneralWidgets.miscGB );
    miscLayout->setSpacing( spacingHint );
    miscLayout->setMargin( fontMetrics().height() );

    m_GeneralWidgets.undoCB = new QCheckBox( i18n("Enable undo"), m_GeneralWidgets.miscGB );
    m_GeneralWidgets.undoCB->setChecked( optionState.generalState.undo );
    miscLayout->addWidget( m_GeneralWidgets.undoCB, 0, 0 );

    m_GeneralWidgets.tabdiagramsCB = new QCheckBox( i18n("Use tabbed diagrams"), m_GeneralWidgets.miscGB );
    m_GeneralWidgets.tabdiagramsCB->setChecked( optionState.generalState.tabdiagrams );
    miscLayout->addWidget( m_GeneralWidgets.tabdiagramsCB, 0, 1 );

    m_GeneralWidgets.newcodegenCB = new QCheckBox( i18n("Use new C++/Java/Ruby generators"), m_GeneralWidgets.miscGB );
    m_GeneralWidgets.newcodegenCB->setChecked( optionState.generalState.newcodegen );
    miscLayout->addWidget( m_GeneralWidgets.newcodegenCB, 1, 0 );

    m_GeneralWidgets.angularLinesCB = new QCheckBox( i18n("Use angular association lines"), m_GeneralWidgets.miscGB );
    m_GeneralWidgets.angularLinesCB->setChecked( optionState.generalState.angularlines );
    miscLayout->addWidget( m_GeneralWidgets.angularLinesCB, 1, 1 );

    m_GeneralWidgets.footerPrintingCB = new QCheckBox( i18n("Turn on footer and page numbers when printing"), m_GeneralWidgets.miscGB );
    m_GeneralWidgets.footerPrintingCB->setChecked( optionState.generalState.footerPrinting );
    miscLayout->addWidget( m_GeneralWidgets.footerPrintingCB, 2, 0 );

    //setup autosave settings
    m_GeneralWidgets.autosaveGB = new QGroupBox( i18n("Autosave"), parent );

    QGridLayout * autosaveLayout = new QGridLayout( m_GeneralWidgets.autosaveGB );
    autosaveLayout->setSpacing( spacingHint );
    autosaveLayout->setMargin( fontMetrics().height() );

    m_GeneralWidgets.autosaveCB = new QCheckBox( i18n("E&nable autosave"), m_GeneralWidgets.autosaveGB );
    m_GeneralWidgets.autosaveCB->setChecked( optionState.generalState.autosave );
    autosaveLayout->addWidget( m_GeneralWidgets.autosaveCB, 0, 0 );

    m_GeneralWidgets.autosaveL = new QLabel( i18n("Select auto-save time interval (mins):"), m_GeneralWidgets.autosaveGB );
    autosaveLayout->addWidget( m_GeneralWidgets.autosaveL, 1, 0 );

    m_GeneralWidgets.timeISB = new KIntSpinBox( 1, 600, 1, optionState.generalState.autosavetime, m_GeneralWidgets.autosaveGB );
    m_GeneralWidgets.timeISB->setEnabled( optionState.generalState.autosave );
    autosaveLayout->addWidget( m_GeneralWidgets.timeISB, 1, 1 );

    // Allow definition of Suffix for autosave ( default: ".xmi" )generator
    Dialog_Utils::makeLabeledEditField( m_GeneralWidgets.autosaveGB, autosaveLayout, 2,
                                        m_GeneralWidgets.autosaveSuffixL, i18n("Set autosave suffix:"),
                                        m_GeneralWidgets.autosaveSuffixT, optionState.generalState.autosavesuffix );
                                        QString autoSaveSuffixToolTip = i18n( "<qt><p>The autosave file will be saved to ~/autosave.xmi if the autosaving occurs "
                                        "before you have manually saved the file.</p>"
                                        "<p>If you have already saved it, the autosave file will be saved in the same folder as the file "
                                        "and will be named like the file's name, followed by the suffix specified.</p>"
                                        "<p>If the suffix is equal to the suffix of the file you have saved, "
                                        "the autosave will overwrite your file automatically.</p></qt>" );
    m_GeneralWidgets.autosaveSuffixL->setToolTip( autoSaveSuffixToolTip );
    m_GeneralWidgets.autosaveSuffixT->setToolTip( autoSaveSuffixToolTip );

    //setup startup settings
    m_GeneralWidgets.startupGB = new QGroupBox( i18n("Startup"), parent );

    QGridLayout * startupLayout = new QGridLayout( m_GeneralWidgets.startupGB );
    startupLayout->setSpacing( spacingHint );
    startupLayout->setMargin( fontMetrics().height() );

    m_GeneralWidgets.loadlastCB = new QCheckBox( i18n("&Load last project"), m_GeneralWidgets.startupGB );
    m_GeneralWidgets.loadlastCB->setChecked( optionState.generalState.loadlast );
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

    m_GeneralWidgets.diagramKB->setCurrentIndex( (int)optionState.generalState.diagram-1 );
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
    m_GeneralWidgets.languageKB->setCurrentIndex(optionState.generalState.defaultLanguage);
}

/**
 * destructor
 */
GeneralOptionPage::~GeneralOptionPage()
{
}

/**
 * sets default values
 */
void GeneralOptionPage::setDefaults()
{
    m_GeneralWidgets.autosaveCB->setChecked( false );
    m_GeneralWidgets.timeISB->setValue( 5 );
    m_GeneralWidgets.timeISB->setEnabled( true );
    m_GeneralWidgets.loadlastCB->setChecked( true );
    m_GeneralWidgets.diagramKB->setCurrentIndex( 0 );
    m_GeneralWidgets.languageKB->setCurrentIndex( Uml::ProgrammingLanguage::Cpp );
}

/**
 * Reads the set values from their corresponding widgets, writes them back to
 * the data structure, and notifies clients.
 */
void GeneralOptionPage::apply()
{
    Settings::OptionState &optionState = Settings::optionState();
    optionState.generalState.undo = m_GeneralWidgets.undoCB->isChecked();
    optionState.generalState.tabdiagrams = m_GeneralWidgets.tabdiagramsCB->isChecked();
    optionState.generalState.newcodegen = m_GeneralWidgets.newcodegenCB->isChecked();
    optionState.generalState.angularlines = m_GeneralWidgets.angularLinesCB->isChecked();
    optionState.generalState.footerPrinting = m_GeneralWidgets.footerPrintingCB->isChecked();
    optionState.generalState.autosave = m_GeneralWidgets.autosaveCB->isChecked();
    optionState.generalState.autosavetime = m_GeneralWidgets.timeISB->value();
    // retrieve Suffix setting from dialog entry
    optionState.generalState.autosavesuffix = m_GeneralWidgets.autosaveSuffixT->text();
    optionState.generalState.loadlast = m_GeneralWidgets.loadlastCB->isChecked();
    optionState.generalState.diagram  = Uml::DiagramType::Value(m_GeneralWidgets.diagramKB->currentIndex() + 1);
    optionState.generalState.defaultLanguage = Uml::ProgrammingLanguage::Value( m_GeneralWidgets.languageKB->currentIndex());
    emit applyClicked();
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 */
void GeneralOptionPage::insertDiagram( const QString& type, int index )
{
    m_GeneralWidgets.diagramKB->insertItem( index, type );
    m_GeneralWidgets.diagramKB->completionObject()->addItem( type );
}

void GeneralOptionPage::slotAutosaveCBClicked()
{
    m_GeneralWidgets.timeISB->setEnabled( m_GeneralWidgets.autosaveCB->isChecked() );
}
