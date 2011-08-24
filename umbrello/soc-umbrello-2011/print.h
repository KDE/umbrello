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

#ifndef PRINT_H
#define PRINT_H

#include <Qt/qobject.h>
#include <QtGui/QPainter>
#include <QtGui/QPrinter>

class UMLScene;
class Diagram;

class Print {


  public:
    Print(UMLScene *scene, Diagram *diagram);
    ~Print();
    QRectF diagramRect();
    
  private:
    UMLScene *m_scene;
    Diagram *m_diagram;    

  
  
};

#endif
