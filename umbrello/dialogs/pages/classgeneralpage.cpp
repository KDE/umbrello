/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// my own header
#include "classgeneralpage.h"

// app includes
#include "debug_utils.h"
#include "documentationwidget.h"
#include "dialog_utils.h"
#include "umlclassifier.h"
#include "umldatatype.h"
#include "instance.h"
#include "umlobject.h"
#include "objectwidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlartifact.h"
#include "umlcomponent.h"
#include "umlview.h"
#include "stereotype.h"
#include "umlpackagelist.h"
#include "model_utils.h"
#include "package.h"
#include "umlfolder.h"
#include "import_utils.h"
#include "umlscene.h"
#include "umlobjectnamewidget.h"
#include "umlpackagewidget.h"
#include "umlstereotypewidget.h"
#include "umlartifacttypewidget.h"
#include "visibilityenumwidget.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>
#include <kcombobox.h>
#include <klineedit.h>

// qt includes
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QLineEdit>

ClassGeneralPage::ClassGeneralPage(UMLDoc* d, QWidget* parent, UMLObject* o)
  : DialogPageBase(parent),
    m_pObject(o),
    m_pWidget(nullptr),
    m_pInstanceWidget(nullptr),
    m_pUmldoc(d),
    m_pMultiCB(nullptr),
    m_pDrawActorCB(nullptr),
    m_pAbstractCB(nullptr),
    m_pDeconCB(nullptr),
    m_pExecutableCB(nullptr),
    m_docWidget(nullptr),
    m_nameWidget(nullptr),
    m_instanceNameWidget(nullptr),
    m_stereotypeWidget(nullptr),
    m_packageWidget(nullptr),
    m_artifactTypeWidget(nullptr),
    m_visibilityEnumWidget(nullptr)
{
    if (!m_pObject) {
        logWarn0("ClassGeneralPage: Given UMLObject is NULL.");
        return;
    }
    for (int i = 0; i < N_STEREOATTRS; i++) {
        m_pTagL [i] = nullptr;
        m_pTagLE[i] = nullptr;
    }

    setMinimumSize(310, 330);
    QVBoxLayout * topLayout = new QVBoxLayout(this);
    topLayout->setSpacing(6);

    // setup name
    UMLObject::ObjectType t = m_pObject->baseType();
    m_pNameLayout = new QGridLayout();
    m_pNameLayout->setSpacing(6);
    topLayout->addLayout(m_pNameLayout, 4);

    if (t == UMLObject::ot_Instance) {
        const UMLInstance *inst = m_pObject->asUMLInstance();
        Q_ASSERT(inst);
        QString name = UMLObject::toI18nString(t);
        m_instanceNameWidget = new UMLObjectNameWidget(name, m_pObject->name());
        m_instanceNameWidget->addToLayout(m_pNameLayout, 0);
        setFocusProxy(m_instanceNameWidget);
        QString classNameLabel = UMLObject::toI18nString(UMLObject::ot_Class);
        QString className;
        if (inst->classifier())
            className = inst->classifier()->name();
        m_nameWidget = new UMLObjectNameWidget(classNameLabel, className);
        m_nameWidget->addToLayout(m_pNameLayout, 1);
    }
    else {
        QString name = UMLObject::toI18nString(t);
        m_nameWidget = new UMLObjectNameWidget(name, m_pObject->name());
        m_nameWidget->addToLayout(m_pNameLayout, 0);
        setFocusProxy(m_nameWidget);
    }

    if (t != UMLObject::ot_Stereotype && t != UMLObject::ot_Instance) {
        m_stereotypeWidget = new UMLStereotypeWidget(m_pObject);
        if (t == UMLObject::ot_Interface || t == UMLObject::ot_Datatype || t == UMLObject::ot_Enum) {
            m_stereotypeWidget->setEditable(false);
        }
        m_stereotypeWidget->addToLayout(m_pNameLayout, 1);
        connect(m_stereotypeWidget->editField(), SIGNAL(currentTextChanged(const QString&)),
                                          this, SLOT(slotStereoTextChanged(const QString&)));
        Dialog_Utils::makeTagEditFields(m_pObject, m_pNameLayout, m_pTagL, m_pTagLE);
    }

    int row = 2;
    if (m_pObject->isUMLDatatype()) {
        const UMLDatatype *d = m_pObject->asUMLDatatype();
        if (d && d->isReference() && d->originType()) {
            QLabel *label = new QLabel(i18n("Reference:"), this);
            m_pNameLayout->addWidget(label, row, 0);
            QLabel *reference = new QLabel(d->originType()->name(), this);
            m_pNameLayout->addWidget(reference, row, 1);
            ++row;
        }
    }

    if (t == UMLObject::ot_Class || t == UMLObject::ot_Interface || t == UMLObject::ot_Enum || t == UMLObject::ot_Entity) {
        m_packageWidget = new UMLPackageWidget(m_pObject);
        m_packageWidget->addToLayout(m_pNameLayout, row);
        ++row;
    }

    if (t == UMLObject::ot_Class || t == UMLObject::ot_UseCase) {
        QString abstractCaption;
        if (t == UMLObject::ot_Class) {
            abstractCaption = i18n("A&bstract class");
        } else {
            abstractCaption = i18n("A&bstract use case");
        }
        m_pAbstractCB = new QCheckBox(abstractCaption, this);
        m_pAbstractCB->setChecked(m_pObject->isAbstract());
        m_pNameLayout->addWidget(m_pAbstractCB, row, 0);
        ++row;
    }

    if (t == UMLObject::ot_Component) {
        m_pExecutableCB = new QCheckBox(i18nc("component is executable", "&Executable"), this);
        m_pExecutableCB->setChecked((o->asUMLComponent())->getExecutable());
        m_pNameLayout->addWidget(m_pExecutableCB, row, 0);
        ++row;
    }

    if (t == UMLObject::ot_Artifact) {
        m_artifactTypeWidget = new UMLArtifactTypeWidget(o->asUMLArtifact());
        m_artifactTypeWidget->addToLayout(topLayout);
    }

    // setup scope
    if (t != UMLObject::ot_Stereotype && t!= UMLObject::ot_Instance) {
        m_visibilityEnumWidget = new VisibilityEnumWidget(m_pObject, this);
        m_visibilityEnumWidget->addToLayout(topLayout);
    }

    m_docWidget = new DocumentationWidget(m_pObject, this);
    topLayout->addWidget(m_docWidget);
}

