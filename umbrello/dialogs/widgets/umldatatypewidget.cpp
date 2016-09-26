/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016                                              *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "umldatatypewidget.h"

#include "classifierlistitem.h"
#include "classifier.h"
#include "debug_utils.h"
#include "entityattribute.h"
#include "import_utils.h"
#include "object_factory.h"
#include "operation.h"
#include "template.h"
#include "uml.h"
#include "umldoc.h"

#include <KLocalizedString>

#include <QApplication>
#include <QGridLayout>
#include <QWidget>

UMLDatatypeWidget::UMLDatatypeWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::UMLDataTypeWidget),
      m_attribute(nullptr),
      m_datatype(nullptr),
      m_operation(nullptr),
      m_entityAttribute(nullptr),
      m_template(nullptr)
{
    ui->setupUi(this);
}

UMLDatatypeWidget::~UMLDatatypeWidget()
{
}

void UMLDatatypeWidget::setAttribute(UMLAttribute *attribute)
{
    m_attribute = attribute;
    m_parent = dynamic_cast<UMLClassifier*>(m_attribute->parent()->parent());
    insertTypesSortedParameter(m_attribute->getTypeName());
}

void UMLDatatypeWidget::setClassifierItem(UMLClassifierListItem *datatype)
{
    m_datatype = datatype;
    m_parent = dynamic_cast<UMLClassifier *>(m_datatype->parent());
    insertTypesSortedAttribute(m_datatype->getTypeName());

}

void UMLDatatypeWidget::setEntityAttribute(UMLEntityAttribute *entityAttribute)
{
    m_entityAttribute = entityAttribute;
    m_parent = 0;
    insertTypesSortedEntityAttribute(m_entityAttribute->getTypeName());
}

void UMLDatatypeWidget::setOPeration(UMLOperation *operation)
{
    m_operation = operation;
    m_parent = dynamic_cast<UMLClassifier*>(m_operation->parent());
    insertTypesSortedOperation(m_operation->getTypeName());

}

void UMLDatatypeWidget::setTemplate(UMLTemplate *_template)
{
    m_template = _template;
    m_parent = 0;
    insertTypesSortedTemplate(m_template->getTypeName());

}

bool UMLDatatypeWidget::apply()
{
    if (m_datatype)
        return applyAttribute();
    else if (m_entityAttribute)
        return applyEntityAttribute();
    else if (m_operation)
        return applyOperation();
    else if (m_attribute)
        return applyParameter();
    else if (m_template)
        return applyTemplate();
    return false;
}

bool UMLDatatypeWidget::applyAttribute()
{
    QString typeName = ui->typesCB->currentText();
    Uml::TypeQualifiers::Enum typeQualifier = m_datatype->qualifier();
    Uml::TypeModifiers::Enum typeModifierEnum = m_datatype->modifier();
    //Need to find a better way to do that
    QString m = Uml::TypeModifiers::toString(typeModifierEnum);
    QString q = Uml::TypeQualifiers::toString(typeQualifier);
    QString finalString = q + typeName + m;

    UMLTemplate *tmplParam = m_parent->findTemplate(finalString);
    if (tmplParam) {
        m_datatype->setType(tmplParam);
        return true;
    }
    UMLDoc * pDoc = UMLApp::app()->document();

    UMLObject *obj = nullptr;
    if (!typeName.isEmpty()) {
        obj = pDoc->findUMLObject(finalString);
    }

    UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(obj);
    if (classifier == NULL) {
        Uml::ProgrammingLanguage::Enum pl = UMLApp::app()->activeLanguage();
        // Import_Utils does not handle creating a new object with empty name
        // string well. Use Object_Factory in those cases.
        if (!typeName.isEmpty() && ((pl == Uml::ProgrammingLanguage::Cpp) || (pl == Uml::ProgrammingLanguage::Java)) )
        {
            // Import_Utils::createUMLObject works better for C++ namespace
            // and java package than Object_Factory::createUMLObject
            Import_Utils::setRelatedClassifier(m_parent);
            obj = Import_Utils::createUMLObject(UMLObject::ot_UMLObject, finalString);
            Import_Utils::setRelatedClassifier(NULL);
        } else {
            // If it's obviously a pointer type (C++) then create a datatype.
            // Else we don't know what it is so as a compromise create a class.
            bool contains = false;
            if(finalString.contains(QChar::fromLatin1('*')) || finalString.contains(QChar::fromLatin1('&')))
                contains = true;
            UMLObject::ObjectType ot =
                (contains ? UMLObject::ot_Datatype : UMLObject::ot_Class);
            obj = Object_Factory::createUMLObject(ot, finalString);
        }
        if (obj == NULL)
            return false;
        classifier = static_cast<UMLClassifier*>(obj);
    }
    m_datatype->setType(classifier);
    return true;
}

