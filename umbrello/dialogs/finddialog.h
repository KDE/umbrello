/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include "ui_finddialog.h"

#include "singlepagedialogbase.h"
#include "umlfinder.h"

/**
  * Class FindDialog provides the ui part for selecting find attributes.
  *
  * @author: Ralf Habacker <ralf.habacker@freenet.de>
  */
class FindDialog : public SinglePageDialogBase, private Ui::FindDialog
{
    Q_OBJECT
public:
    explicit FindDialog(QWidget *parent = 0);
    ~FindDialog();
    QString text() const;
    UMLFinder::Filter filter() const;
    UMLFinder::Category category() const;

protected:
    void showEvent(QShowEvent *event);

private slots:
    void slotFilterButtonClicked(int button);
};

#endif // FINDDIALOG_H
