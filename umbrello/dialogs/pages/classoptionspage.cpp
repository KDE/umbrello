/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "classoptionspage.h"

// local includes
#include "umlscene.h"
#include "umlview.h"
#include "classifierwidget.h"
#include "entitywidget.h"
#include "widgetbase.h"

// kde includes
#include <KLocalizedString>
#include <KComboBox>

// qt includes
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

/**
 * Constructor - observe and modify a Widget
 */
ClassOptionsPage::ClassOptionsPage(QWidget* pParent, ClassifierWidget* pWidget)
  : QWidget(pParent),
    m_isDiagram(false)
{
    init();
    //WidgetType type = pWidget->baseType();
    m_pWidget = pWidget;
    setupPage();
}

ClassOptionsPage::ClassOptionsPage(QWidget *pParent, UMLScene *scene)
  : QWidget(pParent),
    m_isDiagram(true)
{
    init();

    m_scene = scene;

    // class diagram uses full UIState
    if (scene->isClassDiagram()) {
        m_options = &scene->optionState();
        setupClassPageOption();
    }
    else {
        setupPageFromScene();
    }
}

/**
 * Constructor - observe and modify an OptionState structure
 *
 * @param pParent Parent widget
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

ClassOptionsPage::ClassOptionsPage(QWidget *pParent, EntityWidget *widget)
  : QWidget(pParent),
    m_isDiagram(false)
{
    init();
    m_entityWidget = widget;
    setupPageFromEntityWidget();
}

/**
 * Destructor
 */
ClassOptionsPage::~ClassOptionsPage()
{
}

void ClassOptionsPage::setDefaults()
{
    m_showVisibilityCB->setChecked(false);
    m_showAttsCB->setChecked(true);
#ifdef ENABLE_WIDGET_SHOW_DOC
    m_showDocumentationCB->setChecked(false);
#endif
    m_showOpsCB->setChecked(true);
    m_showStereotypeCB->setCurrentIndex(2); // Tags
    m_showAttSigCB->setChecked(false);
    m_showOpSigCB->setChecked(false);
    m_showPackageCB->setChecked(false);
    m_showPublicOnlyCB->setChecked(true);
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
    } else if (m_scene) {
        applyScene();
    } else if (m_options) {
        applyOptionState();
    } else if (m_entityWidget) {
        applyEntityWidget();
    }
}

/**
 * Set related uml widget
 */
void ClassOptionsPage::setWidget(ClassifierWidget * pWidget)
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
    Uml::SignatureType::Enum sigtype;

    QVBoxLayout * topLayout = new QVBoxLayout(this);

    topLayout->setSpacing(6);
    m_visibilityGB = new QGroupBox(i18n("Show"), this);
    topLayout->addWidget(m_visibilityGB);
    QGridLayout * visibilityLayout = new QGridLayout(m_visibilityGB);
    visibilityLayout->setSpacing(10);
    visibilityLayout->setMargin(margin);
    visibilityLayout->setRowStretch(3, 1);

#ifdef ENABLE_WIDGET_SHOW_DOC
    m_showDocumentationCB = new QCheckBox(i18n("&Documentation"), m_visibilityGB);
    m_showDocumentationCB->setChecked(m_pWidget->visualProperty(ClassifierWidget::ShowDocumentation));
    visibilityLayout->addWidget(m_showDocumentationCB, 0, 0);