ClassGeneralPage::ClassGeneralPage(UMLDoc* d, QWidget* parent, ObjectWidget* o)
  : DialogPageBase(parent),
    m_pObject(nullptr),
    m_pWidget(o),
    m_pInstanceWidget(nullptr),
    m_pUmldoc(d),
    m_pMultiCB(nullptr),
    m_pDrawActorCB(nullptr),
    m_pAbstractCB(nullptr),
    m_pDeconCB(nullptr),
    m_pExecutableCB(nullptr),
    m_docWidget(nullptr),
    m_nameWidget(nullptr),
    m_instanceNameWidget(nullptr),
    m_stereotypeWidget(nullptr),
    m_packageWidget(nullptr),
    m_artifactTypeWidget(nullptr),
    m_visibilityEnumWidget(nullptr)
{
    if (!m_pWidget) {
        logWarn0("ClassGeneralPage: Given ObjectWidget is NULL.");
        return;
    }
    for (int i = 0; i < N_STEREOATTRS; i++) {
        m_pTagL [i] = nullptr;
        m_pTagLE[i] = nullptr;
    }

    setMinimumSize(310, 330);
    QVBoxLayout * topLayout = new QVBoxLayout(this);
    topLayout->setSpacing(6);

    // setup name
    m_pNameLayout = new QGridLayout();
    m_pNameLayout->setSpacing(6);
    topLayout->addLayout(m_pNameLayout, 4);

    QString name = UMLObject::toI18nString(UMLObject::ot_Instance);
    m_instanceNameWidget = new UMLObjectNameWidget(name , m_pWidget->instanceName());
    m_instanceNameWidget->addToLayout(m_pNameLayout, 0);
    setFocusProxy(m_instanceNameWidget);

    QString className = UMLObject::toI18nString(UMLObject::ot_Class);
    m_nameWidget = new UMLObjectNameWidget(className, m_pWidget->name());
    m_nameWidget->addToLayout(m_pNameLayout, 1);

    UMLView *view = UMLApp::app()->currentView();

    m_pDrawActorCB = new QCheckBox(i18n("Draw as actor"), this);
    m_pDrawActorCB->setChecked(m_pWidget->drawAsActor());
    m_pNameLayout->addWidget(m_pDrawActorCB, 2, 0);

    if (view->umlScene()->isCollaborationDiagram()) {
        m_pMultiCB = new QCheckBox(i18n("Multiple instance"), this);
        m_pMultiCB->setChecked(m_pWidget->multipleInstance());
        m_pNameLayout->addWidget(m_pMultiCB, 2, 1);
        if (m_pDrawActorCB->isChecked())
            m_pMultiCB->setEnabled(false);
    } else {  // sequence diagram
        m_pDeconCB = new QCheckBox(i18n("Show destruction"), this);
        m_pDeconCB->setChecked(m_pWidget->showDestruction());
        m_pNameLayout->addWidget(m_pDeconCB, 2, 1);
    }
    m_docWidget = new DocumentationWidget(m_pWidget, this);
    topLayout->addWidget(m_docWidget);

    if (m_pMultiCB) {
        connect(m_pDrawActorCB, SIGNAL(toggled(bool)), this, SLOT(slotActorToggled(bool)));
    }
}

