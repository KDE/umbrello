/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLDATATYPEWIDGET_H
#define UMLDATATYPEWIDGET_H

#include <QWidget>

class KComboBox;
class QGridLayout;
class QLabel;
class UMLClassifierListItem;
class UMLOperation;

class UMLDatatypeWidget : public QWidget
{
public:
    UMLDatatypeWidget(UMLOperation *operation, QWidget *parent=0);
    UMLDatatypeWidget(UMLClassifierListItem *datatype, QWidget *parent=0);
    ~UMLDatatypeWidget();

    void addToLayout(QGridLayout *layout, int row, int startColumn = 0);
    QString currentText() const;
    bool apply();
    void reset();

protected:
    QLabel *m_label;
    KComboBox *m_comboBox;
    UMLClassifierListItem *m_datatype;
    UMLOperation *m_operation;
    void init();
    void insertTypesSortedOperation(const QString &type);
    void insertTypesSortedAttribute(const QString &type);
};

#endif
