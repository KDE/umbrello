/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/
#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include "package.h"
#include "umlattributelist.h"
#include "umloperationlist.h"
#include "umlclassifierlistitemlist.h"
#include "classifierlistitem.h"
#include "umltemplatelist.h"
#include "model_utils.h"

// forward declarations
class UMLAssociation;
class IDChangeLog;
class UMLClassifierSet;

/**
 * This class defines the non-graphical information required for a
 * UML Classifier (ie a class or interface).
 * This class inherits from @ref UMLPackage which allows classifiers
 * to also act as namespaces, i.e. it allows classifiers to nest.
 *
 * NOTE: There is a unit test available for this class.
 *       Please, use and adapt it when necessary.
 *
 * @short Information for a non-graphical Concept/Class.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLClassifier : public UMLPackage
{
    Q_OBJECT
public:

    /**
     * Enumeration identifying the type of classifier.
     */
    enum ClassifierType { ALL = 0, CLASS, INTERFACE, DATATYPE };

    explicit UMLClassifier(const QString & name = QString(), Uml::ID::Type id = Uml::ID::None);
    virtual ~UMLClassifier();

    virtual void copyInto(UMLObject *lhs) const;

    UMLObject* clone() const;

    virtual void setNameCmd(const QString &strName);

    virtual UMLAttribute* createAttribute(const QString &name = QString(),
                                          UMLObject *type = 0,
                                          Uml::Visibility::Enum vis = Uml::Visibility::Private,
                                          const QString &init = QString());

    UMLAttribute* addAttribute(const QString &name, Uml::ID::Type id = Uml::ID::None);
    UMLAttribute* addAttribute(const QString &name, UMLObject *type, Uml::Visibility::Enum scope);

    bool addAttribute(UMLAttribute* att, IDChangeLog* log = 0,
                      int position = -1);

    int removeAttribute(UMLAttribute *att);

    UMLAttributeList getAttributeList() const;
    UMLAttributeList getAttributeList(Uml::Visibility::Enum scope) const;
    UMLAttributeList getAttributeListStatic(Uml::Visibility::Enum scope) const;

    UMLOperation* createOperation(const QString &name = QString(),
                                  bool *isExistingOp = 0,
                                  Model_Utils::NameAndType_List *params = 0);

    bool addOperation(UMLOperation* op, int position = -1);
    bool addOperation(UMLOperation* op, IDChangeLog* log);

    UMLOperation * checkOperationSignature(const QString& name,
                                           UMLAttributeList opParams,
                                           UMLOperation *exemptOp = 0);

    int removeOperation(UMLOperation *op);

    int operations();

    UMLOperationList getOpList(bool includeInherited = false, UMLClassifierSet *alreadyTraversed = 0);

    UMLObject* createTemplate(const QString& name = QString());

    UMLTemplate* addTemplate(const QString &name, Uml::ID::Type id = Uml::ID::None);

    bool addTemplate(UMLTemplate* newTemplate, IDChangeLog* log = 0);
    bool addTemplate(UMLTemplate* templt, int position);

    int removeTemplate(UMLTemplate* umltemplate);

    UMLTemplate *findTemplate(const QString& name);

    int templates();

    UMLTemplateList getTemplateList() const;

    int takeItem(UMLClassifierListItem* item);

    virtual UMLClassifierListItemList getFilteredList(UMLObject::ObjectType ot) const;

    virtual bool resolveRef();

    UMLOperationList findOperations(const QString &n);

    virtual UMLObject* findChildObjectById(Uml::ID::Type id, bool considerAncestors = false);

    UMLOperation* findOperation(const QString& name,
                                Model_Utils::NameAndType_List params);

    UMLClassifierList findSuperClassConcepts(ClassifierType type = ALL);

    UMLClassifierList findSubClassConcepts(ClassifierType type = ALL);

    virtual bool acceptAssociationType(Uml::AssociationType::Enum type);

    void setBaseType(UMLObject::ObjectType ot);

    bool isInterface() const;

    bool hasAbstractOps();
    bool hasAssociations();
    bool hasAttributes();
    bool hasStaticAttributes();
    bool hasMethods();
    bool hasAccessorMethods();
    bool hasOperationMethods();
    bool hasVectorFields();

    /**
     * utility functions to allow easy determination of what classifiers
     * are "owned" by the current one via named association type (e.g.
     * plain, aggregate or compositions).
     */
//    UMLClassifierList getPlainAssocChildClassifierList();
//    UMLClassifierList getAggregateChildClassifierList();
//    UMLClassifierList getCompositionChildClassifierList();

    virtual UMLClassifierListItem* makeChildObject(const QString& xmiTag);

    virtual UMLAssociationList  getUniAssociationToBeImplemented();

signals:

    void operationAdded(UMLClassifierListItem *);
    void operationRemoved(UMLClassifierListItem *);

    void templateAdded(UMLClassifierListItem*);
    void templateRemoved(UMLClassifierListItem*);

    // only applies when (m_Type == ot_Class)
    void attributeAdded(UMLClassifierListItem*);
    void attributeRemoved(UMLClassifierListItem*);

private:

    UMLAssociation *m_pClassAssoc;

    /**
     * Utility method called by "get*ChildClassfierList()" methods. It basically
     * finds all the classifiers named in each association in the given association list
     * which aren't the current one. Useful for finding which classifiers are "owned" by the
     * current one via declared associations such as in aggregations/compositions.
     */
//    UMLClassifierList findAssocClassifierObjsInRoles (UMLAssociationList * list);

protected:

    void saveToXMI1(QXmlStreamWriter& writer);

    virtual bool load1(QDomElement& element);

};

#endif // CLASSIFIER_H