bool UMLDatatypeWidget::applyEntityAttribute()
{
    QString typeName = ui->typesCB->currentText();
    UMLDoc *pDoc = UMLApp::app()->document();
    UMLClassifierList dataTypes = pDoc->datatypes();
    foreach (UMLClassifier* dat, dataTypes) {
        if (typeName == dat->name()) {
            m_entityAttribute->setType(dat);
            return true;
        }
    }
    UMLObject *obj = pDoc->findUMLObject(typeName);
    UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(obj);
    if (classifier == NULL) {
        // If it's obviously a pointer type (C++) then create a datatype.
        // Else we don't know what it is so as a compromise create a class.
        UMLObject::ObjectType ot =
            (typeName.contains(QChar::fromLatin1('*')) ? UMLObject::ot_Datatype
                                                      : UMLObject::ot_Class);
        obj = Object_Factory::createUMLObject(ot, typeName);
        if (obj == NULL)
            return false;
        classifier = static_cast<UMLClassifier*>(obj);
    }
    m_entityAttribute->setType(classifier);
    return true;
}

bool UMLDatatypeWidget::applyOperation()
{
    QString typeName = ui->typesCB->currentText();
    UMLTemplate *tmplParam = 0;
    if (m_parent) {
        tmplParam = m_parent->findTemplate(typeName);
    }
    if (tmplParam)
        m_operation->setType(tmplParam);
    else
        m_operation->setTypeName(typeName);
    return true;
}

bool UMLDatatypeWidget::applyParameter()
{
    // set the type name
    QString typeName = ui->typesCB->currentText();
    if (m_parent == NULL) {
        uError() << "grandparent of " << m_attribute->name() << " is not a UMLClassifier";
    } else {
        UMLTemplate *tmplParam = m_parent->findTemplate(typeName);
        if (tmplParam) {
            m_attribute->setType(tmplParam);
            return true;
        }
    }
    UMLDoc * uDoc = UMLApp::app()->document();
    UMLClassifierList namesList(uDoc->concepts());
    bool matchFound = false;

    foreach (UMLClassifier* obj, namesList) {
        if (obj->fullyQualifiedName() == typeName) {
            m_attribute->setType(obj);
            matchFound = true;
            break;
        }
    }
    if (!matchFound) {
        // Nothing found: Create a new type on the fly.
        // @todo There should be an extra dialog to decide whether to
        // create a datatype or a class. For now, we create a class.
        uDebug() << typeName << " not found."
            << " Creating a new class for the type.";
        UMLObject *newObj = Object_Factory::createUMLObject(UMLObject::ot_Class, typeName);
        m_attribute->setType(newObj);
    }
    return true;
}

bool UMLDatatypeWidget::applyTemplate()
{
    QString typeName = ui->typesCB->currentText();
    UMLDoc *pDoc = UMLApp::app()->document();
    UMLClassifierList namesList(pDoc->concepts());
    foreach (UMLClassifier* obj, namesList) {
        if (typeName == obj->name()) {
            m_template->setType(obj);
            return true;
        }
    }

    UMLObject *obj = nullptr;
    if (!typeName.isEmpty()) {
        obj = pDoc->findUMLObject(typeName);
    }

    UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(obj);
    if (classifier == NULL) {
        Uml::ProgrammingLanguage::Enum pl = UMLApp::app()->activeLanguage();
        // Import_Utils does not handle creating a new object with empty name
        // string well. Use Object_Factory in those cases.
        if (!typeName.isEmpty() && ((pl == Uml::ProgrammingLanguage::Cpp) || (pl == Uml::ProgrammingLanguage::Java)) )
        {
            // Import_Utils::createUMLObject works better for C++ namespace
            // and java package than Object_Factory::createUMLObject
            Import_Utils::setRelatedClassifier(m_parent);
            obj = Import_Utils::createUMLObject(UMLObject::ot_UMLObject, typeName);
            Import_Utils::setRelatedClassifier(NULL);
        } else {
            // If it's obviously a pointer type (C++) then create a datatype.
            // Else we don't know what it is so as a compromise create a class.
            bool contains = false;
            if(typeName.contains(QChar::fromLatin1('*')) || typeName.contains(QChar::fromLatin1('&')))
                contains = true;
            UMLObject::ObjectType ot =
                (contains ? UMLObject::ot_Datatype : UMLObject::ot_Class);
            obj = Object_Factory::createUMLObject(ot, typeName);
        }
        if (obj == NULL)
            return false;
        classifier = static_cast<UMLClassifier*>(obj);
    }
    m_template->setType(classifier);
    return true;
}

