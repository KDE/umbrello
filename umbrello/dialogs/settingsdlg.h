/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2008                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/
#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H

// qt includes
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>

// kde includes
#include <kcombobox.h>
#include <kfontdialog.h>
#include <kpagedialog.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <klineedit.h>

// app includes
#include "optionstate.h"

class UMLWidgetStylePage;
class AutoLayoutOptionPage;
class ClassOptionsPage;
class CodeImportOptionsPage;
class CodeGenOptionsPage;
class CodeViewerOptionsPage;
class GeneralOptionPage;

/**
 * @author Paul Hensgen
 * modified by brian thomas Aug-2003
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class SettingsDlg : public KPageDialog
{
    Q_OBJECT

public:
    SettingsDlg(QWidget * parent, Settings::OptionState *state);
    ~SettingsDlg();

    //public methods
    bool getChangesApplied() {
        return m_bChangesApplied;
    }

    QString getCodeGenerationLanguage();

protected:

private:
    //private structs
    struct UIWidgets {
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
        KIntSpinBox  * lineWidthB;

        QCheckBox * useFillColorCB;
    }
    ;//end struct UIWidgets

    struct FontWidgets {
        KFontChooser * chooser;
    };

    //private methods
    void setupFontPage();
    void setupUIPage();
    void setupGeneralPage();
    void setupClassPage();
    void setupCodeImportPage();
    void setupCodeGenPage();
    void setupCodeViewerPage(Settings::CodeViewerState options);
    void setupAutoLayoutPage();
    void applyPage( KPageWidgetItem* );

    //private attributes
    FontWidgets m_FontWidgets;
    UIWidgets m_UiWidgets;
    Settings::OptionState *m_pOptionState;
    UMLWidgetStylePage * m_pUserInterfacePage;
    AutoLayoutOptionPage * m_pAutoLayoutPage;
    CodeImportOptionsPage * m_pCodeImportPage;
    CodeGenOptionsPage * m_pCodeGenPage;
    CodeViewerOptionsPage * m_pCodeViewerPage;
    GeneralOptionPage * m_pGeneralPage;
    ClassOptionsPage * m_pClassPage;

    bool m_bChangesApplied;
    KPageWidgetItem *pageCodeViewer, *pageFont, *pageCodeImport, *pageCodeGen,
                    *pageUserInterface, *pageClass, *pageAutoLayout, *pageGeneral;

private slots:
    void slotApply();
    void slotOk();
    void slotDefault();
    void slotTextCBChecked(bool value);
    void slotLineCBChecked(bool value);
    void slotFillCBChecked(bool value);
    void slotGridCBChecked(bool value);
    void slotBgCBChecked(bool value);
};

#endif
