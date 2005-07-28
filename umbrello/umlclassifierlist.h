/*
 *  copyright (C) 2003
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLCLASSIFIERLIST_H
#define UMLCLASSIFIERLIST_H

#include <q3ptrlist.h>

// forward declaration
class UMLClassifier;

typedef Q3PtrList<UMLClassifier> UMLClassifierList;
typedef Q3PtrListIterator<UMLClassifier> UMLClassifierListIt;

#endif
