/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "classoptionspage.h"

// qt/kde includes
#include <qlayout.h>
#include <klocale.h>
#include <kdebug.h>

// local includes
#include "../umlview.h"
#include "../classifierwidget.h"


ClassOptionsPage::ClassOptionsPage(QWidget* pParent, ClassifierWidget* pWidget)
        : QWidget( pParent ) {
    init();
    //Uml::Widget_Type type = pWidget->getBaseType();
    m_pWidget = pWidget;
    setupPage();
}

ClassOptionsPage::ClassOptionsPage(QWidget* pParent, Settings::OptionState *options) : QWidget( pParent )
{
    init();
    m_options = options;
    setupClassPageOption();
}

void ClassOptionsPage::init() {
    m_options = NULL;
    m_pWidget = NULL;
    m_pShowStereotypeCB = NULL;
    m_pShowAttsCB = NULL;
    m_pShowAttSigCB = NULL;
    m_pDrawAsCircleCB = NULL;
}

ClassOptionsPage::~ClassOptionsPage() {}

void ClassOptionsPage::setupPage() {
    int margin = fontMetrics().height();

    bool sig = false;
    Uml::Signature_Type sigtype;

    QVBoxLayout * topLayout = new QVBoxLayout(this);
    topLayout -> setSpacing(6);
    m_pVisibilityGB = new QGroupBox(i18n("Show"), this);
    topLayout -> addWidget(m_pVisibilityGB);
    QGridLayout * visibilityLayout = new QGridLayout(m_pVisibilityGB);
    visibilityLayout -> setSpacing(10);
    visibilityLayout -> setMargin(margin);
    visibilityLayout -> setRowStretch(3, 1);

    m_pShowOpsCB = new QCheckBox(i18n("Operatio&ns"), m_pVisibilityGB);
    m_pShowOpsCB -> setChecked(m_pWidget -> getShowOps());
    visibilityLayout -> addWidget(m_pShowOpsCB, 0, 0);

    m_pShowVisibilityCB = new QCheckBox(i18n("&Visibility"), m_pVisibilityGB);
    m_pShowVisibilityCB -> setChecked(m_pWidget -> getShowVisibility());
    visibilityLayout -> addWidget(m_pShowVisibilityCB, 0, 1);

    sigtype = m_pWidget -> getShowOpSigs();
    if(sigtype == Uml::st_NoSig || sigtype == Uml::st_NoSigNoVis)
        sig = false;
    else
        sig = true;
    m_pShowOpSigCB = new QCheckBox(i18n("O&peration signature"), m_pVisibilityGB);
    m_pShowOpSigCB -> setChecked(sig);
    visibilityLayout -> addWidget(m_pShowOpSigCB, 1, 0);

    m_pShowPackageCB = new QCheckBox(i18n("Pac&kage"), m_pVisibilityGB);
    m_pShowPackageCB -> setChecked(m_pWidget -> getShowPackage());
    visibilityLayout -> addWidget(m_pShowPackageCB, 1, 1);

    Uml::Widget_Type type = m_pWidget->getBaseType();

    if (type == Uml::wt_Class) {
        m_pShowAttsCB = new QCheckBox(i18n("Att&ributes"), m_pVisibilityGB);
        m_pShowAttsCB->setChecked(m_pWidget->getShowAtts());
        visibilityLayout->addWidget(m_pShowAttsCB, 2, 0);

        m_pShowStereotypeCB = new QCheckBox(i18n("Stereot&ype"), m_pVisibilityGB);
        m_pShowStereotypeCB->setChecked(m_pWidget->getShowStereotype());
        visibilityLayout->addWidget(m_pShowStereotypeCB, 2, 1);

        m_pShowAttSigCB = new QCheckBox(i18n("Attr&ibute signature"), m_pVisibilityGB);
        sigtype = m_pWidget->getShowAttSigs();
        if(sigtype == Uml::st_NoSig || sigtype == Uml::st_NoSigNoVis)
            sig = false;
        else
            sig = true;
        m_pShowAttSigCB->setChecked(sig);
        visibilityLayout->addWidget(m_pShowAttSigCB, 3, 0);

    } else if (type == Uml::wt_Interface) {
        m_pDrawAsCircleCB = new QCheckBox(i18n("Draw as circle"), m_pVisibilityGB);
        m_pDrawAsCircleCB->setChecked( m_pWidget->getDrawAsCircle() );
        visibilityLayout->addWidget(m_pDrawAsCircleCB, 2, 0);
    }
}

