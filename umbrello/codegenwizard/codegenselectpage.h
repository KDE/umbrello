/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#ifndef CODEGENSELECTPAGE_H
#define CODEGENSELECTPAGE_H

// qt includes
#include <QtGui/QWizardPage>

// app includes
#include "ui_codegenselectpage.h"
#include "umlclassifierlist.h"

/**
 * @author Luis De la Parra
 * @author Brian Thomas
 * @author Andi Fischer
 */

class CodeGenSelectPage : public QWizardPage, private Ui::CodeGenSelectPage
{
    Q_OBJECT
public:
    CodeGenSelectPage(QWidget * parent = 0);
    ~CodeGenSelectPage();

    /**
     * Loads the available classes for selection / deselection
     * into the list widget.
     * @param classList   the available classes for generation
     */
    void setClassifierList(UMLClassifierList *classList);

    /**
     * Reimplemented QWizardPage method the enable / disable the next button.
     * @return   complete state 
     */
    bool isComplete() const;

    /**
     * Returns the list widget, which holds the classes for generation.
     * @return   the list widget of the selected classes
     */
    QListWidget* getSelectionListWidget();

private:
    static void moveSelectedItems(QListWidget* fromList, QListWidget* toList);

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

};

#endif