#endif

    m_showOpsCB = new QCheckBox(i18n("Operatio&ns"), m_visibilityGB);
    m_showOpsCB->setChecked(m_pWidget->visualProperty(ClassifierWidget::ShowOperations));
    visibilityLayout->addWidget(m_showOpsCB, 1, 0);

    m_showVisibilityCB = new QCheckBox(i18n("&Visibility"), m_visibilityGB);
    m_showVisibilityCB->setChecked(m_pWidget->visualProperty(ClassifierWidget::ShowVisibility));
    visibilityLayout->addWidget(m_showVisibilityCB, 1, 1);

    sigtype = m_pWidget->operationSignature();
    if (sigtype == Uml::SignatureType::NoSig || sigtype == Uml::SignatureType::NoSigNoVis)
        sig = false;
    else
        sig = true;
    m_showOpSigCB = new QCheckBox(i18n("O&peration signature"), m_visibilityGB);
    m_showOpSigCB->setChecked(sig);
    visibilityLayout->addWidget(m_showOpSigCB, 2, 0);

    m_showPackageCB = new QCheckBox(i18n("Pac&kage"), m_visibilityGB);
    m_showPackageCB->setChecked(m_pWidget->visualProperty(ClassifierWidget::ShowPackage));
    visibilityLayout->addWidget(m_showPackageCB, 2, 1);

    WidgetBase::WidgetType type = m_pWidget->baseType();

    if (type == WidgetBase::wt_Class) {
        m_showAttsCB = new QCheckBox(i18n("Att&ributes"), m_visibilityGB);
        m_showAttsCB->setChecked(m_pWidget->visualProperty(ClassifierWidget::ShowAttributes));
        visibilityLayout->addWidget(m_showAttsCB, 3, 0);

        m_showStereotypeCB = createShowStereotypeCB(m_visibilityGB);
        m_showStereotypeCB->setCurrentIndex(m_pWidget->showStereotype());
        visibilityLayout->addWidget(m_showStereotypeCB, 3, 1);

        m_showAttSigCB = new QCheckBox(i18n("Attr&ibute signature"), m_visibilityGB);
        sigtype = m_pWidget->attributeSignature();
        if (sigtype == Uml::SignatureType::NoSig || sigtype == Uml::SignatureType::NoSigNoVis)
            sig = false;
        else
            sig = true;
        m_showAttSigCB->setChecked(sig);
        visibilityLayout->addWidget(m_showAttSigCB, 4, 0);

        m_showPublicOnlyCB = new QCheckBox(i18n("&Public Only"), m_visibilityGB);
        m_showPublicOnlyCB->setChecked(m_pWidget->visualProperty(ClassifierWidget::ShowPublicOnly));
        visibilityLayout->addWidget(m_showPublicOnlyCB, 4, 1);


    } else if (type == WidgetBase::wt_Interface) {
        m_drawAsCircleCB = new QCheckBox(i18n("Draw as circle"), m_visibilityGB);
        m_drawAsCircleCB->setChecked(m_pWidget->visualProperty(ClassifierWidget::DrawAsCircle));
        visibilityLayout->addWidget(m_drawAsCircleCB, 3, 0);
    }
}

