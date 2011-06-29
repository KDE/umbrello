/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
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
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLClassifier : public UMLPackage
{
    Q_OBJECT
public:

    /**
     * Enumeration identifying the type of classifier.
     */
    enum ClassifierType { ALL = 0, CLASS, INTERFACE, DATATYPE };

    explicit UMLClassifier(const QString & name = QString(), Uml::IDType id = Uml::id_None);
    virtual ~UMLClassifier();

    bool operator==(const UMLClassifier & rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    UMLObject* clone() const;

    virtual UMLAttribute* createAttribute(const QString &name = QString(),
                                          UMLObject *type = 0,
                                          Uml::Visibility vis = Uml::Visibility::Private,
                                          const QString &init = QString());

    UMLAttribute* addAttribute(const QString &name, Uml::IDType id = Uml::id_None);
    UMLAttribute* addAttribute(const QString &name, UMLObject *type, Uml::Visibility scope);

    bool addAttribute(UMLAttribute* att, IDChangeLog* log = 0,
                      int position = -1);

    int removeAttribute(UMLAttribute *att);

    int attributes() ;

    UMLAttributeList getAttributeList() const;
    UMLAttributeList getAttributeList(Uml::Visibility scope) const;

    UMLAttributeList getAttributeListStatic(Uml::Visibility scope) const;

    UMLOperation* createOperation( const QString &name = QString(),
                                   bool *isExistingOp = NULL,
                                   Model_Utils::NameAndType_List *params = NULL);

    bool addOperation(UMLOperation* op, int position = -1);
    bool addOperation(UMLOperation* op, IDChangeLog* log);

    UMLOperation * checkOperationSignature( const QString& name,
                                            UMLAttributeList opParams,
                                            UMLOperation *exemptOp = NULL);

    int removeOperation(UMLOperation *op);

    int operations() ;

    UMLOperationList getOpList(bool includeInherited = false);

    UMLObject* createTemplate(const QString& name = QString());

    UMLTemplate* addTemplate(const QString &name, Uml::IDType id = Uml::id_None);

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

    virtual UMLObject* findChildObjectById(Uml::IDType id, bool considerAncestors = false);

    UMLOperation* findOperation(const QString& name,
                                Model_Utils::NameAndType_List params);

    UMLClassifierList findSuperClassConcepts(ClassifierType type = ALL);

    UMLClassifierList findSubClassConcepts(ClassifierType type = ALL);

    virtual bool acceptAssociationType(Uml::AssociationType);

    void setClassAssoc(UMLAssociation *assoc);
    UMLAssociation *getClassAssoc() const;

    void setBaseType(UMLObject::ObjectType ot);

    bool isInterface() const;

    bool isDatatype() const;

    void setOriginType(UMLClassifier *origType);
    UMLClassifier * originType() const;

    void setIsReference(bool isRef = true);
    bool isReference() const;

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

    bool m_isRef;

    /**
     * Utility method called by "get*ChildClassfierList()" methods. It basically
     * finds all the classifiers named in each association in the given association list
     * which aren't the current one. Useful for finding which classifiers are "owned" by the
     * current one via declared associations such as in aggregations/compositions.
     */
//    UMLClassifierList findAssocClassifierObjsInRoles (UMLAssociationList * list);

protected:

    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    virtual bool load(QDomElement& element);

};

#endif // CLASSIFIER_H