/**
 * Initialize types combo box from a list of types and a selected type.
 * @param types list of types to add to combo box
 * @param type selected type
 */
void UMLDatatypeWidget::initTypesBox(QStringList &types, const QString& type)
{
    if (!types.contains(type)) {
        types << type;
    }
    types.sort();

    ui->typesCB->clear();
    ui->typesCB->insertItems(0, types);

    int currentIndex = ui->typesCB->findText(type);
    if (currentIndex > -1) {
        ui->typesCB->setCurrentIndex(currentIndex);
    }
}

/**
 * Add classes and interfaces from document instance to the given string list.
 * @param types list to store the classes and interfaces
 */
void UMLDatatypeWidget::insertTypesFromConcepts(QStringList& types, bool fullName)
{
    UMLDoc * uDoc = UMLApp::app()->document();
    UMLClassifierList namesList(uDoc->concepts());
    foreach (UMLClassifier* obj, namesList) {
         types << (fullName ? obj->fullyQualifiedName() : obj->name());
    }
}

/**
 * Add datatypes from document instance to the given string list.
 * @param types list to store the datatypes
 */
void UMLDatatypeWidget::insertTypesFromDatatypes(QStringList& types)
{
    // add the data types
    UMLDoc * pDoc = UMLApp::app()->document();
    UMLClassifierList dataTypes = pDoc->datatypes();
    if (dataTypes.count() == 0) {
        // Switch to SQL as the active language if no datatypes are set.
        UMLApp::app()->setActiveLanguage(Uml::ProgrammingLanguage::SQL);
        pDoc->addDefaultDatatypes();
        qApp->processEvents();
        dataTypes = pDoc->datatypes();
    }
    foreach (UMLClassifier* dat, dataTypes) {
        types << dat->name();
    }
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 * The combobox is cleared and all types together with the optional new one
 * sorted and then added again.
 * @param type   a new type to add
 */
void UMLDatatypeWidget::insertTypesSortedAttribute(const QString& type)
{
    QStringList types;
    insertTypesFromConcepts(types);
    initTypesBox(types, type);
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 */
void UMLDatatypeWidget::insertTypesSortedEntityAttribute(const QString& type)
{
    QStringList types;
    insertTypesFromDatatypes(types);
    initTypesBox(types, type);
}

/**
 * Inserts @p type into the type-combobox.
 * The combobox is cleared and all types together with the optional new one
 * sorted and then added again.
 * @param type   a new type to add and selected
 */
void UMLDatatypeWidget::insertTypesSortedOperation(const QString& type)
{
    QStringList types;
    // Add "void". We use this for denoting "no return type" independent
    // of the programming language.
    // For example, the Ada generator would interpret the return type
    // "void" as an instruction to generate a procedure instead of a
    // function.
    types << QLatin1String("void");
    // add template parameters
    UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(m_parent);
    if (classifier) {
        UMLClassifierListItemList tmplParams(classifier->getFilteredList(UMLOperation::ot_Template));
        foreach (UMLClassifierListItem* li, tmplParams) {
            types << li->name();
        }
    }
    insertTypesFromConcepts(types);
    initTypesBox(types, type);
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 * The combobox is cleared and all types together with the optional new one
 * sorted and then added again.
 * @param type   a new type to add and selected
 */
void UMLDatatypeWidget::insertTypesSortedParameter(const QString& type)
{
    QStringList types;
    // add template parameters
    UMLClassifier *pConcept = dynamic_cast<UMLClassifier*>(m_parent);
    if (pConcept == NULL) {
        uError() << "ParameterPropertiesDialog: grandparent of " << m_attribute->name()
                 << " is not a UMLClassifier";
    } else {
        UMLTemplateList tmplParams(pConcept->getTemplateList());
        foreach(UMLTemplate* t, tmplParams) {
            types << t->name();
        }
    }
    insertTypesFromConcepts(types);
    initTypesBox(types, type);
}

/**
 * Inserts @p type into the type-combobox.
 * The combobox is cleared and all types together with the optional new one
 * sorted and then added again.
 * @param type   a new type to add and selected
 */
void UMLDatatypeWidget::insertTypesSortedTemplate(const QString& type)
{
    QStringList types;
    // "class" is the nominal type of template parameter
    types << QLatin1String("class");
    insertTypesFromConcepts(types, false);
    initTypesBox(types, type);
}

/**
 * Add this widget to a given grid layout. Umbrello dialogs places labels in column 0
 * and the editable field in column 1.
 * @param layout The layout to which the widget should be added
 * @param row The row in the grid layout where the widget should be placed
 */
void UMLDatatypeWidget::addToLayout(QGridLayout *layout, int row, int startColumn)
{
    layout->addWidget(this, row, startColumn);
}
