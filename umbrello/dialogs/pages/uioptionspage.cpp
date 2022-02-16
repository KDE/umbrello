/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "uioptionspage.h"

//// local includes
#include "optionstate.h"
#include "umbrellosettings.h"

#include "selectlayouttypewidget.h"

//// kde includes
#include <KLocalizedString>
#include <KColorButton>
#include <KIntSpinBox>

//// qt includes
#include <QCheckBox>
#include <QApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

//TODO don't do that, but it's better than hardcoded in the functions body
#define FILL_COLOR QColor(255, 255, 192)
#define LINK_COLOR Qt::red
#define TEXT_COLOR Qt::black

/**
 * Constructor - observe and modify an OptionState structure
 *
 * @param pParent Parent widget
 * @param options Settings to read from/save into
 */
UIOptionsPage::UIOptionsPage(QWidget* pParent, Settings::OptionState *options)
  : DialogPageBase(pParent),
    m_options(options)
{
    setupPage();
}

/**
 * Destructor
 */
UIOptionsPage::~UIOptionsPage()
{
}

/**
 * Creates the page with the correct options for the class/interface
 */
void UIOptionsPage::setupPage()
{
    QVBoxLayout* uiPageLayout = new QVBoxLayout(this);
    QGroupBox *box = new QGroupBox(i18nc("General options", "General"), this);
    QGridLayout * otherLayout = new QGridLayout(box);
    //otherLayout->setSpacing(spacingHint());
    otherLayout->setMargin(fontMetrics().height());
    uiPageLayout->addWidget(box);

    m_rightToLeftUI = new QCheckBox(i18n("Right to left user interface"), box);
    m_rightToLeftUI->setChecked(UmbrelloSettings::rightToLeftUI());
    otherLayout->addWidget(m_rightToLeftUI, 0, 0);

    QGroupBox *boxAssocs = new QGroupBox(i18nc("Association options", "Associations"), this);
    QGridLayout *layoutAssocs = new QGridLayout(boxAssocs);
    layoutAssocs->setMargin(fontMetrics().height());
    uiPageLayout->addWidget(boxAssocs);

    m_angularLinesCB = new QCheckBox(i18n("Use angular association lines"), boxAssocs);
    m_angularLinesCB->setChecked(m_options->generalState.angularlines);
    layoutAssocs->addWidget(m_angularLinesCB, 0, 0);

    m_layoutTypeW = new SelectLayoutTypeWidget(i18n("Create new association lines as:"), Settings::optionState().generalState.layoutType, boxAssocs);
    m_layoutTypeW->addToLayout(layoutAssocs, 1);

    m_colorGB = new QGroupBox(i18nc("color group box", "Color"), this);
    QGridLayout * colorLayout = new QGridLayout(m_colorGB);
    //colorLayout->setSpacing(spacingHint());
    colorLayout->setMargin(fontMetrics().height());
    uiPageLayout->addWidget(m_colorGB);

    uiPageLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    m_textColorCB = new QCheckBox(i18n("Custom text color"), m_colorGB);
    colorLayout->addWidget(m_textColorCB, 0, 0);

    m_textColorB = new KColorButton(m_options->uiState.textColor, m_colorGB);
    //m_lineColorB->setObjectName(m_colorGB);
    colorLayout->addWidget(m_textColorB, 0, 1);

    m_lineColorCB = new QCheckBox(i18n("Custom line color"), m_colorGB);
    colorLayout->addWidget(m_lineColorCB, 1, 0);

    m_lineColorB = new KColorButton(m_options->uiState.lineColor, m_colorGB);
    //m_lineColorB->setObjectName(m_colorGB);
    colorLayout->addWidget(m_lineColorB, 1, 1);

//     m_lineDefaultB = new QPushButton(i18n("D&efault Color"), m_colorGB);
//     colorLayout->addWidget(m_lineDefaultB, 0, 2);

    m_fillColorCB = new QCheckBox(i18n("Custom fill color"), m_colorGB);
    colorLayout->addWidget(m_fillColorCB, 2, 0);

    m_fillColorB = new KColorButton(m_options->uiState.fillColor, m_colorGB);
    colorLayout->addWidget(m_fillColorB, 2, 1);

    m_gridColorCB = new QCheckBox(i18n("Custom grid color"), m_colorGB);
    colorLayout->addWidget(m_gridColorCB, 3, 0);

    m_gridColorB = new KColorButton(m_options->uiState.gridDotColor, m_colorGB);
    colorLayout->addWidget(m_gridColorB, 3, 1);

    m_bgColorCB = new QCheckBox(i18n("Custom background color"), m_colorGB);
    colorLayout->addWidget(m_bgColorCB, 4, 0);

    m_bgColorB = new KColorButton(m_options->uiState.backgroundColor, m_colorGB);
    colorLayout->addWidget(m_bgColorB, 4, 1);

    m_lineWidthCB = new QCheckBox(i18n("Custom line width"), m_colorGB);
    colorLayout->addWidget(m_lineWidthCB, 5, 0);

#if QT_VERSION >= 0x050000
    m_lineWidthB = new QSpinBox(m_colorGB);
    m_lineWidthB->setMinimum(0);
    m_lineWidthB->setMaximum(10);
    m_lineWidthB->setSingleStep(1);
    m_lineWidthB->setValue(m_options->uiState.lineWidth);
#else
    m_lineWidthB = new KIntSpinBox(0, 10, 1, m_options->uiState.lineWidth, m_colorGB);
#endif
    colorLayout->addWidget(m_lineWidthB, 5, 1);

    m_useFillColorCB = new QCheckBox(i18n("&Use fill color"), m_colorGB);
    //colorLayout->setRowStretch(3, 2);
    colorLayout->addWidget(m_useFillColorCB, 6, 0);
    m_useFillColorCB->setChecked(m_options->uiState.useFillColor);

    //connect button signals up
    connect(m_textColorCB, SIGNAL(toggled(bool)), this, SLOT(slotTextCBChecked(bool)));
    connect(m_lineColorCB, SIGNAL(toggled(bool)), this, SLOT(slotLineCBChecked(bool)));
    connect(m_fillColorCB, SIGNAL(toggled(bool)), this, SLOT(slotFillCBChecked(bool)));
    connect(m_gridColorCB, SIGNAL(toggled(bool)), this, SLOT(slotGridCBChecked(bool)));
    connect(m_bgColorCB, SIGNAL(toggled(bool)), this, SLOT(slotBgCBChecked(bool)));
    connect(m_lineWidthCB, SIGNAL(toggled(bool)), this, SLOT(slotLineWidthCBChecked(bool)));

    // initial setup
    slotTextCBChecked(false);
    slotLineCBChecked(false);
    slotFillCBChecked(false);
    slotGridCBChecked(false);
    slotBgCBChecked(false);
    slotLineWidthCBChecked(false);
}

