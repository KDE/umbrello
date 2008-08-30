/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002                                                    *
 *   Luis De la Parra  <luis@delaparra.org>                                *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#ifndef CODEGENOPTIONSPAGE_H
#define CODEGENOPTIONSPAGE_H

// qt includes
#include <QtGui/QWizardPage>

// app includes
#include "ui_codegenoptionspage.h"
#include "codegenerationpolicy.h"
#include "umlnamespace.h"

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

    /**
     * Returns the user selected language used for code generation.
     * @return   the programming language name 
     */
    QString getLanguage();

    /**
     * Reads the set values from their corresponding widgets, writes them back to 
     * the data structure, and notifies clients.
     */
    void apply();

    /**
     * This function is called when leaving this wizard page.
     * Saves the made settings and checks some values.
     * @return   the success state
     */
    bool save();

    /**
     * Reimplemented QWizardPage method to validate page when user clicks next button.
     * @return   the validation state
     */
    bool validatePage();

protected:
     CodeGenerationPolicy *m_parentPolicy;

private:
     CodeGenerationPolicyPage *m_pCodePolicyPage;

    /**
     * Converts the overwrite policy value to the corresponding widget object.
     * @param value   the overwrite policy
     * @return        the corresponding widget obeject
     */
     QRadioButton* overwriteToWidget(CodeGenerationPolicy::OverwritePolicy value);

    /**
     * Converts the corresponding widget checked value to the overwrite policy.
     * @return   the overwrite policy
     */
     CodeGenerationPolicy::OverwritePolicy widgetToOverwrite();
     static int newLineToInteger(CodeGenerationPolicy::NewLineType value);
     static int indentTypeToInteger(CodeGenerationPolicy::IndentationType value);

    /**
     * Fills the language combo box with items and
     * sets the currently selected value.
     */
     void setupActiveLanguageBox();

protected slots:

    /**
     * Transform signal.
     * @param id   position in combo box
     */
     void activeLanguageChanged(int id);

    /**
     * Updates the code generation policy tab.
     */
     void updateCodeGenerationPolicyTab();

    /**
     * Slot for clicked events of the browse buttons.
     * The selected directory is written to its corresponding text field.
     */
     void browseClicked();

private slots:

    /**
     * When the user changes the language, the codegenoptions page
     * language-dependent stuff has to be updated.
     * The way to do this is to call its "apply" method.
     */
    void changeLanguage();

signals:
     void applyClicked();
     void languageChanged();
     void syncCodeDocumentsToParent();

};

#endif
