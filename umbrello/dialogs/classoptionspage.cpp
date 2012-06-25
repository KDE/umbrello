/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "classoptionspage.h"

// local includes
#include "umlview.h"
#include "classifierwidget.h"
#include "widgetbase.h"

// kde includes
#include <klocale.h>
#include <KComboBox>

// qt includes
#include <QtGui/QCheckBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>

/**
 * Constructor - observe and modify a Widget
 */
ClassOptionsPage::ClassOptionsPage(QWidget* pParent, ClassifierWidget* pWidget)
  : QWidget(pParent)
{
    init();
    //WidgetType type = pWidget->baseType();
    m_pWidget = pWidget;
    setupPage();
}

/**
 * Constructor - observe and modify an OptionState structure
 *
 * @param parent Parent widget
 * @param options Settings to read from/save into
 * @param isDiagram Flag if object is for display diagram class options
 */
ClassOptionsPage::ClassOptionsPage(QWidget* pParent, Settings::OptionState *options, bool isDiagram)
  : QWidget(pParent),
    m_isDiagram(isDiagram)
{
    init();
    m_options = options;
    setupClassPageOption();
}

/**
 * Destructor
 */
ClassOptionsPage::~ClassOptionsPage()
{
}

void ClassOptionsPage::setDefaults()
{
    m_showVisibilityCB->setChecked( false );
    m_showAttsCB->setChecked( true );
    m_showOpsCB->setChecked( true );
    m_showStereotypeCB->setChecked( false );
    m_showAttSigCB->setChecked( false );
    m_showOpSigCB->setChecked( false );
    m_showPackageCB->setChecked( false );
    m_attribScopeCB->setCurrentIndex(1); // Private
    m_operationScopeCB->setCurrentIndex(0); // Public
}

/**
 * apply changes
 */
void ClassOptionsPage::apply()
{
    if (m_pWidget) {
        applyWidget();
    } else if (m_options) {
        applyOptionState();
    }
}

/**
 * Set related uml widget
 */
void ClassOptionsPage::setWidget( ClassifierWidget * pWidget )
{
    m_pWidget = pWidget;
}

/**
 * Creates the page with the correct options for the class/interface
 */
void ClassOptionsPage::setupPage()
{
    int margin = fontMetrics().height();

    bool sig = false;
    Uml::SignatureType sigtype;

    QVBoxLayout * topLayout = new QVBoxLayout(this);

    topLayout->setSpacing(6);
    m_visibilityGB = new QGroupBox(i18n("Show"), this);
    topLayout->addWidget(m_visibilityGB);
    QGridLayout * visibilityLayout = new QGridLayout(m_visibilityGB);
    visibilityLayout->setSpacing(10);
    visibilityLayout->setMargin(margin);
    visibilityLayout->setRowStretch(3, 1);

    m_showOpsCB = new QCheckBox(i18n("Operatio&ns"), m_visibilityGB);
    m_showOpsCB->setChecked(m_pWidget->visualProperty(ClassifierWidget::ShowOperations));
    visibilityLayout->addWidget(m_showOpsCB, 0, 0);

    m_showVisibilityCB = new QCheckBox(i18n("&Visibility"), m_visibilityGB);
    m_showVisibilityCB->setChecked(m_pWidget->visualProperty(ClassifierWidget::ShowVisibility));
    visibilityLayout->addWidget(m_showVisibilityCB, 0, 1);

    sigtype = m_pWidget->operationSignature();
    if (sigtype == Uml::SignatureType::NoSig || sigtype == Uml::SignatureType::NoSigNoVis)
        sig = false;
    else
        sig = true;
    m_showOpSigCB = new QCheckBox(i18n("O&peration signature"), m_visibilityGB);
    m_showOpSigCB->setChecked(sig);
    visibilityLayout->addWidget(m_showOpSigCB, 1, 0);

    m_showPackageCB = new QCheckBox(i18n("Pac&kage"), m_visibilityGB);
    m_showPackageCB->setChecked(m_pWidget->visualProperty(ClassifierWidget::ShowPackage));
    visibilityLayout->addWidget(m_showPackageCB, 1, 1);

    WidgetBase::WidgetType type = m_pWidget->baseType();

    if (type == WidgetBase::wt_Class) {
        m_showAttsCB = new QCheckBox(i18n("Att&ributes"), m_visibilityGB);
        m_showAttsCB->setChecked(m_pWidget->visualProperty(ClassifierWidget::ShowAttributes));
        visibilityLayout->addWidget(m_showAttsCB, 2, 0);

        m_showStereotypeCB = new QCheckBox(i18n("Stereot&ype"), m_visibilityGB);
        m_showStereotypeCB->setChecked(m_pWidget->visualProperty(ClassifierWidget::ShowStereotype));
        visibilityLayout->addWidget(m_showStereotypeCB, 2, 1);

        m_showAttSigCB = new QCheckBox(i18n("Attr&ibute signature"), m_visibilityGB);
        sigtype = m_pWidget->attributeSignature();
        if (sigtype == Uml::SignatureType::NoSig || sigtype == Uml::SignatureType::NoSigNoVis)
            sig = false;
        else
            sig = true;
        m_showAttSigCB->setChecked(sig);
        visibilityLayout->addWidget(m_showAttSigCB, 3, 0);

        m_showPublicOnlyCB = new QCheckBox(i18n("&Public Only"), m_visibilityGB);
        m_showPublicOnlyCB->setChecked(m_pWidget->visualProperty(ClassifierWidget::ShowPublicOnly));
        visibilityLayout->addWidget(m_showPublicOnlyCB, 3, 1);


    } else if (type == WidgetBase::wt_Interface) {
        m_drawAsCircleCB = new QCheckBox(i18n("Draw as circle"), m_visibilityGB);
        m_drawAsCircleCB->setChecked( m_pWidget->visualProperty(ClassifierWidget::DrawAsCircle) );
        visibilityLayout->addWidget(m_drawAsCircleCB, 2, 0);
    }
}

