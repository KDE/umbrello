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
    void setClassifierList(UMLClassifierList *classList);
    bool isComplete() const;
    QListWidget* getSelectionListWidget();

private:
    static void moveSelectedItems(QListWidget* fromList, QListWidget* toList);

protected slots:
    void selectClass();
    void deselectClass();

};

#endif
