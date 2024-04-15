/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "autolayoutoptionpage.h"

// local includes
#include "layoutgenerator.h"

// kde includes
#include <QLineEdit>
#include <KLocalizedString>

// qt includes
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

/**
 * Constructor.general
 * @param parent   the parent (wizard) of this wizard page
 */
AutoLayoutOptionPage::AutoLayoutOptionPage(QWidget* parent)
  : DialogPageBase(parent)
{
    setupUi(this);
    m_autoDotPath->setChecked(Settings::optionState().autoLayoutState.autoDotPath);
    m_dotPath->setText(Settings::optionState().autoLayoutState.dotPath);
    m_showExportLayout->setChecked(Settings::optionState().autoLayoutState.showExportLayout);
    connect(m_autoDotPath, SIGNAL(toggled(bool)), this, SLOT(slotAutoDotPathCBClicked(bool)));

    if (Settings::optionState().autoLayoutState.autoDotPath) {
        m_dotPath->setEnabled (false);
        m_dotPath->setText(LayoutGenerator::currentDotPath());
    }
}

/**
 * destructor
 */
AutoLayoutOptionPage::~AutoLayoutOptionPage()
{
}

/**
 * sets default values
 */
void AutoLayoutOptionPage::setDefaults()
{
    m_autoDotPath->setChecked(true);
    m_showExportLayout->setChecked(false);
}

/**
 * Reads the set values from their corresponding widgets, writes them back to
 * the data structure, and notifies clients.
 */
void AutoLayoutOptionPage::apply()
{
    Settings::optionState().autoLayoutState.autoDotPath = m_autoDotPath->isChecked();
    Settings::optionState().autoLayoutState.dotPath = m_autoDotPath->isChecked() ? QString()
                                                                                 : m_dotPath->text();
    Settings::optionState().autoLayoutState.showExportLayout = m_showExportLayout->isChecked();
    Q_EMIT applyClicked();
}

void AutoLayoutOptionPage::slotAutoDotPathCBClicked(bool value)
{
    if (value)
        m_dotPath->setText(LayoutGenerator::currentDotPath());
}
