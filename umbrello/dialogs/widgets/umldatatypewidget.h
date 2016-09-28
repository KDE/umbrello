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

#include "ui_umldatatypewidget.h"
#include <QWidget>

class UMLAttribute;
class UMLEntityAttribute;
class UMLClassifier;
class UMLClassifierListItem;
class UMLOperation;
class UMLTemplate;

class UMLDatatypeWidget : public QWidget
{
public:
    UMLDatatypeWidget(QWidget *parent = 0);
    ~UMLDatatypeWidget();

    void setAttribute(UMLAttribute *attribute);
    void setClassifierItem(UMLClassifierListItem *datatype);
    void setEntityAttribute(UMLEntityAttribute *entityAttribute);
    void setOperation(UMLOperation *operation);
    void setTemplate(UMLTemplate *_template);

    void addToLayout(QGridLayout *layout, int row, int startColumn = 0);
    bool apply();
    void reset();

private:
    Ui::UMLDataTypeWidget *ui;

protected:
    UMLAttribute *m_attribute;
    UMLClassifierListItem *m_datatype;
    UMLOperation *m_operation;
    UMLEntityAttribute *m_entityAttribute;
    UMLTemplate *m_template;
    UMLClassifier *m_parent;
    bool applyAttribute();
    bool applyEntityAttribute();
    bool applyOperation();
    bool applyParameter();
    bool applyTemplate();
    void initTypesBox(QStringList &types, const QString &type);
    void insertTypesFromConcepts(QStringList &types, bool fullName = true);
    void insertTypesFromDatatypes(QStringList &types);
    void insertTypesSortedAttribute(const QString &type);
    void insertTypesSortedEntityAttribute(const QString &type);
    void insertTypesSortedOperation(const QString &type);
    void insertTypesSortedParameter(const QString &type);
    void insertTypesSortedTemplate(const QString &type);

};

#endif