void ClassOptionsPage::setupPageFromScene()
{
    int margin = fontMetrics().height();

    QVBoxLayout * topLayout = new QVBoxLayout(this);

    topLayout->setSpacing(6);
    m_visibilityGB = new QGroupBox(i18n("Show"), this);
    topLayout->addWidget(m_visibilityGB);
    QGridLayout * visibilityLayout = new QGridLayout(m_visibilityGB);
    visibilityLayout->setSpacing(10);
    visibilityLayout->setMargin(margin);
    visibilityLayout->setRowStretch(3, 1);

    m_showOpSigCB = new QCheckBox(i18n("O&peration signature"), m_visibilityGB);
    m_showOpSigCB->setChecked(m_scene->showOpSig());
    visibilityLayout->addWidget(m_showOpSigCB, 1, 0);
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

#ifdef ENABLE_WIDGET_SHOW_DOC
    m_showDocumentationCB = new QCheckBox(i18n("&Documentation"), m_visibilityGB);
    m_showDocumentationCB->setChecked(m_options->classState.showDocumentation);
    visibilityLayout->addWidget(m_showDocumentationCB, 0, 0);
#endif

    m_showOpsCB = new QCheckBox(i18n("Operatio&ns"), m_visibilityGB);
    m_showOpsCB->setChecked(m_options->classState.showOps);
    visibilityLayout->addWidget(m_showOpsCB, 1, 0);

    m_showOpSigCB = new QCheckBox(i18n("O&peration signature"), m_visibilityGB);
    m_showOpSigCB->setChecked(m_options->classState.showOpSig);
    visibilityLayout->addWidget(m_showOpSigCB, 2, 0);
    visibilityLayout->setRowStretch(3, 1);

    m_showAttsCB = new QCheckBox(i18n("Att&ributes"), m_visibilityGB);
    m_showAttsCB->setChecked(m_options->classState.showAtts);
    visibilityLayout->addWidget(m_showAttsCB, 3, 0);

    m_showAttSigCB = new QCheckBox(i18n("Attr&ibute signature"), m_visibilityGB);
    m_showAttSigCB->setChecked(m_options->classState.showAttSig);
    visibilityLayout->addWidget(m_showAttSigCB, 4, 0);

    m_showVisibilityCB = new QCheckBox(i18n("&Visibility"), m_visibilityGB);
    m_showVisibilityCB->setChecked(m_options->classState.showVisibility);
    visibilityLayout->addWidget(m_showVisibilityCB, 1, 1);

    m_showPackageCB = new QCheckBox(i18n("Pac&kage"), m_visibilityGB);
    m_showPackageCB->setChecked(m_options->classState.showPackage);
    visibilityLayout->addWidget(m_showPackageCB, 2, 1);

    m_showStereotypeCB = createShowStereotypeCB(m_visibilityGB);
    m_showStereotypeCB->setCurrentIndex(m_options->classState.showStereoType);
    visibilityLayout->addWidget(m_showStereotypeCB, 3, 1);

    m_showAttribAssocsCB = new QCheckBox(i18n("&Attribute associations"), m_visibilityGB);
    m_showAttribAssocsCB->setChecked(m_options->classState.showAttribAssocs);
    visibilityLayout->addWidget(m_showAttribAssocsCB, 4, 1);

    m_showPublicOnlyCB = new QCheckBox(i18n("&Public Only"), m_visibilityGB);
    m_showPublicOnlyCB->setChecked(m_options->classState.showPublicOnly);
    visibilityLayout->addWidget(m_showPublicOnlyCB, 5, 1);

    if (!m_isDiagram) {
        m_scopeGB = new QGroupBox(i18n("Starting Scope"));
        topLayout->addWidget(m_scopeGB);

        QGridLayout * scopeLayout = new QGridLayout(m_scopeGB);
        scopeLayout->setSpacing(10);
        scopeLayout->setMargin(fontMetrics().height());

        m_attributeLabel = new QLabel(i18n("Default attribute scope:"), m_scopeGB);
        scopeLayout->addWidget(m_attributeLabel, 0, 0);

        m_operationLabel = new QLabel(i18n("Default operation scope:"), m_scopeGB);
        scopeLayout->addWidget(m_operationLabel, 1, 0);

        m_attribScopeCB = new KComboBox(m_scopeGB);
        insertAttribScope(i18n("Public"));
        insertAttribScope(i18n("Private"));
        insertAttribScope(i18n("Protected"));
        m_attribScopeCB->setCurrentIndex(m_options->classState.defaultAttributeScope);
        scopeLayout->addWidget(m_attribScopeCB, 0, 1);

        m_operationScopeCB = new KComboBox(m_scopeGB);
        insertOperationScope(i18n("Public"));
        insertOperationScope(i18n("Private"));
        insertOperationScope(i18n("Protected"));
        m_operationScopeCB->setCurrentIndex(m_options->classState.defaultOperationScope);
        scopeLayout->addWidget(m_operationScopeCB, 1, 1);
        topLayout->addWidget(m_scopeGB);
    }
}

void ClassOptionsPage::setupPageFromEntityWidget()
{
    int margin = fontMetrics().height();

    QVBoxLayout * topLayout = new QVBoxLayout(this);

    topLayout->setSpacing(6);
    m_visibilityGB = new QGroupBox(i18n("Show"), this);
    topLayout->addWidget(m_visibilityGB);
    QGridLayout * visibilityLayout = new QGridLayout(m_visibilityGB);
    visibilityLayout->setSpacing(10);
    visibilityLayout->setMargin(margin);
    visibilityLayout->setRowStretch(3, 1);

    m_showAttSigCB = new QCheckBox(i18n("Attribute Signatures"), m_visibilityGB);
    m_showAttSigCB->setChecked(m_entityWidget->showAttributeSignature());
    visibilityLayout->addWidget(m_showAttSigCB, 1, 0);

    m_showStereotypeCB = createShowStereotypeCB(m_visibilityGB);
    m_showStereotypeCB->setCurrentIndex(m_entityWidget->showStereotype());
    visibilityLayout->addWidget(m_showStereotypeCB, 2, 0);
}

/**
 * Sets the ClassifierWidget's properties to those selected in this dialog page.
 */