void ClassOptionsPage::setupClassPageOption() {

    int margin = fontMetrics().height();

    QVBoxLayout * topLayout = new QVBoxLayout(this);
    topLayout -> setSpacing(6);
    m_pVisibilityGB = new QGroupBox(i18n("Show"), this);
    topLayout -> addWidget(m_pVisibilityGB);
    QGridLayout * visibilityLayout = new QGridLayout(m_pVisibilityGB);
    visibilityLayout -> setSpacing(10);
    visibilityLayout -> setMargin(margin);

    m_pShowOpsCB = new QCheckBox(i18n("Operatio&ns"), m_pVisibilityGB);
    m_pShowOpsCB -> setChecked( m_options->classState.showOps );
    visibilityLayout -> addWidget(m_pShowOpsCB, 0, 0);

    m_pShowOpSigCB = new QCheckBox(i18n("O&peration signature"), m_pVisibilityGB);
    m_pShowOpSigCB -> setChecked(m_options->classState.showOpSig);
    visibilityLayout -> addWidget(m_pShowOpSigCB, 1, 0);
    visibilityLayout -> setRowStretch(3, 1);

    m_pShowAttsCB = new QCheckBox(i18n("Att&ributes"), m_pVisibilityGB);
    m_pShowAttsCB -> setChecked(m_options->classState.showAtts );
    visibilityLayout -> addWidget(m_pShowAttsCB, 2, 0);

    m_pShowAttSigCB = new QCheckBox(i18n("Attr&ibute signature"), m_pVisibilityGB);
    m_pShowAttSigCB -> setChecked(m_options->classState.showAttSig);
    visibilityLayout -> addWidget(m_pShowAttSigCB, 3, 0);

    m_pShowVisibilityCB = new QCheckBox(i18n("&Visibility"), m_pVisibilityGB);
    m_pShowVisibilityCB -> setChecked(m_options->classState.showVisibility);
    visibilityLayout -> addWidget(m_pShowVisibilityCB, 0, 1);

    m_pShowPackageCB = new QCheckBox(i18n("Pac&kage"), m_pVisibilityGB);
    m_pShowPackageCB -> setChecked(m_options->classState.showPackage);
    visibilityLayout -> addWidget(m_pShowPackageCB, 1, 1);

    m_pShowStereotypeCB = new QCheckBox(i18n("Stereot&ype"), m_pVisibilityGB);
    m_pShowStereotypeCB -> setChecked(m_options->classState.showStereoType);
    visibilityLayout -> addWidget(m_pShowStereotypeCB, 2, 1);

}

void ClassOptionsPage::updateUMLWidget() {
    if (m_pWidget) {
        updateWidget();
    } else if (m_options) {
        updateOptionState();
    }
}

void ClassOptionsPage::updateWidget() {
    m_pWidget->setShowPackage( m_pShowPackageCB->isChecked() );
    m_pWidget->setShowVisibility( m_pShowVisibilityCB->isChecked() );
    m_pWidget->setShowOps( m_pShowOpsCB->isChecked() );
    m_pWidget->setShowOpSigs( m_pShowOpSigCB->isChecked() );
    Uml::Widget_Type type = m_pWidget->getBaseType();
    if (type == Uml::wt_Class) {
        m_pWidget->setShowStereotype( m_pShowStereotypeCB->isChecked() );
        m_pWidget->setShowAtts( m_pShowAttsCB->isChecked() );
        m_pWidget->setShowAttSigs( m_pShowAttSigCB->isChecked() );
    } else if (type == Uml::wt_Interface) {
        if (m_pDrawAsCircleCB)
            m_pWidget->setDrawAsCircle( m_pDrawAsCircleCB->isChecked() );
    }
}

void ClassOptionsPage::updateOptionState() {
    m_options->classState.showVisibility = m_pShowVisibilityCB->isChecked();
    if (m_pShowAttsCB)
        m_options->classState.showAtts = m_pShowAttsCB->isChecked();
    m_options->classState.showOps = m_pShowOpsCB->isChecked();
    if (m_pShowStereotypeCB)
        m_options->classState.showStereoType = m_pShowStereotypeCB->isChecked();
    m_options->classState.showPackage = m_pShowPackageCB->isChecked();
    if (m_pShowAttSigCB)
        m_options->classState.showAttSig = m_pShowAttSigCB->isChecked();
    m_options->classState.showOpSig = m_pShowOpSigCB->isChecked();
}


//#include "classoptionspage.moc"
