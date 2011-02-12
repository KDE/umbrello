/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002                                                    *
 *   Luis De la Parra  <luis@delaparra.org>                                *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#ifndef CODEGENOPTIONSPAGE_H
#define CODEGENOPTIONSPAGE_H

// qt includes
#include <QtGui/QWizardPage>

// app includes
#include "ui_codegenoptionspage.h"
#include "basictypes.h"
#include "codegenerationpolicy.h"

class QRadioButton;
class CodeGenerationPolicy;
class CodeGenerationPolicyPage;

/**
 * This class is used in the code generation wizard and
 * also in SettingsDlg.
 *
 * @author Luis De la Parra
 * @author Brian Thomas
 * @author Andi Fischer
 */
class CodeGenOptionsPage : public QWizardPage, private Ui::CodeGenOptionsPage
{
    Q_OBJECT
public:
    CodeGenOptionsPage(QWidget *parent = 0);
    ~CodeGenOptionsPage();
    QString getLanguage();
    void apply();
    bool save();
    bool validatePage();

protected:
     CodeGenerationPolicy *m_parentPolicy;

private:
     CodeGenerationPolicyPage *m_pCodePolicyPage;

     QRadioButton* overwriteToWidget(CodeGenerationPolicy::OverwritePolicy value);
     CodeGenerationPolicy::OverwritePolicy widgetToOverwrite();
     static int newLineToInteger(CodeGenerationPolicy::NewLineType value);
     static int indentTypeToInteger(CodeGenerationPolicy::IndentationType value);
     void setupActiveLanguageBox();

protected slots:
     void activeLanguageChanged(int id);
     void updateCodeGenerationPolicyTab();
     void browseClicked();

private slots:
    void changeLanguage();

signals:
     void applyClicked();
     void languageChanged();
     void syncCodeDocumentsToParent();

};

#endif
