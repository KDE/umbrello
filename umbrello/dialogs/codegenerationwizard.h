/***************************************************************************
                          codegenerationwizard.h  -  description
                             -------------------
    begin                : Wed Jul 24 2002
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
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CODEGENERATIONWIZARD_H
#define CODEGENERATIONWIZARD_H

#include <qwidget.h>
#include <qptrlist.h>
#include "codegenerationwizardbase.h"
#include "settingsdlg.h"
#include "../umlclassifierlist.h"

class UMLApp;
class UMLDoc;
class CodeGenerator;
class CodeGenerationOptionsPage;

/**
 * @author Luis De la Parra
 * based on wizard from Paul Hensgen
 */

class CodeGenerationWizard : public CodeGenerationWizardBase {
    Q_OBJECT
public:
    CodeGenerationWizard(UMLClassifierList *classList);
    ~CodeGenerationWizard();

    void showPage(QWidget *);

    int exec() {
        return QWizard::exec();
    }
protected slots:

    /**
     * Adds the classes selected in the available classes list to the
     * list of classes used to generate the code.
     */
    void selectClass();

    /**
     * Removes the classes selected in the selected classes list from the
     * list of classes used to generate the code.
     */
    void deselectClass();
    void populateStatusList();
    void generateCode();
    void classGenerated(UMLClassifier* concept, bool generated);

private slots:
    void changeLanguage();


private:
    CodeGenerator* generator();

    /**
     * Moves the selected items from first list to second list.
     * The selected items are removed from the first list and added to the
     * second. An item is added to the second list only if it isn't already
     * there (no duplicated items are created).
     */
    void moveSelectedItems(QListView* fromList, QListView* toList);

    UMLApp* m_app;
    UMLDoc* m_doc;
    CodeGenerationOptionsPage* m_CodeGenerationOptionsPage;
};

#endif
