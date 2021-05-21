/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

#include <KComboBox>
#include <KLocalizedString>

#include <QApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

UMLDatatypeWidget::UMLDatatypeWidget(UMLAttribute *attribute, QWidget *parent)
 :  ComboBoxWidgetBase(i18n("&Type:"), QString(), parent),
    m_attribute(attribute),
    m_datatype(0),
    m_entityAttribute(0),
    m_operation(0),
    m_template(0)
{
    init();
    m_parent = m_attribute->umlParent()->umlParent()->asUMLClassifier();
    insertTypesSortedParameter(m_attribute->getTypeName());
}

UMLDatatypeWidget::UMLDatatypeWidget(UMLClassifierListItem *datatype, QWidget *parent)
 :  ComboBoxWidgetBase(i18n("&Type:"), QString(), parent),
    m_attribute(0),
    m_datatype(datatype),
    m_entityAttribute(0),
    m_operation(0),
    m_template(0)
{
    init();
    m_parent = m_datatype->umlParent()->asUMLClassifier();
    insertTypesSortedAttribute(m_datatype->getTypeName());
}

UMLDatatypeWidget::UMLDatatypeWidget(UMLEntityAttribute *entityAttribute, QWidget *parent)
  :  ComboBoxWidgetBase(i18n("&Type:"), QString(), parent),
     m_attribute(0),
     m_datatype(0),
     m_entityAttribute(entityAttribute),
     m_operation(0),
     m_template(0)
{
    init();
    m_parent = 0;
    insertTypesSortedEntityAttribute(m_entityAttribute->getTypeName());
}

UMLDatatypeWidget::UMLDatatypeWidget(UMLOperation *operation, QWidget *parent)
 :  ComboBoxWidgetBase(i18n("&Type:"), QString(), parent),
    m_attribute(0),
    m_datatype(0),
    m_entityAttribute(0),
    m_operation(operation),
    m_template(0)
{
    init();
    m_parent = m_operation->umlParent()->asUMLClassifier();
    insertTypesSortedOperation(m_operation->getTypeName());
}

UMLDatatypeWidget::UMLDatatypeWidget(UMLTemplate *_template, QWidget *parent)
 :  ComboBoxWidgetBase(i18n("&Type:"), QString(), parent),
    m_attribute(0),
    m_datatype(0),
    m_entityAttribute(0),
    m_operation(0),
    m_template(_template)
{
    init();
    m_parent = 0;
    insertTypesSortedTemplate(m_template->getTypeName());
}

void UMLDatatypeWidget::init()
{
    connect(m_editField, SIGNAL(editTextChanged(QString)), this, SIGNAL(editTextChanged(QString)));
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
    QString typeName = m_editField->currentText();
    UMLTemplate *tmplParam = m_parent->findTemplate(typeName);
    if (tmplParam) {
        m_datatype->setType(tmplParam);
        return true;
    }
    UMLDoc * pDoc = UMLApp::app()->document();

    UMLObject *obj = 0;
    if (!typeName.isEmpty()) {
        obj = pDoc->findUMLObject(typeName);
    }

    UMLClassifier *classifier = obj->asUMLClassifier();
    if (classifier == 0) {
        Uml::ProgrammingLanguage::Enum pl = UMLApp::app()->activeLanguage();
        // Import_Utils does not handle creating a new object with empty name
        // string well. Use Object_Factory in those cases.
        if (
            (!typeName.isEmpty()) &&
            ((pl == Uml::ProgrammingLanguage::Cpp) ||
                (pl == Uml::ProgrammingLanguage::Java))
        ) {
            // Import_Utils::createUMLObject works better for C++ namespace
            // and java package than Object_Factory::createUMLObject
            Import_Utils::setRelatedClassifier(m_parent);
            obj = Import_Utils::createUMLObject(UMLObject::ot_UMLObject, typeName);
            Import_Utils::setRelatedClassifier(0);
        } else {
            // If it's obviously a pointer type (C++) then create a datatype.
            // Else we don't know what it is so as a compromise create a class.
            UMLObject::ObjectType ot =
                (typeName.contains(QChar::fromLatin1('*')) ? UMLObject::ot_Datatype
                                                          : UMLObject::ot_Class);
            obj = Object_Factory::createUMLObject(ot, typeName);
        }
        if (obj == 0)
            return false;
        classifier = obj->asUMLClassifier();
    }
    m_datatype->setType(classifier);
    return true;
}

bool UMLDatatypeWidget::applyEntityAttribute()
{
    QString typeName = m_editField->currentText();
    UMLDoc *pDoc = UMLApp::app()->document();
    UMLClassifierList dataTypes = pDoc->datatypes();
    foreach (UMLClassifier* dat, dataTypes) {
        if (typeName == dat->name()) {
            m_entityAttribute->setType(dat);
            return true;
        }
    }
    UMLObject *obj = pDoc->findUMLObject(typeName);
    UMLClassifier *classifier = obj->asUMLClassifier();
    if (classifier == 0) {
        // If it's obviously a pointer type (C++) then create a datatype.
        // Else we don't know what it is so as a compromise create a class.
        UMLObject::ObjectType ot =
            (typeName.contains(QChar::fromLatin1('*')) ? UMLObject::ot_Datatype
                                                      : UMLObject::ot_Class);
        obj = Object_Factory::createUMLObject(ot, typeName);
        if (obj == 0)
            return false;
        classifier = obj->asUMLClassifier();
    }
    m_entityAttribute->setType(classifier);
    return true;
}

bool UMLDatatypeWidget::applyOperation()
{
    QString typeName = m_editField->currentText();
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
    QString typeName = m_editField->currentText();
    if (m_parent == 0) {
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
    QString typeName = m_editField->currentText();
    UMLDoc *pDoc = UMLApp::app()->document();
    UMLClassifierList namesList(pDoc->concepts());
    foreach (UMLClassifier* obj, namesList) {
        if (typeName == obj->name()) {
            m_template->setType(obj);
        }
    }
    if (namesList.isEmpty()) { // not found.
        // FIXME: This implementation is not good yet.
        m_template->setTypeName(typeName);
    }
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

    m_editField->clear();
    m_editField->insertItems(-1, types);

    int currentIndex = m_editField->findText(type);
    if (currentIndex > -1) {
        m_editField->setCurrentIndex(currentIndex);
    }
    m_editField->completionObject()->addItem(type);
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
    UMLClassifier *classifier = m_parent->asUMLClassifier();
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
    UMLClassifier *pConcept = m_parent->asUMLClassifier();
    if (pConcept == 0) {
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
