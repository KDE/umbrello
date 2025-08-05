/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "activitydialog.h"

//local includes
#include "umlview.h"
#include "activitywidget.h"
#include "dialog_utils.h"
#include "documentationwidget.h"
#include "icon_utils.h"

//kde includes
#include <klineedit.h>
#include <KLocalizedString>

//qt includes
#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>

/**
 * Constructor.
 */
ActivityDialog::ActivityDialog(QWidget * parent, ActivityWidget * pWidget)
   : MultiPageDialogBase(parent),
     m_pActivityWidget(pWidget),
     m_bChangesMade(false)
{
    setCaption(i18n("Properties"));
    setupPages();

    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
}

/**
 * Entered when OK button pressed.
 */
void ActivityDialog::slotOk()
{
    applyPage(pageItemStyle);
    applyPage(pageItemFont);
    applyPage(pageItemGeneral);
    accept();
}

/**
 * Entered when Apply button pressed.
 */
void ActivityDialog::slotApply()
{
    applyPage(currentPage());
}

void ActivityDialog::slotShowActivityParameter()
{
    m_GenPageWidgets.preL->show();
    m_GenPageWidgets.preLE->show();
    m_GenPageWidgets.postL->show();
    m_GenPageWidgets.postLE->show();
    if(!m_pActivityWidget->postconditionText().isEmpty()) {
        m_GenPageWidgets.postLE->setText(m_pActivityWidget->postconditionText());
    }
    if (!m_pActivityWidget->preconditionText().isEmpty()) {
        m_GenPageWidgets.preLE->setText(m_pActivityWidget->preconditionText());
    }
}

void ActivityDialog::slotHideActivityParameter()
{
    m_GenPageWidgets.preL->hide();
    m_GenPageWidgets.preLE->hide();
    m_GenPageWidgets.postL->hide();
    m_GenPageWidgets.postLE->hide();
}

/**
 * Sets up the pages of the dialog.
 */
void ActivityDialog::setupPages()
{
    setupGeneralPage();
    pageItemStyle = setupStylePage(m_pActivityWidget);
    pageItemFont = setupFontPage(m_pActivityWidget);
}

/**
 * Applies changes to the given page.
 */
void ActivityDialog::applyPage(KPageWidgetItem *item)
{
    m_bChangesMade = true;
    if (item == pageItemGeneral)
    {
        m_pActivityWidget->setName(m_GenPageWidgets.nameLE->text());
        m_GenPageWidgets.docWidget->apply();
        m_pActivityWidget->setPreconditionText(m_GenPageWidgets.preLE->text());
        m_pActivityWidget->setPostconditionText(m_GenPageWidgets.postLE->text());

        ActivityWidget::ActivityType newType = ActivityWidget::Initial;
        bool setType = false;
        if (m_GenPageWidgets.NormalRB->isChecked()) {
            newType = ActivityWidget::Normal;
            setType = true;
        } else if (m_GenPageWidgets.InvokRB->isChecked()) {
            newType = ActivityWidget::Invok;
            setType = true;
        } else if (m_GenPageWidgets.ParamRB->isChecked()) {
            newType = ActivityWidget::Param;
            setType = true;
        }
        if (setType)
            m_pActivityWidget->setActivityType (newType);

    }
    else if (item == pageItemFont)
    {
        applyFontPage(m_pActivityWidget);
    }
    else if (item == pageItemStyle)
    {
        applyStylePage();
    }
}

/**
 * Sets up the general page of the dialog.
 */
