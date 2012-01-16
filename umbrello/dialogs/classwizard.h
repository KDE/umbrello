/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CLASSWIZARD_H
#define CLASSWIZARD_H

// qt includes
#include <QtGui/QWizard>

class QWizardPage;
class ClassifierListPage;
class ClassGenPage;
class UMLClassifier;
class UMLDoc;

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassWizard : public QWizard
{
    Q_OBJECT
public:
    ClassWizard(UMLDoc* doc);
    ~ClassWizard();

private:
    UMLDoc             * m_doc;      //< Document currently opened.
    UMLClassifier      * m_pClass;    //< Class to create.

    ClassGenPage       * m_pGenPage;  //< General class info.
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

protected slots:
    void showHelp();

};

#endif