/**
 * Creates the page based on the OptionState
 */
void ClassOptionsPage::setupClassPageOption()
{
    int margin = fontMetrics().height();

    QVBoxLayout * topLayout = new QVBoxLayout(this);

    topLayout->setSpacing(6);
    m_visibilityGB = new QGroupBox(i18n("Show"), this);
    topLayout->addWidget(m_visibilityGB);
    QGridLayout * visibilityLayout = new QGridLayout(m_visibilityGB);
    visibilityLayout->setSpacing(10);
    visibilityLayout->setMargin(margin);

    m_showOpsCB = new QCheckBox(i18n("Operatio&ns"), m_visibilityGB);
    m_showOpsCB->setChecked( m_options->classState.showOps );
    visibilityLayout->addWidget(m_showOpsCB, 0, 0);

    m_showOpSigCB = new QCheckBox(i18n("O&peration signature"), m_visibilityGB);
    m_showOpSigCB->setChecked(m_options->classState.showOpSig);
    visibilityLayout->addWidget(m_showOpSigCB, 1, 0);
    visibilityLayout->setRowStretch(3, 1);

    m_showAttsCB = new QCheckBox(i18n("Att&ributes"), m_visibilityGB);
    m_showAttsCB->setChecked(m_options->classState.showAtts );
    visibilityLayout->addWidget(m_showAttsCB, 2, 0);

    m_showAttSigCB = new QCheckBox(i18n("Attr&ibute signature"), m_visibilityGB);
    m_showAttSigCB->setChecked(m_options->classState.showAttSig);
    visibilityLayout->addWidget(m_showAttSigCB, 3, 0);

    m_showVisibilityCB = new QCheckBox(i18n("&Visibility"), m_visibilityGB);
    m_showVisibilityCB->setChecked(m_options->classState.showVisibility);
    visibilityLayout->addWidget(m_showVisibilityCB, 0, 1);

    m_showPackageCB = new QCheckBox(i18n("Pac&kage"), m_visibilityGB);
    m_showPackageCB->setChecked(m_options->classState.showPackage);
    visibilityLayout->addWidget(m_showPackageCB, 1, 1);

    m_showStereotypeCB = new QCheckBox(i18n("Stereot&ype"), m_visibilityGB);
    m_showStereotypeCB->setChecked(m_options->classState.showStereoType);
    visibilityLayout->addWidget(m_showStereotypeCB, 2, 1);

    m_showAttribAssocsCB = new QCheckBox(i18n("&Attribute associations"), m_visibilityGB);
    m_showAttribAssocsCB->setChecked(m_options->classState.showAttribAssocs);
    visibilityLayout->addWidget(m_showAttribAssocsCB, 3, 1);

    m_showPublicOnlyCB = new QCheckBox(i18n("&Public Only"), m_visibilityGB);
    m_showPublicOnlyCB->setChecked(m_options->classState.showPublicOnly);
    visibilityLayout->addWidget(m_showPublicOnlyCB, 4, 1);

    if (!m_isDiagram) {
        m_scopeGB = new QGroupBox( i18n("Starting Scope"), parentWidget() );
        QGridLayout * scopeLayout = new QGridLayout( m_scopeGB );
        scopeLayout->setSpacing( 10 );
        scopeLayout->setMargin(  fontMetrics().height()  );

        m_attributeLabel = new QLabel( i18n("Default attribute scope:"), m_scopeGB);
        scopeLayout->addWidget( m_attributeLabel, 0, 0 );

        m_operationLabel = new QLabel( i18n("Default operation scope:"), m_scopeGB);
        scopeLayout->addWidget( m_operationLabel, 1, 0 );

        m_attribScopeCB = new KComboBox(m_scopeGB);
        insertAttribScope( tr2i18n( "Public" ) );
        insertAttribScope( tr2i18n( "Private" ) );
        insertAttribScope( tr2i18n( "Protected" ) );
        m_attribScopeCB->setCurrentIndex(m_options->classState.defaultAttributeScope);
        m_attribScopeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
        scopeLayout->addWidget( m_attribScopeCB, 0, 1 );

        m_operationScopeCB = new KComboBox(m_scopeGB);
        insertOperationScope( tr2i18n( "Public" ) );
        insertOperationScope( tr2i18n( "Private" ) );
        insertOperationScope( tr2i18n( "Protected" ) );
        m_operationScopeCB->setCurrentIndex(m_options->classState.defaultOperationScope);
        m_operationScopeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
        scopeLayout->addWidget( m_operationScopeCB, 1, 1 );
    }
}

