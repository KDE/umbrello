/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/
#ifndef CODEGENSELECTPAGE_H
#define CODEGENSELECTPAGE_H

// app includes
#include "ui_codegenselectpage.h"
#include "umlclassifierlist.h"

// qt includes
#include <QWizardPage>

/**
 * @author Luis De la Parra
 * @author Brian Thomas
 * @author Andi Fischer
 */
class CodeGenSelectPage : public QWizardPage, private Ui::CodeGenSelectPage
{
    Q_OBJECT
public:
    explicit CodeGenSelectPage(QWidget * parent = 0);
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
