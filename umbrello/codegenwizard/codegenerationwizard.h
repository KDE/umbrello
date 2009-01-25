/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002                                                    *
 *   Luis De la Parra <luis@delaparra.org>                                 *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#ifndef CODEGENERATIONWIZARD_H
#define CODEGENERATIONWIZARD_H

// qt includes
#include <QtGui/QWizard>

// app includes
#include "umlclassifierlist.h"

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
    enum {SelectionPage, OptionsPage, StatusPage};

    CodeGenerationWizard(UMLClassifierList *classList);
    ~CodeGenerationWizard();

    QListWidget* getSelectionListWidget();

private:
    QWizardPage* createSelectionPage(UMLClassifierList *classList);
    QWizardPage* createOptionsPage();
    QWizardPage* createStatusPage();

    CodeGenSelectPage  *m_SelectionPage;
    CodeGenOptionsPage *m_OptionsPage;
    CodeGenStatusPage  *m_StatusPage;

    static const QSize c_pageSize;
};

#endif
