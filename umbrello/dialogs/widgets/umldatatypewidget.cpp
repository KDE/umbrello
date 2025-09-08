/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umldatatypewidget.h"

#include "umlclassifierlistitem.h"
#include "umlclassifier.h"
#include "debug_utils.h"
#include "umlentityattribute.h"
#include "import_utils.h"
#include "model_utils.h"
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

DEBUG_REGISTER(UMLDatatypeWidget)

UMLDatatypeWidget::UMLDatatypeWidget(UMLAttribute *attribute, QWidget *parent)
 :  ComboBoxWidgetBase(i18n("&Type:"), QString(), parent),
    m_attribute(attribute),
    m_datatype(nullptr),
    m_entityAttribute(nullptr),
    m_operation(nullptr),
    m_template(nullptr)
{
    init();
    m_parent = m_attribute->umlParent()->umlParent()->asUMLClassifier();
    insertTypesSortedParameter(m_attribute->getTypeName());
}

UMLDatatypeWidget::UMLDatatypeWidget(UMLClassifierListItem *datatype, QWidget *parent)
 :  ComboBoxWidgetBase(i18n("&Type:"), QString(), parent),
    m_attribute(nullptr),
    m_datatype(datatype),
    m_entityAttribute(nullptr),
    m_operation(nullptr),
    m_template(nullptr)
{
    init();
    m_parent = m_datatype->umlParent()->asUMLClassifier();
    insertTypesSortedAttribute(m_datatype->getTypeName());
}

UMLDatatypeWidget::UMLDatatypeWidget(UMLEntityAttribute *entityAttribute, QWidget *parent)
  :  ComboBoxWidgetBase(i18n("&Type:"), QString(), parent),
     m_attribute(nullptr),
     m_datatype(nullptr),
     m_entityAttribute(entityAttribute),
     m_operation(nullptr),
     m_template(nullptr)
{
    init();
    m_parent = nullptr;
    insertTypesSortedEntityAttribute(m_entityAttribute->getTypeName());
}

UMLDatatypeWidget::UMLDatatypeWidget(UMLOperation *operation, QWidget *parent)
 :  ComboBoxWidgetBase(i18n("&Type:"), QString(), parent),
    m_attribute(nullptr),
    m_datatype(nullptr),
    m_entityAttribute(nullptr),
    m_operation(operation),
    m_template(nullptr)
{
    init();
    m_parent = m_operation->umlParent()->asUMLClassifier();
    insertTypesSortedOperation(m_operation->getTypeName());
}

UMLDatatypeWidget::UMLDatatypeWidget(UMLTemplate *_template, QWidget *parent)
 :  ComboBoxWidgetBase(i18n("&Type:"), QString(), parent),
    m_attribute(nullptr),
    m_datatype(nullptr),
    m_entityAttribute(nullptr),
    m_operation(nullptr),
    m_template(_template)
{
    init();
    m_parent = nullptr;
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
    QString typeName = Model_Utils::normalize(m_editField->currentText());
    UMLTemplate *tmplParam = m_parent->findTemplate(typeName);
    if (tmplParam) {
        m_datatype->setType(tmplParam);
        return true;
    }
    UMLDoc * pDoc = UMLApp::app()->document();

    UMLObject  *obj = nullptr;
    if (!typeName.isEmpty()) {
        obj = pDoc->findUMLObject(typeName);
    }

    UMLClassifier *classifier = obj ? obj->asUMLClassifier() : nullptr;
    if (classifier == nullptr) {
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
            Import_Utils::setRelatedClassifier(nullptr);
        } else {
            // If it's obviously a pointer type (C++) then create a datatype.
            // Else we don't know what it is so as a compromise create a class.
            UMLObject::ObjectType ot =
                (typeName.contains(QChar::fromLatin1('*')) ? UMLObject::ot_Datatype
                                                          : UMLObject::ot_Class);
            obj = Object_Factory::createUMLObject(ot, typeName);
        }
        if (obj == nullptr)
            return false;
        classifier = obj->asUMLClassifier();
    }
    m_datatype->setType(classifier);
    return true;
}

bool UMLDatatypeWidget::applyEntityAttribute()
{
    QString typeName = Model_Utils::normalize(m_editField->currentText());
    UMLDoc *pDoc = UMLApp::app()->document();
    UMLClassifierList dataTypes = pDoc->datatypes();
    for(UMLClassifier* dat : dataTypes) {
        if (typeName == dat->name()) {
            m_entityAttribute->setType(dat);
            return true;
        }
    }
    UMLObject *obj = pDoc->findUMLObject(typeName);
    UMLClassifier *classifier = obj->asUMLClassifier();
    if (classifier == nullptr) {
        // If it's obviously a pointer type (C++) then create a datatype.
        // Else we don't know what it is so as a compromise create a class.
        UMLObject::ObjectType ot =
            (typeName.contains(QChar::fromLatin1('*')) ? UMLObject::ot_Datatype
                                                      : UMLObject::ot_Class);
        obj = Object_Factory::createUMLObject(ot, typeName);
        if (obj == nullptr)
            return false;
        classifier = obj->asUMLClassifier();
    }
    m_entityAttribute->setType(classifier);
    return true;
}

bool UMLDatatypeWidget::applyOperation()
{
    QString typeName = Model_Utils::normalize(m_editField->currentText());
    UMLTemplate  *tmplParam = nullptr;
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
    QString typeName = Model_Utils::normalize(m_editField->currentText());
    if (m_parent == nullptr) {
        logError1("UMLDatatypeWidget::applyParameter: grandparent of %1 is not a UMLClassifier",
                  m_attribute->name());
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

    for(UMLClassifier *obj : namesList) {
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
        logDebug1("UMLDatatypeWidget::applyParameter: %1 not found. Creating a new class for the type.",
                  typeName);
        UMLObject *newObj = Object_Factory::createUMLObject(UMLObject::ot_Class, typeName);
        m_attribute->setType(newObj);
    }
    return true;
}

bool UMLDatatypeWidget::applyTemplate()
{
    QString typeName = Model_Utils::normalize(m_editField->currentText());
    UMLDoc *pDoc = UMLApp::app()->document();
    UMLClassifierList namesList(pDoc->concepts());
    for(UMLClassifier *obj : namesList) {
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
 * @param fullName if true then insert names including their package path
 */
void UMLDatatypeWidget::insertTypesFromConcepts(QStringList& types, bool fullName)
{
    UMLDoc * uDoc = UMLApp::app()->document();
    UMLClassifierList namesList(uDoc->concepts());
    for(UMLClassifier *obj : namesList) {
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
    for(UMLClassifier *dat : dataTypes) {
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
    types << QStringLiteral("void");
    // add template parameters
    const UMLClassifier *classifier = m_parent->asUMLClassifier();
    if (classifier) {
        UMLClassifierListItemList tmplParams(classifier->getFilteredList(UMLOperation::ot_Template));
        for(UMLClassifierListItem *li : tmplParams) {
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
    const UMLClassifier *pConcept = m_parent->asUMLClassifier();
    if (pConcept == nullptr) {
        logError1("UMLDatatypeWidget::insertTypesSortedParameter: grandparent of %1 "
                  " is not a UMLClassifier", m_attribute->name());
    } else {
        UMLTemplateList tmplParams(pConcept->getTemplateList());
        for(UMLTemplate *t : tmplParams) {
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
    types << QStringLiteral("class");
    insertTypesFromConcepts(types, false);
    initTypesBox(types, type);
}
