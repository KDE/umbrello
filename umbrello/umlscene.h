/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#ifndef UMLSCENE_H
#define UMLSCENE_H

#include "UMLView.h"

/**
 * UMLScene instances represent diagrams.
 * The UMLScene class inherits from UMLView yet and 
 * in the future from QGraphicsScene.
 */
class UMLScene : public UMLView
{
public:
    UMLScene(UMLFolder *parentFolder) : UMLView(parentFolder) {}
};

#endif