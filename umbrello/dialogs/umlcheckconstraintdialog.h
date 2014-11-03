/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLCHECKCONSTRAINTDIALOG_H
#define UMLCHECKCONSTRAINTDIALOG_H

// qt includes
#include <QDialog>

class UMLDoc;
class UMLCheckConstraint;
class KLineEdit;
class KTextEdit;
class QLabel;

/**
 * A dialog page to display check constraint properties.
 *
 * @short A dialog page to display check constraint properties.
 * @author Sharan Rao
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class UMLCheckConstraintDialog : public QDialog
{
    Q_OBJECT
public:
    UMLCheckConstraintDialog(QWidget* parent, UMLCheckConstraint* pUniqueConstraint);
    ~UMLCheckConstraintDialog();

protected:
    void setupDialog();

    bool apply();

private:
    UMLCheckConstraint* m_pCheckConstraint;

    UMLDoc* m_doc;  //< the UMLDocument where all objects live

    // the GUI widgets
    QLabel* m_pNameL;
    KLineEdit* m_pNameLE;
    QLabel* m_pCheckConditionL;
    KTextEdit* m_pCheckConditionTE;

public slots:
    void slotOk();

};

#endif