void ActivityDialog::setupGeneralPage()
{
    QString types[ ] = { i18n("Initial activity"), i18n("Activity"), i18n("End activity"), i18n("Final activity"), i18n("Branch/Merge"), i18n("Invoke action"), i18n("Parameter activity") };
    ActivityWidget::ActivityType type = m_pActivityWidget->activityType();

    QWidget *page = new QWidget();
    QVBoxLayout* topLayout = new QVBoxLayout();
    page->setLayout(topLayout);

    pageItemGeneral = createPage(i18nc("general properties page", "General"), i18n("General Properties"),
                                 Icon_Utils::it_Properties_General, page);

    m_GenPageWidgets.generalGB = new QGroupBox(i18n("Properties"));
    topLayout->addWidget(m_GenPageWidgets.generalGB);

    QGridLayout * generalLayout = new QGridLayout(m_GenPageWidgets.generalGB);
    generalLayout->setSpacing(Dialog_Utils::spacingHint());
    generalLayout->setContentsMargins(fontMetrics().height(), fontMetrics().height(), fontMetrics().height(), fontMetrics().height());

    QString actType (types[ (int)type ]);
    Dialog_Utils::makeLabeledEditField(generalLayout, 0,
                                    m_GenPageWidgets.typeL, i18n("Activity type:"),
                                    m_GenPageWidgets.typeLE, actType);
    m_GenPageWidgets.typeLE->setEnabled(false);

    Dialog_Utils::makeLabeledEditField(generalLayout, 1,
                                    m_GenPageWidgets.nameL, i18n("Activity name:"),
                                    m_GenPageWidgets.nameLE);

    Dialog_Utils::makeLabeledEditField(generalLayout, 5,
                                    m_GenPageWidgets.preL, i18n("Precondition :"),
                                    m_GenPageWidgets.preLE);

    Dialog_Utils::makeLabeledEditField(generalLayout, 6,
                                    m_GenPageWidgets.postL, i18n("Postcondition :"),
                                    m_GenPageWidgets.postLE);
    m_GenPageWidgets.preL->hide();
    m_GenPageWidgets.preLE->hide();
    m_GenPageWidgets.postL->hide();
    m_GenPageWidgets.postLE->hide();

    m_GenPageWidgets.NormalRB = new QRadioButton(i18n("&Normal activity"));
    generalLayout->addWidget(m_GenPageWidgets.NormalRB, 3, 0);

    m_GenPageWidgets.InvokRB = new QRadioButton(i18n("&Invoke action "));
    generalLayout->addWidget(m_GenPageWidgets.InvokRB, 3, 1);

    m_GenPageWidgets.ParamRB = new QRadioButton(i18n("&Parameter activity node"));
    generalLayout->addWidget(m_GenPageWidgets.ParamRB, 4, 0);

    if (type == ActivityWidget::Param)
    {
        showParameterActivity();
    }

    connect(m_GenPageWidgets.ParamRB, SIGNAL(clicked()), this, SLOT(slotShowActivityParameter()));
    connect(m_GenPageWidgets.NormalRB, SIGNAL(clicked()), this, SLOT(slotHideActivityParameter()));
    connect(m_GenPageWidgets.InvokRB, SIGNAL(clicked()), this, SLOT(slotHideActivityParameter()));

    ActivityWidget::ActivityType newType = m_pActivityWidget->activityType() ;

    m_GenPageWidgets.NormalRB->setChecked(newType == ActivityWidget::Normal);

    m_GenPageWidgets.InvokRB->setChecked (newType == ActivityWidget::Invok);

    m_GenPageWidgets.ParamRB->setChecked (newType == ActivityWidget::Param);

    m_GenPageWidgets.docWidget = new DocumentationWidget(m_pActivityWidget, (QWidget *)page);
    generalLayout->addWidget(m_GenPageWidgets.docWidget, 7, 0, 1, 3);

    if (type != ActivityWidget::Normal && type != ActivityWidget::Invok && type != ActivityWidget::Param) {
        m_GenPageWidgets.nameLE->setEnabled(false);
        m_GenPageWidgets.nameLE->setText(QString());
    } else
        m_GenPageWidgets.nameLE->setText(m_pActivityWidget->name());
    m_GenPageWidgets.nameLE->setFocus();
}

/**
 * Show the Activity Parameter entry text.
 */
void ActivityDialog::showParameterActivity()
{
    m_GenPageWidgets.preL->show();
    m_GenPageWidgets.preLE->show();
    m_GenPageWidgets.postL->show();
    m_GenPageWidgets.postLE->show();
    if (!m_pActivityWidget->postconditionText().isEmpty()) {
        m_GenPageWidgets.postLE->setText(m_pActivityWidget->postconditionText());
    }
    if (!m_pActivityWidget->preconditionText().isEmpty()) {
        m_GenPageWidgets.preLE->setText(m_pActivityWidget->preconditionText());
    }
}