void ClassOptionsPage::applyWidget()
{
#ifdef ENABLE_WIDGET_SHOW_DOC
    m_pWidget->setVisualProperty(ClassifierWidget::ShowDocumentation, m_showDocumentationCB->isChecked());
#endif
    m_pWidget->setVisualProperty(ClassifierWidget::ShowPackage, m_showPackageCB->isChecked());
    m_pWidget->setVisualProperty(ClassifierWidget::ShowVisibility, m_showVisibilityCB->isChecked());
    m_pWidget->setVisualProperty(ClassifierWidget::ShowOperations, m_showOpsCB->isChecked());
    m_pWidget->setVisualProperty(ClassifierWidget::ShowOperationSignature, m_showOpSigCB->isChecked());
    WidgetBase::WidgetType type = m_pWidget->baseType();
    if (type == WidgetBase::wt_Class) {
        m_pWidget->setShowStereotype(Uml::ShowStereoType::Enum(m_showStereotypeCB->currentIndex()));
        m_pWidget->setVisualProperty(ClassifierWidget::ShowAttributes, m_showAttsCB->isChecked());
        m_pWidget->setVisualProperty(ClassifierWidget::ShowAttributeSignature, m_showAttSigCB->isChecked());
        m_pWidget->setVisualProperty(ClassifierWidget::ShowPublicOnly, m_showPublicOnlyCB->isChecked());
    } else if (type == WidgetBase::wt_Interface) {
        if (m_drawAsCircleCB)
            m_pWidget->setVisualProperty(ClassifierWidget::DrawAsCircle, m_drawAsCircleCB->isChecked());
    }
}

/**
 * Sets the OptionState to the values selected in this dialog page.
 */
void ClassOptionsPage::applyOptionState()
{
#ifdef ENABLE_WIDGET_SHOW_DOC
    m_options->classState.showDocumentation = m_showDocumentationCB->isChecked();
#endif
    m_options->classState.showVisibility = m_showVisibilityCB->isChecked();
    if (m_showAttsCB)
        m_options->classState.showAtts = m_showAttsCB->isChecked();
    m_options->classState.showOps = m_showOpsCB->isChecked();
    if (m_showStereotypeCB)
        m_options->classState.showStereoType = Uml::ShowStereoType::Enum(m_showStereotypeCB->currentIndex());
    m_options->classState.showPackage = m_showPackageCB->isChecked();
    if (m_showAttribAssocsCB)
        m_options->classState.showAttribAssocs = m_showAttribAssocsCB->isChecked();
    if (m_showAttSigCB)
        m_options->classState.showAttSig = m_showAttSigCB->isChecked();
    m_options->classState.showOpSig = m_showOpSigCB->isChecked();
    m_options->classState.showPublicOnly = m_showPublicOnlyCB->isChecked();
    if (!m_isDiagram) {
        m_options->classState.defaultAttributeScope = Uml::Visibility::fromInt(m_attribScopeCB->currentIndex());
        m_options->classState.defaultOperationScope = Uml::Visibility::fromInt(m_operationScopeCB->currentIndex());
    }
}

/**
 * Sets the UMLScene's properties to those selected in this dialog page.
 */
void ClassOptionsPage::applyScene()
{
    if (m_scene->isClassDiagram()) {
        applyOptionState();
        m_scene->setClassWidgetOptions(this);
    }
    else
        m_scene->setShowOpSig(m_showOpSigCB->isChecked());
}

void ClassOptionsPage::applyEntityWidget()
{
    Q_ASSERT(m_entityWidget);
    m_entityWidget->setShowStereotype(Uml::ShowStereoType::Enum(m_showStereotypeCB->currentIndex()));
    m_entityWidget->setShowAttributeSignature(m_showAttSigCB->isChecked());
}

/**
 * Initialize optional items
 */
void ClassOptionsPage::init()
{
    m_scene = nullptr;
    m_options = nullptr;
    m_pWidget = nullptr;
    m_entityWidget = nullptr;
    m_showStereotypeCB = nullptr;
    m_showAttsCB = nullptr;
    m_showAttSigCB = nullptr;
    m_showAttribAssocsCB = nullptr;
    m_showDocumentationCB = nullptr;
    m_showPublicOnlyCB = nullptr;
    m_drawAsCircleCB = nullptr;
}

/**
 * This need not be a member method, it can be made "static" or be moved to Dialog_Utils
 */
KComboBox * ClassOptionsPage::createShowStereotypeCB(QGroupBox * grpBox)
{
    KComboBox * cobox = new KComboBox(grpBox);
    cobox->setEditable(false);
    cobox->addItem(i18n("No Stereotype"));
    cobox->addItem(i18n("Stereotype Name"));
    cobox->addItem(i18n("Stereotype with Tags"));
    return cobox;
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 */
void ClassOptionsPage::insertAttribScope(const QString& type, int index)
{
    m_attribScopeCB->insertItem(index, type);
    m_attribScopeCB->completionObject()->addItem(type);
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 */
void ClassOptionsPage::insertOperationScope(const QString& type, int index)
{
    m_operationScopeCB->insertItem(index, type);
    m_operationScopeCB->completionObject()->addItem(type);
}
