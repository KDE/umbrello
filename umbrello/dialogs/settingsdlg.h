/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/


#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H
//qt includes
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qdict.h>
//kde includes
#include <kcombobox.h>
#include <kfontdialog.h>
#include <kdialogbase.h>
#include <kcolorbutton.h>
#include <knuminput.h>
//app includes

#include "../optionstate.h"

class CodeGenerationOptionsPage;
class CodeViewerOptionsPage;
class CodeGenerator;

/**
 * @author Paul Hensgen
 * modified by brian thomas Aug-2003
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class SettingsDlg : public KDialogBase {

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
    /**
    * Inserts @p type into the type-combobox as well as its completion object.
    */
    void insertDiagram( const QString& type, int index = -1 );
    /**
    * Inserts @p type into the type-combobox as well as its completion object.
    */
    void insertAttribScope( const QString& type, int index = -1 );
    /**
    * Inserts @p type into the type-combobox as well as its completion object.
    */
    void insertOperationScope( const QString& type, int index = -1 );

private:
    //private structs
    struct UIWidgets {
        QGroupBox * colorGB;

        QLabel * lineColorL;
        QLabel * fillColorL;
        QLabel * lineWidthL;

        QPushButton * lineDefaultB;
        QPushButton * fillDefaultB;
        QPushButton * lineWidthDefaultB;

        KColorButton * lineColorB;
        KColorButton * fillColorB;
        KIntSpinBox  * lineWidthB;

        QCheckBox * useFillColorCB;
    }
    ;//end struct UIWidgets

    struct GeneralWidgets {
        QGroupBox * miscGB;
        QGroupBox * autosaveGB;
        QGroupBox * startupGB;

        KIntSpinBox * timeISB;
        KComboBox * diagramKB;

        QCheckBox * undoCB;
        QCheckBox * tabdiagramsCB;
        QCheckBox * newcodegenCB;
        QCheckBox * angularLinesCB;
        QCheckBox * autosaveCB;
        QCheckBox * logoCB;
        QCheckBox * tipCB;
        QCheckBox * loadlastCB;

        // 2004-05-17 Achim Spangler: Allow definition of Suffix for autosave
        // ( Default: ".xmi" )
        QLineEdit * autosaveSuffixT;
        QLabel    * autosaveSuffixL;
        // End AutoSave Suffix

        QLabel * startL;
        QLabel *  autosaveL;
    }
    ;//end struct GeneralWidgets

    struct ClassWidgets {
        QGroupBox * visibilityGB;
        QGroupBox * scopeGB;

        QCheckBox * showVisibilityCB;
        QCheckBox * showAttsCB;
        QCheckBox * showOpsCB;
        QCheckBox * showStereotypeCB;
        QCheckBox * showAttSigCB;
        QCheckBox * showPackageCB;

        QCheckBox * showOpSigCB;

        QLabel * attributeLabel;
        QLabel * operationLabel;

        KComboBox* m_pAttribScopeCB;
        KComboBox* m_pOperationScopeCB;

    }
    ;//end struct ClassWidgets

    struct FontWidgets {
        KFontChooser * chooser;
    };

    //private methods
    void setupFontPage();
    void setupUIPage();
    void setupGeneralPage();
    void setupClassPage();
    void setupCodeGenPage();
    void setupCodeViewerPage(Settings::CodeViewerState options);
    void applyPage( Settings::Page page );

    //private attributes
    FontWidgets m_FontWidgets;
    GeneralWidgets m_GeneralWidgets;
    UIWidgets m_UiWidgets;
    ClassWidgets m_ClassWidgets;
    Settings::OptionState *m_pOptionState;
    CodeGenerationOptionsPage * m_pCodeGenPage;
    CodeViewerOptionsPage * m_pCodeViewerPage;

    KConfig * m_pCfg;
    bool m_bChangesApplied;

private slots:
    void slotApply();
    void slotOk();
    void slotDefault();
    void slotLineBClicked();
    void slotFillBClicked();
    void slotAutosaveCBClicked();
};

#endif
