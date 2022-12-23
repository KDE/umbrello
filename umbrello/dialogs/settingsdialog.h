/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

// app includes
#include "multipagedialogbase.h"
#include "optionstate.h"

// kde includes
#include <kcombobox.h>
#include <kcolorbutton.h>
#include <klineedit.h>

class AutoLayoutOptionPage;
class ClassOptionsPage;
class CodeImportOptionsPage;
class CodeGenOptionsPage;
class CodeViewerOptionsPage;
class DontAskAgainWidget;
class GeneralOptionPage;
class KColorButton;
class KFontChooser;
class UIOptionsPage;

class QCheckBox;
#if QT_VERSION >= 0x050000
class QFontDialog;
#endif
class QGroupBox;
class QSpinBox;
class UMLWidgetStylePage;

/**
 * @author Paul Hensgen
 * modified by brian thomas Aug-2003
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class SettingsDialog : public MultiPageDialogBase
{
    Q_OBJECT

public:
    SettingsDialog(QWidget * parent, Settings::OptionState *state);
    ~SettingsDialog();

    void setCurrentPage(PageType page);

    //public methods
    bool getChangesApplied() const {
        return m_bChangesApplied;
    }

    QString getCodeGenerationLanguage();

protected:

private:
    //private structs
    struct UIWidgets {
        QCheckBox * rightToLeftUI;
        QGroupBox * colorGB;

        QCheckBox * textColorCB;
        QCheckBox * lineColorCB;
        QCheckBox * fillColorCB;
        QCheckBox * lineWidthCB;
        QCheckBox * gridColorCB;
        QCheckBox * bgColorCB;

        KColorButton * textColorB;
        KColorButton * lineColorB;
        KColorButton * fillColorB;
        KColorButton * gridColorB;
        KColorButton * bgColorB;
        QSpinBox     * lineWidthB;
        QCheckBox * useFillColorCB;
    }
    ;//end struct UIWidgets

    //private methods
    void setupUIPage();
    void setupGeneralPage();
    void setupClassPage();
    void setupCodeImportPage();
    void setupCodeGenPage();
    void setupCodeViewerPage(Settings::CodeViewerState options);
    void setupAutoLayoutPage();
    void applyPage(KPageWidgetItem*);

    //private attributes
    DontAskAgainWidget *m_dontAskAgainWidget;
    Settings::OptionState *m_pOptionState;
    UMLWidgetStylePage * m_pUserInterfacePage;
    AutoLayoutOptionPage * m_pAutoLayoutPage;
    CodeImportOptionsPage * m_pCodeImportPage;
    CodeGenOptionsPage * m_pCodeGenPage;
    CodeViewerOptionsPage * m_pCodeViewerPage;
    GeneralOptionPage * m_pGeneralPage;
    ClassOptionsPage * m_pClassPage;
    UIOptionsPage *m_uiOptionsPage;

    bool m_bChangesApplied;
    KPageWidgetItem *pageCodeViewer, *pageFont, *pageCodeImport, *pageCodeGen,
                    *pageUserInterface, *pageClass, *pageAutoLayout, *pageGeneral;

private slots:
    void slotApply();
    void slotOk();
    void slotDefault();
};

#endif
