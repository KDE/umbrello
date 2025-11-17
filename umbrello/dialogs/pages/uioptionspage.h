/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UIOPTIONSPAGE_H
#define UIOPTIONSPAGE_H

#include "dialogpagebase.h"

namespace Settings {
    class OptionState;
}

class SelectLayoutTypeWidget;

class KColorButton;
class KComboBox;
class QSpinBox;

class QCheckBox;
class QGroupBox;
class QSpinBox;
class QLabel;

/**
 * A dialog page to display user interface options
 * This dialog is either embedded into @ref SettingsDialog,
 * @ref UMLViewDialog and @ref ClassPropertiesDialog
 *
 * @short A dialog page to display the user interface options.
 * @author Paul Hensgen
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UIOptionsPage : public DialogPageBase
{
    Q_OBJECT
public:
    UIOptionsPage(QWidget* parent, Settings::OptionState *options);
    virtual ~UIOptionsPage();

    void setDefaults();
    void apply();

protected:
    Settings::OptionState *m_options;

    QCheckBox * m_rightToLeftUI;
    QCheckBox * m_alignmentGuidesUI;
    QGroupBox * m_colorGB;
    QCheckBox * m_textColorCB;
    QCheckBox * m_lineColorCB;
    QCheckBox * m_fillColorCB;
    QCheckBox * m_lineWidthCB;
    QCheckBox * m_gridColorCB;
    QCheckBox * m_bgColorCB;
    KColorButton * m_textColorB;
    KColorButton * m_lineColorB;
    KColorButton * m_fillColorB;
    KColorButton * m_gridColorB;
    KColorButton * m_bgColorB;
    QSpinBox     * m_lineWidthB;
    QCheckBox * m_useFillColorCB;
    SelectLayoutTypeWidget *m_layoutTypeW;
    void setupPage();

protected Q_SLOTS:
    void slotTextCBChecked(bool value);
    void slotLineCBChecked(bool value);
    void slotFillCBChecked(bool value);
    void slotGridCBChecked(bool value);
    void slotBgCBChecked(bool value);
    void slotLineWidthCBChecked(bool value);
};
#endif
