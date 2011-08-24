/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/ 

#include "print.h"
#include "umlscene.h"
#include "messagewidgetlist.h"
#include "umlwidget.h"
#include "umlwidgetlist.h"
#include "associationwidgetlist.h"
#include "diagram.h"
#include "klocalizedstring.h"



// system includes
#include <cmath>  // for ceil

Print::Print(UMLScene *scene, Diagram *diagram)
{
  m_scene = scene;
  m_diagram = diagram;  
}

Print::~Print()
{
    delete this;
}


