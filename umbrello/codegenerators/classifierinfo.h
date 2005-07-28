/***************************************************************************
                          classifierinfo.h  -  description
                             -------------------
    copyright            : (C) 2003 Brian Thomas
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CLASSIFIERINFO_H
#define CLASSIFIERINFO_H

#include "../classifier.h"
#include "../umldoc.h"
#include "../attribute.h"
#include "../association.h"
#include "../umlclassifierlist.h"
#include "../umlassociationlist.h"
#include "../umlattributelist.h"

#include <q3ptrlist.h>
#include <qstring.h>

/**
  * class ClassInfo is an object to hold summary information about a classifier
  * in a convient form for easy access by a code generator.
  */
class ClassifierInfo {
public:

    /**
     * Constructor, initialises a couple of variables
     */
    ClassifierInfo (UMLClassifier * classifier, UMLDoc * doc);

    /**
     * Destructor, empty
     */
    virtual ~ClassifierInfo();

    // Fields
    //

    /**
     * Lists of attributes of this classifier (if a class)
     * Sorted by scope.
     */
    UMLAttributeList atpub;
    UMLAttributeList atprot;
    UMLAttributeList atpriv;

    /**
     * Lists of static attributes of this classifier (if a class)
     */
    UMLAttributeList static_atpub;
    UMLAttributeList static_atprot;
    UMLAttributeList static_atpriv;

    /**
     * Lists of types of associations this classifier has
     */
    UMLAssociationList plainAssociations;
    UMLAssociationList aggregations;
    UMLAssociationList compositions;

    /**
     * what sub and super classifiers are related to this class
     */
    UMLClassifierList superclasses;
    UMLClassifierList subclasses;

    /**
     * Various conditional information about our classifier.
     */
    bool isInterface; // Whether or not this classifier is an interface.
    bool hasAssociations;
    bool hasAttributes;
    bool hasStaticAttributes;
    bool hasMethods;
    bool hasAccessorMethods;
    bool hasOperationMethods;
    bool hasVectorFields;

    /**
     * Class and File names
     */
    QString className;
    QString fileName;

    /**
     * utility functions to allow easy determination of what classifiers
     * are "owned" by the current one via named association type (e.g. 
     * plain, aggregate or compositions).
     */
    UMLClassifierList getPlainAssocChildClassifierList();
    UMLClassifierList getAggregateChildClassifierList();
    UMLClassifierList getCompositionChildClassifierList();

    /**
     * Utility method to obtain list of attributes, if they exist, for
     * the current classfier.
     */
    UMLAttributeList* getAttList();


protected:
    void init (UMLClassifier *c);

private:

    Uml::IDType m_nID; // id of the classifier

    /**
     * Utility method called by "get*ChildClassfierList()" methods. It basically 
     * finds all the classifiers named in each association in the given association list 
     * which arent the current one. Usefull for finding which classifiers are "owned" by the 
     * current one via declared associations such as in aggregations/compositions.
     */
    UMLClassifierList findAssocClassifierObjsInRoles (UMLAssociationList * list);

    /**
     *      List of all the attributes in this class.
     */
    UMLAttributeList m_AttsList;

};

#endif // CLASSIFIERINFO_H

