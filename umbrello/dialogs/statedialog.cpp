/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "statedialog.h"

// local includes
#include "activitypage.h"
#include "documentationwidget.h"
#include "umlview.h"
#include "umlscene.h"
#include "umlviewlist.h"
#include "umldoc.h"
#include "uml.h"
#include "selectdiagramwidget.h"
#include "statewidget.h"
#include "dialog_utils.h"
#include "icon_utils.h"

// kde includes
#include <KComboBox>
#include <klineedit.h>
#include <KLocalizedString>

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
  : MultiPageDialogBase(parent),
    m_pActivityPage(nullptr),
    m_pStateWidget(pWidget),
    m_bChangesMade(false),
    pageActivity(nullptr)
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
        if (m_pStateWidget->stateType() == StateWidget::Combined) {
            m_pStateWidget->setDiagramLink(m_GenPageWidgets.diagramLinkWidget->currentID());
        }
        m_pStateWidget->setName(m_GenPageWidgets.nameLE->text());
        m_GenPageWidgets.docWidget->apply();
    }
    else if (item == pageActivity) {
        if (m_pActivityPage) {
            m_pActivityPage->updateActivities();
        }
    }
    else if (item == pageStyle) {
        applyStylePage();
    }
    else if (item == pageFont) {
        applyFontPage(m_pStateWidget);
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

    pageGeneral = createPage(i18nc("general page", "General"), i18n("General Properties"),
                             Icon_Utils::it_Properties_General, page);

    m_GenPageWidgets.generalGB = new QGroupBox(i18n("Properties"));
    topLayout->addWidget(m_GenPageWidgets.generalGB);

    QGridLayout * generalLayout = new QGridLayout(m_GenPageWidgets.generalGB);
    generalLayout->setSpacing(Dialog_Utils::spacingHint());
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
    case StateWidget::Combined:
        typeStr = i18nc("combined state in statechart", "Combined");
        break;
    default:
        typeStr = QString::fromLatin1("???");
        break;
    }
    int row = 0;
    Dialog_Utils::makeLabeledEditField(generalLayout, row++,
                                       m_GenPageWidgets.typeL, i18n("State type:"),
                                       m_GenPageWidgets.typeLE, typeStr);
    m_GenPageWidgets.typeLE->setEnabled(false);

    Dialog_Utils::makeLabeledEditField(generalLayout, row++,
                                       m_GenPageWidgets.nameL, i18n("State name:"),
                                       m_GenPageWidgets.nameLE);

    if (type != StateWidget::Normal && type != StateWidget::Combined) {
        m_GenPageWidgets.nameLE->setEnabled(false);
        m_GenPageWidgets.nameLE->setText(QString());
    } else
        m_GenPageWidgets.nameLE->setText(m_pStateWidget->name());

    if (type == StateWidget::Combined) {
        m_GenPageWidgets.diagramLinkWidget = new SelectDiagramWidget(i18n("Linked diagram:"), this);
        m_GenPageWidgets.diagramLinkWidget->setupWidget(Uml::DiagramType::State,
            m_pStateWidget->linkedDiagram() ? m_pStateWidget->linkedDiagram()->name() : QString(),
            m_pStateWidget->umlScene()->name(), false);
        m_GenPageWidgets.diagramLinkWidget->addToLayout(generalLayout, row++);
    }

    m_GenPageWidgets.docWidget = new DocumentationWidget(m_pStateWidget);
    generalLayout->addWidget(m_GenPageWidgets.docWidget, row, 0, 1, 2);
    m_GenPageWidgets.nameLE->setFocus();
}

/**
 * Sets up the activity page.
 */
void StateDialog::setupActivityPage()
{
    m_pActivityPage = new ActivityPage(nullptr, m_pStateWidget);
    pageActivity = createPage(i18n("Activities"), i18n("Activities"),
                              Icon_Utils::it_Properties_Activities, m_pActivityPage);
}