void UIOptionsPage::setDefaults()
{
    m_useFillColorCB->setChecked(true);
    m_textColorCB->setChecked(false);
    m_lineColorCB->setChecked(false);
    m_fillColorCB->setChecked(false);
    m_gridColorCB->setChecked(false);
    m_bgColorCB->setChecked(false);
    m_lineWidthCB->setChecked(false);
    slotTextCBChecked(false);
    slotLineCBChecked(false);
    slotFillCBChecked(false);
    slotGridCBChecked(false);
    slotBgCBChecked(false);
    slotLineWidthCBChecked(false);
    m_rightToLeftUI->setChecked(false);
    m_layoutTypeW->setCurrentLayout(Uml::LayoutType::Direct);
    m_angularLinesCB->setChecked(false);
}

/**
 * apply changes
 */
void UIOptionsPage::apply()
{
    m_options->uiState.useFillColor = m_useFillColorCB->isChecked();
    m_options->uiState.fillColor = m_fillColorB->color();
    m_options->uiState.textColor = m_textColorB->color();
    m_options->uiState.lineColor = m_lineColorB->color();
    m_options->uiState.lineWidth = m_lineWidthB->value();
    m_options->uiState.backgroundColor = m_bgColorB->color();
    m_options->uiState.gridDotColor = m_gridColorB->color();
    m_options->generalState.layoutType = m_layoutTypeW->currentLayout();
    m_options->generalState.angularlines = m_angularLinesCB->isChecked();
    UmbrelloSettings::setRightToLeftUI(m_rightToLeftUI->isChecked());
    qApp->setLayoutDirection(UmbrelloSettings::rightToLeftUI() ? Qt::RightToLeft : Qt::LeftToRight);
}

void UIOptionsPage::slotTextCBChecked(bool value)
{
    if (value == false) {
        m_textColorB->setColor(TEXT_COLOR);
        m_textColorB->setDisabled(true);
    }
    else {
        m_textColorB->setDisabled(false);
    }
}

void UIOptionsPage::slotLineCBChecked(bool value)
{
    if (value == false) {
        m_lineColorB->setColor(LINK_COLOR);
        m_lineColorB->setDisabled(true);
    }
    else {
        m_lineColorB->setDisabled(false);
    }
}

void UIOptionsPage::slotFillCBChecked(bool value)
{
    if (value == false) {
        m_fillColorB->setColor(FILL_COLOR);
        m_fillColorB->setDisabled(true);
    }
    else {
        m_fillColorB->setDisabled(false);
    }
}

void UIOptionsPage::slotGridCBChecked(bool value)
{
    if (value == false) {
        QPalette palette;
        m_gridColorB->setColor(palette.alternateBase().color());
        m_gridColorB->setDisabled(true);
    }
    else {
        m_gridColorB->setDisabled(false);
    }
}

void UIOptionsPage::slotBgCBChecked(bool value)
{
    if (value == false) {
        QPalette palette;
        m_bgColorB->setColor(palette.base().color());
        m_bgColorB->setDisabled(true);
    }
    else {
        m_bgColorB->setDisabled(false);
    }
}

void UIOptionsPage::slotLineWidthCBChecked(bool value)
{
    if (value == false) {
        m_lineWidthB->setValue(0);
        m_lineWidthB->setDisabled(true);
    }
    else {
        m_lineWidthB->setDisabled(false);
    }
}
