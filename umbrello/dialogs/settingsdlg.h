/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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

#include "../codegenerator.h"
#include "../optionstate.h"

class CodeGenerationOptionsPage;
class CodeViewerOptionsPage;


/**
 * @author Paul Hensgen
 * modified by brian thomas Aug-2003
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

using namespace Settings;

class SettingsDlg : public KDialogBase {

	Q_OBJECT

public:
	SettingsDlg( QWidget * parent, OptionState state,
		     QDict<GeneratorInfo> ldict, QString activeLanguage, CodeGenerator * gen);
	~SettingsDlg();

	//public methods
	OptionState getOptionState() {
		return m_OptionState;
	}
	bool getChangesApplied() {
		return m_bChangesApplied;
	}

	void setCodeGenerator(CodeGenerator *gen);
	QString getCodeGenerationLanguage();

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
		QGroupBox * autosaveGB;
		QGroupBox * startupGB;

		KComboBox * timeKB;
		KComboBox * diagramKB;

		QCheckBox * autosaveCB;
		QCheckBox * logoCB;
		QCheckBox * tipCB;
		QCheckBox * loadlastCB;

		QLabel * startL;
		QLabel *  autosaveL;
	}
	;//end struct GeneralWidgets

	struct ClassWidgets {
		QGroupBox * visibilityGB;
		QGroupBox * scopeGB;

		QCheckBox * showScopeCB;
		QCheckBox * showAttsCB;
		QCheckBox * showOpsCB;
		QCheckBox * showStereotypeCB;
		QCheckBox * showAttSigCB;
		QCheckBox * showPackageCB;

		QCheckBox * showOpSigCB;

		QLabel * attributeLabel;
		QLabel * operationLabel;

		QComboBox* m_pAttribScopeCB;
		QComboBox* m_pOperationScopeCB;

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
  	void setupCodeGenPage( CodeGenerator *gen, QDict<GeneratorInfo> ldict, QString activeLanguage);
	void setupCodeViewerPage(CodeViewerState options);
	void applyPage( Page page );

	//private attributes
	FontWidgets m_FontWidgets;
	GeneralWidgets m_GeneralWidgets;
	UIWidgets m_UiWidgets;
	ClassWidgets m_ClassWidgets;
	OptionState m_OptionState;
	CodeGenerationOptionsPage * m_pCodeGenPage;
	CodeViewerOptionsPage * m_pCodeViewerPage;

	KConfig * m_pCfg;
	bool m_bChangesApplied;

	//signals
signals:
	//slots
private slots:
	void slotApply();
	void slotOk();
	void slotDefault();
	void slotLineBClicked();
	void slotFillBClicked();
	void slotAutosaveCBClicked();
};

#endif
