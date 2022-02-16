/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2002 Luis De la Parra <luis@delaparra.org>
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/
#ifndef CODEGENERATIONWIZARD_H
#define CODEGENERATIONWIZARD_H

// app includes
#include "umlclassifierlist.h"

// qt includes
#include <QWizard>

class QWizardPage;
class QListWidget;
class CodeGenSelectPage;
class CodeGenOptionsPage;
class CodeGenStatusPage;

/**
 * @author Luis De la Parra
 * based on wizard from Paul Hensgen
 * @author Andi Fischer refactored and ported to QWizard
 */
class CodeGenerationWizard : public QWizard 
{
    Q_OBJECT
public:
    enum {OptionsPage, SelectionPage, StatusPage};

    explicit CodeGenerationWizard(UMLClassifierList *classList);
    ~CodeGenerationWizard();

    QListWidget* getSelectionListWidget();

protected slots:
    void slotLanguageChanged();

private:
    QWizardPage* createSelectionPage(UMLClassifierList *classList);
    QWizardPage* createOptionsPage();
    QWizardPage* createStatusPage();

    CodeGenSelectPage  *m_SelectionPage;
    CodeGenOptionsPage *m_OptionsPage;
    CodeGenStatusPage  *m_StatusPage;

};

#endif
