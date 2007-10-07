/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright       : (C) 2003 Brian Thomas brian.thomas@gsfc.nasa.gov    *
 *   (C) 2004-2006  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>   *
 ***************************************************************************/


#include "classifierinfo.h"

#include "../classifier.h"
#include "../operation.h"

ClassifierInfo::ClassifierInfo( UMLClassifier *classifier)
{
    classifier_ = classifier;

    // set default class, file names
    className = classifier->getName();
    fileName = classifier->getName().lower();

    // determine up-front what we are dealing with
    isInterface = classifier->isInterface();

    // sort attributes by Scope
    if(!isInterface) {
        UMLAttributeList atl = classifier->getAttributeList();
        for(UMLAttribute *at=atl.first(); at ; at=atl.next()) {
            switch(at->getVisibility())
            {
              case Uml::Visibility::Public:
                if(at->getStatic())
                    static_atpub.append(at);
                else
                    atpub.append(at);
                break;
              case Uml::Visibility::Protected:
                if(at->getStatic())
                    static_atprot.append(at);
                else
                    atprot.append(at);
                break;
              case Uml::Visibility::Private:
              case Uml::Visibility::Implementation:
                    if(at->getStatic())
                    static_atpriv.append(at);
                else
                    atpriv.append(at);
                break;
            }
        }
    }

    // inheritance issues
    superclasses = classifier->getSuperClasses(); // list of what we inherit from

    subclasses = classifier->getSubClasses();     // list of what inherits from us

    // another preparation, determine what we have
    plainAssociations = classifier->getSpecificAssocs(Uml::at_Association); // BAD! only way to get "general" associations.

    uniAssociations = classifier->getUniAssociationToBeImplemented();

    aggregations = classifier->getAggregations();

    compositions = classifier->getCompositions();

    // set some summary information about the classifier now
    hasAssociations = plainAssociations.count() > 0 || aggregations.count() > 0 || compositions.count() > 0 || uniAssociations.count() > 0;
    hasAttributes = atpub.count() > 0 || atprot.count() > 0 || atpriv.count() > 0
                    || static_atpub.count() > 0
                    || static_atprot.count() > 0
                    || static_atpriv.count() > 0;

    hasStaticAttributes = static_atpub.count() > 0
                          || static_atprot.count() > 0
                          || static_atpriv.count() > 0;

    hasAccessorMethods = hasAttributes || hasAssociations;

    hasOperationMethods = classifier->getOpList().last() ? true : false;

    hasMethods = hasOperationMethods || hasAccessorMethods;

    // this is a bit too simplistic..some associations are for
    // SINGLE objects, and WONT be declared as Vectors, so this
    // is a bit overly inclusive (I guess that's better than the other way around)
    hasVectorFields = hasAssociations ? true : false;
}

ClassifierInfo::~ClassifierInfo() { }

UMLClassifierList ClassifierInfo::getPlainAssocChildClassifierList()
{
    return findAssocClassifierObjsInRoles(&plainAssociations);
}

UMLClassifierList ClassifierInfo::getAggregateChildClassifierList()
{
    return findAssocClassifierObjsInRoles(&aggregations);
}

UMLClassifierList ClassifierInfo::getCompositionChildClassifierList() 
{
    return findAssocClassifierObjsInRoles(&compositions);
}

UMLClassifierList ClassifierInfo::findAssocClassifierObjsInRoles (UMLAssociationList * list)
{
    UMLClassifierList classifiers;

    for (UMLAssociation *a = list->first(); a; a = list->next()) {
        // DONT accept a classifier IF the association role is empty, by
        // convention, that means to ignore the classifier on that end of
        // the association.
        // We also ignore classifiers which are the same as the current one
        // (e.g. id matches), we only want the "other" classifiers
        if (a->getObjectId(Uml::A) == classifier_->getID() && !a->getRoleName(Uml::B).isEmpty()) {
            UMLClassifier *c = dynamic_cast<UMLClassifier*>(a->getObject(Uml::B));
            if(c)
                classifiers.append(c);
        } else if (a->getObjectId(Uml::B) == classifier_->getID() && !a->getRoleName(Uml::A).isEmpty()) {
            UMLClassifier *c = dynamic_cast<UMLClassifier*>(a->getObject(Uml::A));
            if(c)
                classifiers.append(c);
        }
    }

    return classifiers;
}

UMLAttributeList ClassifierInfo::getAttList()
{
    return classifier_->getAttributeList();
}

