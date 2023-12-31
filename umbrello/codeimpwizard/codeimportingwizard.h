/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef CODEIMPORTINGWIZARD_H
#define CODEIMPORTINGWIZARD_H

// qt includes
#include <QWizard>

class QFileInfo;
class QWizardPage;
class CodeImpSelectPage;
class CodeImpStatusPage;

/**
 * @author Andi Fischer
 */
class CodeImportingWizard : public QWizard 
{
    Q_OBJECT
public:
    enum { SelectionPage, StatusPage };

    CodeImportingWizard();
    ~CodeImportingWizard();
    void setupPages();

    QList<QFileInfo> selectedFiles();

private:
    QWizardPage* createSelectionPage();
    QWizardPage* createStatusPage();

    CodeImpSelectPage  *m_SelectionPage;
    CodeImpStatusPage  *m_StatusPage;

};

#endif