/**
 * Sets the ClassifierWidget's properties to those selected in this dialog page.
 */
void ClassOptionsPage::applyWidget()
{
    m_pWidget->setVisualProperty( ClassifierWidget::ShowPackage, m_showPackageCB->isChecked() );
    m_pWidget->setVisualProperty( ClassifierWidget::ShowVisibility, m_showVisibilityCB->isChecked() );
    m_pWidget->setVisualProperty( ClassifierWidget::ShowOperations, m_showOpsCB->isChecked() );
    m_pWidget->setVisualProperty( ClassifierWidget::ShowOperationSignature, m_showOpSigCB->isChecked() );
    WidgetBase::WidgetType type = m_pWidget->baseType();
    if (type == WidgetBase::wt_Class) {
        m_pWidget->setVisualProperty( ClassifierWidget::ShowStereotype, m_showStereotypeCB->isChecked() );
        m_pWidget->setVisualProperty( ClassifierWidget::ShowAttributes, m_showAttsCB->isChecked() );
        m_pWidget->setVisualProperty( ClassifierWidget::ShowAttributeSignature, m_showAttSigCB->isChecked() );
        m_pWidget->setVisualProperty( ClassifierWidget::ShowPublicOnly, m_showPublicOnlyCB->isChecked() );
    } else if (type == WidgetBase::wt_Interface) {
        if (m_drawAsCircleCB)
            m_pWidget->setVisualProperty( ClassifierWidget::DrawAsCircle, m_drawAsCircleCB->isChecked() );
    }
}

/**
 * Sets the OptionState to the values selected in this dialog page.
 */
void ClassOptionsPage::applyOptionState()
{
    m_options->classState.showVisibility = m_showVisibilityCB->isChecked();
    if (m_showAttsCB)
        m_options->classState.showAtts = m_showAttsCB->isChecked();
    m_options->classState.showOps = m_showOpsCB->isChecked();
    if (m_showStereotypeCB)
        m_options->classState.showStereoType = m_showStereotypeCB->isChecked();
    m_options->classState.showPackage = m_showPackageCB->isChecked();
    if (m_showAttribAssocsCB)
        m_options->classState.showAttribAssocs = m_showAttribAssocsCB->isChecked();
    if (m_showAttSigCB)
        m_options->classState.showAttSig = m_showAttSigCB->isChecked();
    m_options->classState.showOpSig = m_showOpSigCB->isChecked();
    m_options->classState.showPublicOnly = m_showPublicOnlyCB->isChecked();
    if (!m_isDiagram) {
        m_options->classState.defaultAttributeScope = (Uml::Visibility::Value) m_attribScopeCB->currentIndex();
        m_options->classState.defaultOperationScope = (Uml::Visibility::Value) m_operationScopeCB->currentIndex();
    }
}

/**
 * Initialize optional items
 */
void ClassOptionsPage::init()
{
    m_options = NULL;
    m_pWidget = NULL;
    m_showStereotypeCB = NULL;
    m_showAttsCB = NULL;
    m_showAttSigCB = NULL;
    m_showAttribAssocsCB = NULL;
    m_showPublicOnlyCB = NULL;
    m_drawAsCircleCB = NULL;
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 */
void ClassOptionsPage::insertAttribScope( const QString& type, int index )
{
    m_attribScopeCB->insertItem( index, type );
    m_attribScopeCB->completionObject()->addItem( type );
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 */
void ClassOptionsPage::insertOperationScope( const QString& type, int index )
{
    m_operationScopeCB->insertItem( index, type );
    m_operationScopeCB->completionObject()->addItem( type );
}
