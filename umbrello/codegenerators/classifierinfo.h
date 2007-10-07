/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003 Brian Thomas <brian.thomas@gsfc.nasa.gov>          *
 *   copyright (C) 2004-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CLASSIFIERINFO_H
#define CLASSIFIERINFO_H

#include "../umldoc.h"
#include "../attribute.h"
#include "../association.h"
#include "../umlclassifierlist.h"
#include "../umlassociationlist.h"
#include "../umlattributelist.h"

#include <qstring.h>


class UMLClassifier;
	

/**
  * class ClassInfo is an object to hold summary information about a classifier
  * in a convenient form for easy access by a code generator.
  */
class ClassifierInfo {
public:

    /**
     * Constructor, initialises a couple of variables
     */
    ClassifierInfo (UMLClassifier * classifier);

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
    UMLAssociationList uniAssociations;
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
    UMLAttributeList getAttList();

private:

    UMLClassifier* classifier_;
    
    /**
     * Utility method called by "get*ChildClassfierList()" methods. It basically
     * finds all the classifiers named in each association in the given association list
     * which arent the current one. Usefull for finding which classifiers are "owned" by the
     * current one via declared associations such as in aggregations/compositions.
     */
    UMLClassifierList findAssocClassifierObjsInRoles (UMLAssociationList * list);

};

#endif // CLASSIFIERINFO_H

