/***************************************************************************
                          codegenerationoptionspage.h  -  description
                             -------------------
    begin                : Thu Jul 25 2002
    copyright            : (C) 2002 by Luis De la Parra
    email                : luis@delaparra.org
  Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CODEGENERATIONOPTIONSPAGE_H
#define CODEGENERATIONOPTIONSPAGE_H

#include <qwidget.h>
#include "codegenerationoptionsbase.h"
#include "settingsdlg.h"

/**
 * @author Luis De la Parra
 * @author Brian Thomas
 */

// 2003-07-30 : Updated for new code generation system. No longer need Yucky codegenstate
// structure.

class CodeGenerationOptionsPage : public CodeGenerationOptionsBase  {
	Q_OBJECT
public:
	CodeGenerationOptionsPage(CodeGenerator * gen,
	                          QString activeLanguage, QWidget *parent=0, const char *name=0);
	~CodeGenerationOptionsPage();
	void setDefaults();
	QString getCodeGenerationLanguage();
	void updateCodeGenerationPolicyTab(CodeGenerator * gen);
	void apply();
	void setCodeGenerator ( CodeGenerator * gen);

protected:
	CodeGenerationPolicy * m_parentPolicy;

private:

	CodeGenerator * m_pCodeGenerator;
	CodeGenerationPolicyPage * m_pCodePolicyPage;
	void init (CodeGenerator * gen, QString activeLanguage);
	int overwriteToInteger(CodeGenerationPolicy::OverwritePolicy value);
	int newLineToInteger(CodeGenerationPolicy::NewLineType value);
	int indentTypeToInteger(CodeGenerationPolicy::IndentationType value);
	void setupActiveLanguageBox(QString activeLanguage); 

protected slots:
	void activeLanguageChanged(int id); 
	void browseClicked();

signals:
	void applyClicked();
	void languageChanged();
	void syncCodeDocumentsToParent();


};

#endif
