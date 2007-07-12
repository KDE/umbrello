/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CLASSIFIER__H
#define CLASSIFIER__H

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
 * @short Information for a non-graphical Concept/Class.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLClassifier : public UMLPackage {
    Q_OBJECT
public:

    /**
     * Enumeration identifying the type of classifier.
     */
    enum ClassifierType { ALL = 0, CLASS, INTERFACE, DATATYPE };


    /**
     * Sets up a Concept.
     *
     * @param name              The name of the Concept.
     * @param id                The unique id of the Concept.
     */
    explicit UMLClassifier(const QString & name = "", Uml::IDType id = Uml::id_None);

    /**
     * Standard deconstructor.
     */
    virtual ~UMLClassifier();

    /**
     * Overloaded '==' operator.
     */
    bool operator==( UMLClassifier & rhs );

    /**
     * Copy the internal presentation of this object into the new
     * object.
     */
    virtual void copyInto(UMLClassifier *rhs) const;

    /**
     * Make a clone of this object.
     */
    UMLObject* clone() const;

    /**
     * Creates an attribute for the class.
     *
     * @param name  An optional name, used by when creating through UMLListView
     * @param type  An optional type, used by when creating through UMLListView
     * @param vis   An optional visibility, used by when creating through UMLListView
     * @param init  An optional initial value, used by when creating through UMLListView
     * @return  The UMLAttribute created
     */
    virtual UMLAttribute* createAttribute(const QString &name = QString::null,
                                          UMLObject *type = 0,
                                          Uml::Visibility vis = Uml::Visibility::Private,
                                          const QString &init = QString::null);

    /**
     * Adds an attribute to the class.
     * If an attribute of the given name already exists, then
     * returns the existing attribute instead of creating a new one.
     *
     * @param name              The name of the attribute.
     * @param id                The id of the attribute (optional.)
     *                      If not given, and the attribute name
     *                      does not already exist, then the method
     *                      will internally assign a new ID.
     * @return  Pointer to the UMLAttribute created or found.
     */
    UMLAttribute* addAttribute(const QString &name, Uml::IDType id = Uml::id_None);

    UMLAttribute* addAttribute(const QString &name, UMLObject *type, Uml::Visibility scope);

    /**
     * Adds an already created attribute.
     * The attribute object must not belong to any other concept.
     *
     * @param Att               Pointer to the UMLAttribute.
     * @param Log               Pointer to the IDChangeLog (optional.)
     * @param position  Position index for the insertion (optional.)
     *                  If the position is omitted, or if it is
     *                  negative or too large, the attribute is added
     *                  to the end of the list.
     * @return  True if the attribute was successfully added.
     */
    bool addAttribute(UMLAttribute* Att, IDChangeLog* Log = 0,
                      int position = -1);

    /**
     * Removes an attribute from the class.
     *
     * @param a         The attribute to remove.
     * @return  Count of the remaining attributes after removal.
     *          Returns -1 if the given attribute was not found.
     */
    int removeAttribute(UMLAttribute *a);

    /**
     * Returns the number of attributes for the class.
     *
     * @return  The number of attributes for the class.
     */
    int attributes() ;

    /**
     * Returns the attributes.
     * Same as UMLClassifier::getFilteredList(ot_Attribute) but
     * return type is a true UMLAttributeList.
     *
     * @return  List of true attributes for the class.
     */
    UMLAttributeList getAttributeList() const;

    /**
     * Creates an operation in the current document.
     * The new operation is initialized with name, id, etc.
     * If a method with the given profile already exists in the classifier,
     * no new method is created and the existing operation is returned.
     * If no name is provided, or if the params are NULL, an Operation
     * Dialog is shown to ask the user for a name and parameters.
     * The operation's signature is checked for validity within the parent
     * classifier.
     *
     * @param name              The operation name (will be chosen internally if
     *                  none given.)
     * @param isExistingOp      Optional pointer to bool. If supplied, the bool is
     *                  set to true if an existing operation is returned.
     * @param params    Optional list of parameter names and types.
     *                  If supplied, new operation parameters are
     *                  constructed using this list.
     * @return The new operation, or NULL if the operation could not be
     *         created because for example, the user canceled the dialog
     *         or no appropriate name can be found.
    */
    UMLOperation* createOperation( const QString &name = QString::null,
                                   bool *isExistingOp = NULL,
                                   Model_Utils::NameAndType_List *params = NULL);

    /**
     * Adds an operation to the classifier, at the given position.
     * If position is negative or too large, the attribute is added
     * to the end of the list.
     * The Classifier first checks and only adds the Operation if the
     * signature does not conflict with exising operations
     *
     * @param Op                Pointer to the UMLOperation to add.
     * @param position  Index at which to insert into the list.
     * @return true if the Operation could be added to the Classifier.
     */
    bool addOperation(UMLOperation* Op, int position = -1);

    /**
     * Appends an operation to the classifier.
     * @see bool addOperation(UMLOperation* Op, int position = -1)
     * This function is mainly intended for the clipboard.
     *
     * @param Op                Pointer to the UMLOperation to add.
     * @param Log               Pointer to the IDChangeLog.
     * @return  True if the operation was added successfully.
     */
    bool addOperation(UMLOperation* Op, IDChangeLog* Log);

    /**
     * Checks whether an operation is valid based on its signature -
     * An operation is "valid" if the operation's name and parameter list
     * are unique in the classifier.
     *
     * @param name              Name of the operation to check.
     * @param opParams  The operation's argument list.
     * @param exemptOp  Pointer to the exempt method (optional.)
     * @return  NULL if the signature is valid (ok), else return a pointer
     *          to the existing UMLOperation that causes the conflict.
     */
    UMLOperation * checkOperationSignature( const QString& name,
                                            UMLAttributeList opParams,
                                            UMLOperation *exemptOp = NULL);

    /**
     * Remove an operation from the Classifier.
     * The operation is not deleted so the caller is responsible for what
     * happens to it after this.
     *
     * @param op        The operation to remove.
     * @return  Count of the remaining operations after removal, or
     *          -1 if the given operation was not found.
     */
    int removeOperation(UMLOperation *op);

    /**
     * counts the number of operations in the Classifier.
     *
     * @return  The number of operations for the Classifier.
     */
    int operations() ;

    /**
     * Return a list of operations for the Classifier.
     * @param includeInherited Includes operations from superclasses.
     *
     * @return  The list of operations for the Classifier.
     */
    UMLOperationList getOpList(bool includeInherited = false);

    /**
     * Creates a template for the concept.
     *
     * @return  The UMLTemplate created
     */
    UMLObject* createTemplate(const QString& name = QString::null);

    /**
     * Adds a template to the class if it is not there yet.
     *
     * @param name              The name of the template.
     * @param id                The id of the template.
     * @return  Pointer to the UMLTemplate object created.
     */
    UMLTemplate* addTemplate(const QString &name, Uml::IDType id = Uml::id_None);

    /**
     * Adds an already created template.
     * The template object must not belong to any other concept.
     *
     * @param newTemplate       Pointer to the UMLTemplate object to add.
     * @param log               Pointer to the IDChangeLog.
     * @return  True if the template was successfully added.
     */
    bool addTemplate(UMLTemplate* newTemplate, IDChangeLog* log = 0);

    /**
     * Adds an template to the class.
     * The template object must not belong to any other class.
     *
     * @param Template  Pointer to the UMLTemplate to add.
     * @param position  The position of the template in the list.
     *                  A value of -1 will add the template at the end.
     * @return  True if the template was successfully added.
     */
    //TODO: if the param IDChangeLog from the method above is not being used,
    // give position a default value of -1 and the method can replace the above one
    bool addTemplate(UMLTemplate* Template, int position);

    /**
     * Removes a template from the class.
     *
     * @param umltemplate  The template to remove.
     * @return  Count of the remaining templates after removal.
     *          Returns -1 if the given template was not found.
     */
    int removeTemplate(UMLTemplate* umltemplate);

    /**
     * Seeks the template parameter of the given name.
     */
    UMLTemplate *findTemplate(const QString& name);

    /**
     * Returns the number of templates for the class.
     *
     * @return  The number of templates for the class.
     */
    int templates();

    /**
     * Returns the templates.
     * Same as UMLClassifier::getFilteredList(ot_Template) but
     * return type is a true UMLTemplateList.
     *
     * @return  Pointer to the list of true templates for the class.
     */
    UMLTemplateList getTemplateList() const;

    /**
     * Take and return a subordinate item from this classifier.
     * Ownership of the item is passed to the caller.
     *
     * @param item    Subordinate item to take.
     * @return        Index in m_List of the item taken.
     *                Return -1 if the item is not found in m_List.
     */
    int takeItem(UMLClassifierListItem* item);

    /**
     * Returns the entries in m_List that are of the requested type.
     * If the requested type is Uml::ot_UMLObject then all entries
     * are returned.
     *
     * @return  The list of true operations for the Concept.
     */
    virtual UMLClassifierListItemList getFilteredList(Uml::Object_Type ot) const;

    /**
     * Needs to be called after all UML objects are loaded from file.
     * Calls the parent resolveRef(), and calls resolveRef() on all
     * UMLClassifierListItems.
     * Overrides the method from UMLObject.
     *
     * @return  true for success.
     */
    virtual bool resolveRef();

    /**
     * Find a list of operations with the given name.
     *
     * @param n         The name of the operation to find.
     * @return  The list of objects found; will be empty if none found.
     */
    UMLOperationList findOperations(const QString &n);

    /**
     * Find an attribute, operation, association or template.
     *
     * @param id                The id of the object to find.
     *
     * @return  The object found.  Will return 0 if none found.
     */
    virtual UMLObject* findChildObjectById(Uml::IDType id, bool considerAncestors = false);

    /**
     * Find an operation of the given name and parameter signature.
     *
     * @param name              The name of the operation to find.
     * @param params    The parameter descriptors of the operation to find.
     *
     * @return  The operation found.  Will return 0 if none found.
     */
    UMLOperation* findOperation(const QString& name,
                                Model_Utils::NameAndType_List params);

    /**
     * Returns a list of concepts which this concept inherits from.
     *
     * @param type              The ClassifierType to seek.
     * @return  List of UMLClassifiers we inherit from.
     */
    UMLClassifierList findSuperClassConcepts(ClassifierType type = ALL);

    /**
     * Returns a list of concepts which inherit from this concept.
     *
     * @param type              The ClassifierType to seek.
     * @return  List of UMLClassifiers that inherit from us.
     */
    UMLClassifierList findSubClassConcepts(ClassifierType type = ALL);

    /** reimplemented from UMLObject */
    virtual bool acceptAssociationType(Uml::Association_Type);

    /**
     * Sets the UMLAssociation for which this class shall act as an
     * association class.
     */
    void setClassAssoc(UMLAssociation *assoc);

    /**
     * Returns the UMLAssociation for which this class acts as an
     * association class. Returns NULL if this class does not act
     * as an association class.
     */
    UMLAssociation *getClassAssoc() const;

    /**
     * Reimplementation of method from class UMLObject for controlling the
     * exact type of this classifier: class, interface, or datatype.
     */
    void setBaseType(Uml::Object_Type ot);

    /**
     * Returns true if this classifier represents an interface.
     */
    bool isInterface() const;

    /**
     * Returns true if this classifier represents a datatype.
     */
    bool isDatatype() const;

    /**
     * Set the origin type (in case of e.g. typedef)
     */
    void setOriginType(UMLClassifier *origType);

    /**
     * Get the origin type (in case of e.g. typedef)
     */
    UMLClassifier * originType() const;

    /**
     * Set the m_isRef flag (true when dealing with a pointer type)
     */
    void setIsReference(bool isRef = true);

    /**
     * Get the m_isRef flag.
     */
    bool isReference() const;

    /**
     * Return true if this classifier has abstract operations.
     */
    bool hasAbstractOps ();

    /**
     * Create a new ClassifierListObject (attribute, operation, template)
     * according to the given XMI tag.
     * Returns NULL if the string given does not contain one of the tags
     * <UML:Attribute>, <UML:Operation>, or <UML:TemplateParameter>.
     * Used by the clipboard for paste operation.
     */
    UMLClassifierListItem* makeChildObject(const QString& xmiTag);

    /**
     * Return the list of unidirectional association that should show up in the code
     */
    virtual UMLAssociationList  getUniAssociationToBeImplemented();

signals:
    /** Signals that a new UMLOperation has been added to the classifer.
     */
    void operationAdded(UMLClassifierListItem *);

    /** Signals that a UMLOperation has been removed from the classifer.
     */
    void operationRemoved(UMLClassifierListItem *);

    void templateAdded(UMLClassifierListItem*);
    void templateRemoved(UMLClassifierListItem*);

    // only applies when (m_Type == ot_Class)
    void attributeAdded(UMLClassifierListItem*);
    void attributeRemoved(UMLClassifierListItem*);

private:

    /**
     * Initializes key variables of the class.
     */
    void init();

    UMLAssociation *m_pClassAssoc;

    bool m_isRef;

protected:

    /**
     * Auxiliary to saveToXMI of inheriting classes:
     * Saves template parameters to the given QDomElement.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    /**
     * Auxiliary to loadFromXMI:
     * The loading of operations is implemented here.
     * Calls loadSpecialized() for any other tag.
     * Child classes can override the loadSpecialized method
     * to load its additional tags.
     */
    virtual bool load(QDomElement& element);

};

#endif // CONCEPT_H