ClassGeneralPage::ClassGeneralPage(UMLDoc* d, QWidget* parent, UMLWidget* widget)
  : DialogPageBase(parent),
    m_pObject(nullptr),
    m_pWidget(nullptr),
    m_pInstanceWidget(widget),
    m_pUmldoc(d),
    m_pMultiCB(nullptr),
    m_pDrawActorCB(nullptr),
    m_pAbstractCB(nullptr),
    m_pDeconCB(nullptr),
    m_pExecutableCB(nullptr),
    m_docWidget(nullptr),
    m_nameWidget(nullptr),
    m_instanceNameWidget(nullptr),
    m_stereotypeWidget(nullptr),
    m_packageWidget(nullptr),
    m_artifactTypeWidget(nullptr),
    m_visibilityEnumWidget(nullptr)
{
    for (int i = 0; i < N_STEREOATTRS; i++) {
        m_pTagL [i] = nullptr;
        m_pTagLE[i] = nullptr;
    }

    setMinimumSize(310, 330);
    QVBoxLayout * topLayout = new QVBoxLayout(this);
    topLayout->setSpacing(6);

    // setup name
    m_pNameLayout = new QGridLayout();
    m_pNameLayout->setSpacing(6);
    topLayout->addLayout(m_pNameLayout, 4);

    QString typeName = UMLWidget::toI18nString(widget->baseType());
    m_nameWidget = new UMLObjectNameWidget(typeName, widget->name());
    m_nameWidget->addToLayout(m_pNameLayout, 0);
    setFocusProxy(m_nameWidget);

    if (widget->umlObject()) {
        m_stereotypeWidget = new UMLStereotypeWidget(widget->umlObject());
        m_stereotypeWidget->addToLayout(m_pNameLayout, 1);
    }

    m_instanceNameWidget = new UMLObjectNameWidget(
                UMLObject::toI18nString(UMLObject::ot_Instance), widget->instanceName());
    m_instanceNameWidget->addToLayout(m_pNameLayout, 2);

    m_docWidget = new DocumentationWidget(widget, this);
    topLayout->addWidget(m_docWidget);
}

ClassGeneralPage::~ClassGeneralPage()
{
}

void ClassGeneralPage::slotStereoTextChanged(const QString &stereoText)
{
    Dialog_Utils::remakeTagEditFields(stereoText, m_pObject, m_pNameLayout, m_pTagL, m_pTagLE);
}

/**
 * Will move information from the dialog into the object.
 * Call when the ok or apply button is pressed.
 */
