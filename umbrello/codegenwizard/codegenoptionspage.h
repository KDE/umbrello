/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2002 Luis De la Parra <luis@delaparra.org>
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/
#ifndef CODEGENOPTIONSPAGE_H
#define CODEGENOPTIONSPAGE_H

// app includes
#include "ui_codegenoptionspage.h"
#include "basictypes.h"
#include "codegenerationpolicy.h"

// qt includes
#include <QWizardPage>

class QRadioButton;
class CodeGenerationPolicy;
class CodeGenerationPolicyPage;

/**
 * This class is used in the code generation wizard and
 * also in SettingsDialog.
 *
 * @author Luis De la Parra
 * @author Brian Thomas
 * @author Andi Fischer
 */
class CodeGenOptionsPage : public QWizardPage, private Ui::CodeGenOptionsPage
{
    Q_OBJECT
public:
    explicit CodeGenOptionsPage(QWidget *parent = 0);
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

protected Q_SLOTS:
     void activeLanguageChanged(int id);
     void updateCodeGenerationPolicyTab();
     void browseClicked();

private Q_SLOTS:
    void changeLanguage();

Q_SIGNALS:
     void applyClicked();
     void languageChanged();
     void syncCodeDocumentsToParent();

};

#endif
