/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CLASSWIZARD_H
#define CLASSWIZARD_H

// qt includes
#include <QWizard>

class QWizardPage;
class ClassifierListPage;
class ClassGeneralPage;
class UMLClassifier;
class UMLDoc;

/**
 * @author Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ClassWizard : public QWizard
{
    Q_OBJECT
public:
    explicit ClassWizard(UMLDoc* doc);
    ~ClassWizard();

private:
    UMLDoc             * m_doc;      //< Document currently opened.
    UMLClassifier      * m_pClass;    //< Class to create.

    ClassGeneralPage   * m_pGenPage;  //< General class info.
    ClassifierListPage * m_pAttPage;  //< Class attributes.
    ClassifierListPage * m_pOpPage;   //< Class operations.

    QWizardPage        * m_GeneralPage;
    QWizardPage        * m_AttributesPage;
    QWizardPage        * m_OperationsPage;

    QWizardPage* createGeneralPage();
    QWizardPage* createAttributesPage();
    QWizardPage* createOperationsPage();

    void next();
    void back();
    void accept();
    void reject();

protected Q_SLOTS:
    void showHelp();

};

#endif
