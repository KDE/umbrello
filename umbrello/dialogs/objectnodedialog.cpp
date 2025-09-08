/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "objectnodedialog.h"

// local includes
#include "debug_utils.h"
#include "documentationwidget.h"
#include "dialog_utils.h"
#include "icon_utils.h"
#include "objectnodewidget.h"
#include "umlview.h"
#include "umlapp.h"  // only needed for log{Warn,Error}

// kde includes
#include <klineedit.h>
#include <KLocalizedString>

// qt includes
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
ObjectNodeDialog::ObjectNodeDialog(QWidget *parent, ObjectNodeWidget * pWidget)
  : MultiPageDialogBase(parent),
    m_pObjectNodeWidget(pWidget),
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
void ObjectNodeDialog::slotOk()
{
    applyPage(pageItemStyle);
    applyPage(pageItemFont);
    applyPage(pageItemGeneral);
    accept();
}

/**
 * Entered when Apply button pressed.
 */
void ObjectNodeDialog::slotApply()
{
    applyPage(currentPage());
}

void ObjectNodeDialog::slotShowState()
{
    m_GenPageWidgets.stateL->show();
    m_GenPageWidgets.stateLE->show();

    m_GenPageWidgets.stateLE->setText(m_pObjectNodeWidget->state());
}

void ObjectNodeDialog::slotHideState()
{
    m_GenPageWidgets.stateL->hide();
    m_GenPageWidgets.stateLE->hide();
}

/**
 * Sets up the pages of the dialog.
 */
void ObjectNodeDialog::setupPages()
{
    setupGeneralPage();
    pageItemStyle = setupStylePage(m_pObjectNodeWidget) ;
    pageItemFont = setupFontPage(m_pObjectNodeWidget);
}

/**
 * Applies changes to the given page.
 */
void ObjectNodeDialog::applyPage(KPageWidgetItem *item)
{
    m_bChangesMade = true;
    if (item == pageItemGeneral)
    {
        m_pObjectNodeWidget->setName(m_GenPageWidgets.nameLE->text());
        m_GenPageWidgets.docWidget->apply();
        m_pObjectNodeWidget->setState(m_GenPageWidgets.stateLE->text());

        ObjectNodeWidget::ObjectNodeType newType = ObjectNodeWidget::Normal;
        if (m_GenPageWidgets.bufferRB->isChecked())
            newType = ObjectNodeWidget::Buffer;
        else if (m_GenPageWidgets.dataRB->isChecked())
            newType = ObjectNodeWidget::Data;
        else if (m_GenPageWidgets.flowRB->isChecked())
             newType = ObjectNodeWidget::Flow;

        m_pObjectNodeWidget->setObjectNodeType (newType);
    }
    else if (item == pageItemFont)
    {
        applyFontPage(m_pObjectNodeWidget);
    }
    else if (item == pageItemStyle)
    {
        applyStylePage();
    }
}

/**
 * Sets up the general page of the dialog.
 */
void ObjectNodeDialog::setupGeneralPage()
{
    QStringList types;
    types << i18n("Central Buffer") << i18n("Data Store") << i18n("ObjectFlow");
    ObjectNodeWidget::ObjectNodeType type = m_pObjectNodeWidget->objectNodeType();

    int margin = fontMetrics().height();
    QWidget *page = new QWidget();
    QVBoxLayout *topLayout = new QVBoxLayout(page);
    pageItemGeneral = createPage(i18n("General"), i18n("General Properties"),
                                 Icon_Utils::it_Properties_General, page);

    m_GenPageWidgets.generalGB = new QGroupBox(i18nc("properties group title", "Properties"));
    topLayout->addWidget(m_GenPageWidgets.generalGB);

    QGridLayout * generalLayout = new QGridLayout(m_GenPageWidgets.generalGB);
    generalLayout->setSpacing(Dialog_Utils::spacingHint());
    generalLayout->setContentsMargins(margin, margin, margin, margin);

    QString objType;
    if (type < types.count()) {
        objType = types.at((int)type);
    } else {
        logWarn1("ObjectNodeDialog::setupGeneralPage: type of ObjectNodeWidget is out of range! "
                 "Value=%1", type);
    }
    Dialog_Utils::makeLabeledEditField(generalLayout, 0,
                                    m_GenPageWidgets.typeL, i18n("Object Node type:"),
                                    m_GenPageWidgets.typeLE, objType);
    m_GenPageWidgets.typeLE->setEnabled(false);

    Dialog_Utils::makeLabeledEditField(generalLayout, 1,
                                    m_GenPageWidgets.nameL, i18n("Object Node name:"),
                                    m_GenPageWidgets.nameLE);

    Dialog_Utils::makeLabeledEditField(generalLayout, 2,
                                    m_GenPageWidgets.stateL, i18nc("enter state label", "State :"),
                                    m_GenPageWidgets.stateLE);
    m_GenPageWidgets.stateL->hide();
    m_GenPageWidgets.stateLE->hide();

    m_GenPageWidgets.bufferRB = new QRadioButton(i18n("&Central Buffer"));
    generalLayout->addWidget(m_GenPageWidgets.bufferRB, 3, 0);

    m_GenPageWidgets.dataRB = new QRadioButton(i18n("&Data Store "));
    generalLayout->addWidget(m_GenPageWidgets.dataRB, 3, 1);

    m_GenPageWidgets.flowRB = new QRadioButton(i18n("&Object Flow"));
    generalLayout->addWidget(m_GenPageWidgets.flowRB, 4, 1);

    if (type == ObjectNodeWidget::Flow)
    {
        showState();
    }

    connect(m_GenPageWidgets.bufferRB, SIGNAL(clicked()), this, SLOT(slotHideState()));
    connect(m_GenPageWidgets.dataRB, SIGNAL(clicked()), this, SLOT(slotHideState()));
    connect(m_GenPageWidgets.flowRB, SIGNAL(clicked()), this, SLOT(slotShowState()));

    ObjectNodeWidget::ObjectNodeType newType = m_pObjectNodeWidget->objectNodeType() ;

    m_GenPageWidgets.bufferRB->setChecked(newType == ObjectNodeWidget::Buffer);
    m_GenPageWidgets.dataRB->setChecked (newType == ObjectNodeWidget::Data);
    m_GenPageWidgets.flowRB->setChecked (newType == ObjectNodeWidget::Flow);

    m_GenPageWidgets.docWidget = new DocumentationWidget(m_pObjectNodeWidget);
    generalLayout->addWidget(m_GenPageWidgets.docWidget, 5, 0, 1, 2);

    if (type != ObjectNodeWidget::Buffer && type != ObjectNodeWidget::Data && type != ObjectNodeWidget::Flow) {
        m_GenPageWidgets.nameLE->setEnabled(false);
        m_GenPageWidgets.nameLE->setText(QString());
    } else
        m_GenPageWidgets.nameLE->setText(m_pObjectNodeWidget->name());
}

/**
 * Show the State entry text.
 */
void ObjectNodeDialog::showState()
{
    m_GenPageWidgets.stateL->show();
    m_GenPageWidgets.stateLE->show();

    m_GenPageWidgets.stateLE->setText(m_pObjectNodeWidget->state());
}

