/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "statedialog.h"

// local includes
#include "activitypage.h"
#include "umlview.h"
#include "statewidget.h"
#include "dialog_utils.h"
#include "icon_utils.h"

// kde includes
#include <klineedit.h>
#include <klocale.h>
#include <ktextedit.h>

// qt includes
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>

/**
 * Constructor.
 */
StateDialog::StateDialog(QWidget * parent, StateWidget * pWidget)
  : DialogBase(parent),
    m_pActivityPage(0),
    m_pStateWidget(pWidget),
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
void StateDialog::slotOk()
{
    applyPage(pageGeneral);
    applyPage(pageFont);
    applyPage(pageActivity);
    applyPage(pageStyle);
    accept();
}

/**
 * Entered when Apply button pressed.
 */
void StateDialog::slotApply()
{
    applyPage(currentPage());
}

/**
 * Sets up the pages of the dialog.
 */
void StateDialog::setupPages()
{
    setupGeneralPage();
    if (m_pStateWidget->stateType() == StateWidget::Normal) {
        setupActivityPage();
    }
    pageStyle = setupStylePage(m_pStateWidget);
    pageFont = setupFontPage(m_pStateWidget);
}

/**
 * Applies changes to the given page.
 */
void StateDialog::applyPage(KPageWidgetItem*item)
{
    m_bChangesMade = true;
    if (item == pageGeneral) {
        m_pStateWidget->setName(m_GenPageWidgets.nameLE->text());
        m_pStateWidget->setDocumentation(m_GenPageWidgets.docMLE->toPlainText());
    }
    else if (item == pageActivity) {
        if (m_pActivityPage) {
            m_pActivityPage->updateActivities();
        }
    }
    else if (item == pageStyle) {
        saveStylePageData(m_pStateWidget);
    }
    else if (item == pageFont) {
        saveFontPageData(m_pStateWidget);
    }
}

/**
 * Sets up the general page of the dialog.
 */
void StateDialog::setupGeneralPage()
{
    StateWidget::StateType type = m_pStateWidget->stateType();

    QWidget* page = new QWidget();
    QVBoxLayout* topLayout = new QVBoxLayout();
    page->setLayout(topLayout);

    pageGeneral = new KPageWidgetItem(page, i18nc("general page", "General"));
    pageGeneral->setHeader(i18n("General Properties"));
    pageGeneral->setIcon(Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_General));
    addPage(pageGeneral);

    m_GenPageWidgets.generalGB = new QGroupBox(i18n("Properties"));
    topLayout->addWidget(m_GenPageWidgets.generalGB);

    QGridLayout * generalLayout = new QGridLayout(m_GenPageWidgets.generalGB);
    generalLayout->setSpacing(spacingHint());
    generalLayout->setMargin(fontMetrics().height());

    QString typeStr;
    switch (type) {
    case StateWidget::Initial:
        typeStr = i18nc("initial state in statechart", "Initial state");
        break;
    case StateWidget::Normal:
        typeStr = i18nc("state in statechart", "State");
        break;
    case StateWidget::End:
        typeStr = i18nc("end state in statechart", "End state");
        break;
    case StateWidget::Fork:
        typeStr = i18nc("fork state in statechart", "Fork");
        break;
    case StateWidget::Join:
        typeStr = i18nc("join state in statechart", "Join");
        break;
    case StateWidget::Junction:
        typeStr = i18nc("junction state in statechart", "Junction");
        break;
    case StateWidget::DeepHistory:
        typeStr = i18nc("deep history state in statechart", "DeepHistory");
        break;
    case StateWidget::ShallowHistory:
        typeStr = i18nc("shallow history state in statechart", "ShallowHistory");
        break;
    case StateWidget::Choice:
        typeStr = i18nc("choice state in statechart", "Choice");
        break;
    default:
        typeStr = QString::fromLatin1("???");
        break;
    }
    Dialog_Utils::makeLabeledEditField(generalLayout, 0,
                                       m_GenPageWidgets.typeL, i18n("State type:"),
                                       m_GenPageWidgets.typeLE, typeStr);
    m_GenPageWidgets.typeLE->setEnabled(false);

    Dialog_Utils::makeLabeledEditField(generalLayout, 1,
                                    m_GenPageWidgets.nameL, i18n("State name:"),
                                    m_GenPageWidgets.nameLE);

    m_GenPageWidgets.docGB = new QGroupBox(i18n("Documentation"));
    topLayout->addWidget(m_GenPageWidgets.docGB);

    QHBoxLayout * docLayout = new QHBoxLayout(m_GenPageWidgets.docGB);
    docLayout->setSpacing(spacingHint());
    docLayout->setMargin(fontMetrics().height());

    m_GenPageWidgets.docMLE = new KTextEdit(m_GenPageWidgets.docGB);
    m_GenPageWidgets.docMLE->setText(m_pStateWidget->documentation());
    docLayout->addWidget(m_GenPageWidgets.docMLE);

    if (type != StateWidget::Normal) {
        m_GenPageWidgets.nameLE->setEnabled(false);
        m_GenPageWidgets.nameLE->setText(QString());
    } else
        m_GenPageWidgets.nameLE->setText(m_pStateWidget->name());
}

/**
 * Sets up the activity page.
 */
void StateDialog::setupActivityPage()
{
    QFrame * activityPage = new QFrame();
    pageActivity = new KPageWidgetItem(activityPage, i18n("Activities"));
    pageActivity->setHeader(i18n("Activities"));
    pageActivity->setIcon(Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Activities));
    addPage(pageActivity);

    QHBoxLayout * activityLayout = new QHBoxLayout(activityPage);
    m_pActivityPage = new ActivityPage(activityPage, m_pStateWidget);
    activityLayout->addWidget(m_pActivityPage);
}
