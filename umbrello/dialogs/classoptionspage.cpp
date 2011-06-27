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

// qt includes
#include <QtGui/QCheckBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>

ClassOptionsPage::ClassOptionsPage(QWidget* pParent, ClassifierWidget* pWidget)
  : QWidget(pParent)
{
    init();
    //WidgetType type = pWidget->baseType();
    m_pWidget = pWidget;
    setupPage();
}

ClassOptionsPage::ClassOptionsPage(QWidget* pParent, Settings::OptionState *options)
  : QWidget(pParent)
{
    init();
    m_options = options;
    setupClassPageOption();
}

/**
 * Initialize optional items
 */
void ClassOptionsPage::init()
{
    m_options = NULL;
    m_pWidget = NULL;
    m_pShowStereotypeCB = NULL;
    m_pShowAttsCB = NULL;
    m_pShowAttSigCB = NULL;
    m_pShowAttribAssocsCB = NULL;
    m_pShowPublicOnlyCB = NULL;
    m_pDrawAsCircleCB = NULL;
}

ClassOptionsPage::~ClassOptionsPage()
{
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
    m_pVisibilityGB = new QGroupBox(i18n("Show"), this);
    topLayout->addWidget(m_pVisibilityGB);
    QGridLayout * visibilityLayout = new QGridLayout(m_pVisibilityGB);
    visibilityLayout->setSpacing(10);
    visibilityLayout->setMargin(margin);
    visibilityLayout->setRowStretch(3, 1);

    m_pShowOpsCB = new QCheckBox(i18n("Operatio&ns"), m_pVisibilityGB);
    m_pShowOpsCB->setChecked(m_pWidget->getShowOps());
    visibilityLayout->addWidget(m_pShowOpsCB, 0, 0);

    m_pShowVisibilityCB = new QCheckBox(i18n("&Visibility"), m_pVisibilityGB);
    m_pShowVisibilityCB->setChecked(m_pWidget->getShowVisibility());
    visibilityLayout->addWidget(m_pShowVisibilityCB, 0, 1);

    sigtype = m_pWidget->operationSignatureType();
    if (sigtype == Uml::SignatureType::NoSig || sigtype == Uml::SignatureType::NoSigNoVis)
        sig = false;
    else
        sig = true;
    m_pShowOpSigCB = new QCheckBox(i18n("O&peration signature"), m_pVisibilityGB);
    m_pShowOpSigCB->setChecked(sig);
    visibilityLayout->addWidget(m_pShowOpSigCB, 1, 0);

    m_pShowPackageCB = new QCheckBox(i18n("Pac&kage"), m_pVisibilityGB);
    m_pShowPackageCB->setChecked(m_pWidget->getShowPackage());
    visibilityLayout->addWidget(m_pShowPackageCB, 1, 1);

    WidgetBase::WidgetType type = m_pWidget->baseType();

    if (type == WidgetBase::wt_Class) {
        m_pShowAttsCB = new QCheckBox(i18n("Att&ributes"), m_pVisibilityGB);
        m_pShowAttsCB->setChecked(m_pWidget->getShowAtts());
        visibilityLayout->addWidget(m_pShowAttsCB, 2, 0);

        m_pShowStereotypeCB = new QCheckBox(i18n("Stereot&ype"), m_pVisibilityGB);
        m_pShowStereotypeCB->setChecked(m_pWidget->getShowStereotype());
        visibilityLayout->addWidget(m_pShowStereotypeCB, 2, 1);

        m_pShowAttSigCB = new QCheckBox(i18n("Attr&ibute signature"), m_pVisibilityGB);
        sigtype = m_pWidget->attributeSignatureType();
        if (sigtype == Uml::SignatureType::NoSig || sigtype == Uml::SignatureType::NoSigNoVis)
            sig = false;
        else
            sig = true;
        m_pShowAttSigCB->setChecked(sig);
        visibilityLayout->addWidget(m_pShowAttSigCB, 3, 0);

        m_pShowPublicOnlyCB = new QCheckBox(i18n("&Public Only"), m_pVisibilityGB);
        m_pShowPublicOnlyCB->setChecked(m_pWidget->getShowPublicOnly());
        visibilityLayout->addWidget(m_pShowPublicOnlyCB, 3, 1);


    } else if (type == WidgetBase::wt_Interface) {
        m_pDrawAsCircleCB = new QCheckBox(i18n("Draw as circle"), m_pVisibilityGB);
        m_pDrawAsCircleCB->setChecked( m_pWidget->getDrawAsCircle() );
        visibilityLayout->addWidget(m_pDrawAsCircleCB, 2, 0);
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
    m_pVisibilityGB = new QGroupBox(i18n("Show"), this);
    topLayout->addWidget(m_pVisibilityGB);
    QGridLayout * visibilityLayout = new QGridLayout(m_pVisibilityGB);
    visibilityLayout->setSpacing(10);
    visibilityLayout->setMargin(margin);

    m_pShowOpsCB = new QCheckBox(i18n("Operatio&ns"), m_pVisibilityGB);
    m_pShowOpsCB->setChecked( m_options->classState.showOps );
    visibilityLayout->addWidget(m_pShowOpsCB, 0, 0);

    m_pShowOpSigCB = new QCheckBox(i18n("O&peration signature"), m_pVisibilityGB);
    m_pShowOpSigCB->setChecked(m_options->classState.showOpSig);
    visibilityLayout->addWidget(m_pShowOpSigCB, 1, 0);
    visibilityLayout->setRowStretch(3, 1);

    m_pShowAttsCB = new QCheckBox(i18n("Att&ributes"), m_pVisibilityGB);
    m_pShowAttsCB->setChecked(m_options->classState.showAtts );
    visibilityLayout->addWidget(m_pShowAttsCB, 2, 0);

    m_pShowAttSigCB = new QCheckBox(i18n("Attr&ibute signature"), m_pVisibilityGB);
    m_pShowAttSigCB->setChecked(m_options->classState.showAttSig);
    visibilityLayout->addWidget(m_pShowAttSigCB, 3, 0);

    m_pShowVisibilityCB = new QCheckBox(i18n("&Visibility"), m_pVisibilityGB);
    m_pShowVisibilityCB->setChecked(m_options->classState.showVisibility);
    visibilityLayout->addWidget(m_pShowVisibilityCB, 0, 1);

    m_pShowPackageCB = new QCheckBox(i18n("Pac&kage"), m_pVisibilityGB);
    m_pShowPackageCB->setChecked(m_options->classState.showPackage);
    visibilityLayout->addWidget(m_pShowPackageCB, 1, 1);

    m_pShowStereotypeCB = new QCheckBox(i18n("Stereot&ype"), m_pVisibilityGB);
    m_pShowStereotypeCB->setChecked(m_options->classState.showStereoType);
    visibilityLayout->addWidget(m_pShowStereotypeCB, 2, 1);

    m_pShowAttribAssocsCB = new QCheckBox(i18n("&Attribute associations"), m_pVisibilityGB);
    m_pShowAttribAssocsCB->setChecked(m_options->classState.showAttribAssocs);
    visibilityLayout->addWidget(m_pShowAttribAssocsCB, 3, 1);

    m_pShowPublicOnlyCB = new QCheckBox(i18n("&Public Only"), m_pVisibilityGB);
    m_pShowPublicOnlyCB->setChecked(m_options->classState.showPublicOnly);
    visibilityLayout->addWidget(m_pShowPublicOnlyCB, 4, 1);
}

/**
 * Updates the widget with the dialog page properties.
 */
void ClassOptionsPage::updateUMLWidget()
{
    if (m_pWidget) {
        updateWidget();
    } else if (m_options) {
        updateOptionState();
    }
}

/**
 * Sets the ClassifierWidget's properties to those selected in this dialog page.
 */
void ClassOptionsPage::updateWidget()
{
    m_pWidget->setShowPackage( m_pShowPackageCB->isChecked() );
    m_pWidget->setShowVisibility( m_pShowVisibilityCB->isChecked() );
    m_pWidget->setShowOps( m_pShowOpsCB->isChecked() );
    m_pWidget->setShowOpSigs( m_pShowOpSigCB->isChecked() );
    WidgetBase::WidgetType type = m_pWidget->baseType();
    if (type == WidgetBase::wt_Class) {
        m_pWidget->setShowStereotype( m_pShowStereotypeCB->isChecked() );
        m_pWidget->setShowAtts( m_pShowAttsCB->isChecked() );
        m_pWidget->setShowAttSigs( m_pShowAttSigCB->isChecked() );
        m_pWidget->setShowPublicOnly( m_pShowPublicOnlyCB->isChecked() );
    } else if (type == WidgetBase::wt_Interface) {
        if (m_pDrawAsCircleCB)
            m_pWidget->setDrawAsCircle( m_pDrawAsCircleCB->isChecked() );
    }
}

/**
 * Sets the OptionState to the values selected in this dialog page.
 */
void ClassOptionsPage::updateOptionState()
{
    m_options->classState.showVisibility = m_pShowVisibilityCB->isChecked();
    if (m_pShowAttsCB)
        m_options->classState.showAtts = m_pShowAttsCB->isChecked();
    m_options->classState.showOps = m_pShowOpsCB->isChecked();
    if (m_pShowStereotypeCB)
        m_options->classState.showStereoType = m_pShowStereotypeCB->isChecked();
    m_options->classState.showPackage = m_pShowPackageCB->isChecked();
    if (m_pShowAttribAssocsCB)
        m_options->classState.showAttribAssocs = m_pShowAttribAssocsCB->isChecked();
    if (m_pShowAttSigCB)
        m_options->classState.showAttSig = m_pShowAttSigCB->isChecked();
    m_options->classState.showOpSig = m_pShowOpSigCB->isChecked();
    m_options->classState.showPublicOnly = m_pShowPublicOnlyCB->isChecked();
}