void ClassGeneralPage::apply()
{
    QString name = m_nameWidget->text();
    m_docWidget->apply();

    if (m_stereotypeWidget) {
        m_stereotypeWidget->apply();
        if (m_pObject) {
            Dialog_Utils::updateTagsFromEditFields(m_pObject, m_pTagLE);
        }
    }

    if (m_pObject) {
        UMLObject::ObjectType t = m_pObject->baseType();
        if (t == UMLObject::ot_Class || t == UMLObject::ot_Interface || t == UMLObject::ot_Enum || t == UMLObject::ot_Entity) {
            m_packageWidget->apply();
        }

        if (m_pAbstractCB) {
            m_pObject->setAbstract(m_pAbstractCB->isChecked());
        }

        if(m_instanceNameWidget && m_pObject->isUMLInstance()) {
            m_pObject->asUMLInstance()->setName(m_instanceNameWidget->text());
        }

        //make sure unique name
        if(m_pObject->baseType() != UMLObject::ot_Instance) {
            UMLObject *o = m_pUmldoc->findUMLObject(name);
            if (o && m_pObject != o) {
                 KMessageBox::information(this, i18n("The name you have chosen\nis already being used.\nThe name has been reset."),
                                    i18n("Name is Not Unique"), QString(), KMessageBox::Options(0));
                 m_nameWidget->reset();
            } else {
                 m_pObject->setName(name);
            }
        }

        if (t != UMLObject::ot_Stereotype) {
            if (m_visibilityEnumWidget)
                m_visibilityEnumWidget->apply();
        }

        if (m_pObject->baseType() == UMLObject::ot_Component) {
            (m_pObject->asUMLComponent())->setExecutable(m_pExecutableCB->isChecked());
        }

        if (m_pObject->baseType() == UMLObject::ot_Artifact) {
            m_artifactTypeWidget->apply();
            m_pObject->emitModified();
        }
    } // end if m_pObject
    else if (m_pWidget) {
        m_pWidget->setInstanceName(m_instanceNameWidget->text());
        if (m_pMultiCB) {
            m_pWidget->setMultipleInstance(m_pMultiCB->isChecked());
        }
        m_pWidget->setDrawAsActor(m_pDrawActorCB->isChecked());
        if (m_pDeconCB) {
            m_pWidget->setShowDestruction(m_pDeconCB->isChecked());
        }
        UMLObject * o = m_pWidget->umlObject();
        if (!o) {
            logError0("ClassGeneralPage::apply: UML object of widget is null");
            return;
        }
        UMLObject * old = m_pUmldoc->findUMLObject(name);
        if (old && o != old) {
            KMessageBox::information(this, i18n("The name you have chosen\nis already being used.\nThe name has been reset."),
                               i18n("Name is Not Unique"), QString(), KMessageBox::Options(0));
            m_nameWidget->reset();
        } else {
            o->setName(name);
        }
    } // end if m_pWidget
    else if (m_pInstanceWidget) {
        m_pInstanceWidget->setInstanceName(m_instanceNameWidget->text());
        UMLObject* o = m_pInstanceWidget->umlObject();
        if (!o) {
            logError0("ClassGeneralPage::apply: UML object of instance widget is null");
            setInstanceWidgetNameIfApplicable(name);
            return;
        }

        UMLObject* old = m_pUmldoc->findUMLObject(name);
        if (old && o != old) {
            KMessageBox::information(this, i18n("The name you have chosen\nis already being used.\nThe name has been reset."),
                               i18n("Name is Not Unique"), QString(), KMessageBox::Options(0));
            m_nameWidget->reset();
        } else {
            o->setName(name);
        }
    } // end if m_pInstanceWidget
}

/**
 * When the draw as actor check box is toggled, the draw
 * as multi instance need to be enabled/disabled. They
 * both can't be available at the same time.
 */
void ClassGeneralPage::slotActorToggled(bool state)
{
    if (m_pMultiCB) {
        m_pMultiCB->setEnabled(!state);
    }
}

/**
 * Sets the input name to the instance widget if the change is applicable.
 * @param name The name of the widget
 */
void ClassGeneralPage::setInstanceWidgetNameIfApplicable(const QString& name) const
{
    if(!m_pInstanceWidget)
        return;

    if(m_pInstanceWidget->isCombinedFragmentWidget() || m_pInstanceWidget->isFloatingDashLineWidget())
    {
        m_pInstanceWidget->setName(name);
    }
}
