/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016,2019                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLDATATYPEWIDGET_H
#define UMLDATATYPEWIDGET_H

#include "comboboxwidgetbase.h"

class UMLAttribute;
class UMLEntityAttribute;
class UMLClassifier;
class UMLClassifierListItem;
class UMLOperation;
class UMLTemplate;

class UMLDatatypeWidget : public ComboBoxWidgetBase
{
    Q_OBJECT
public:
    UMLDatatypeWidget(UMLAttribute *attribute, QWidget *parent=0);
    UMLDatatypeWidget(UMLClassifierListItem *datatype, QWidget *parent=0);
    UMLDatatypeWidget(UMLEntityAttribute *entityAttribute, QWidget *parent=0);
    UMLDatatypeWidget(UMLOperation *operation, QWidget *parent=0);
    UMLDatatypeWidget(UMLTemplate *_template, QWidget *parent=0);

    bool apply();
    void reset();

Q_SIGNALS:
    void editTextChanged(const QString &);

protected:
    UMLAttribute *m_attribute;
    UMLClassifierListItem *m_datatype;
    UMLEntityAttribute *m_entityAttribute;
    UMLOperation *m_operation;
    UMLTemplate *m_template;
    UMLClassifier *m_parent;
    void init();
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
