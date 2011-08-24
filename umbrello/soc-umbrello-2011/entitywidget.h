/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#ifndef QGV_ENTITYWIDGET_H
#define QGV_ENTITYWIDGET_H

#include "soc-umbrello-2011/umlwidget.h"

class UMLObject;

namespace QGV {
  
  class UMLView;
  class UMLWidget;
  
  class EntityWidget : public UMLWidget {
    
  public:
    EntityWidget(UMLView* view = 0, UMLObject* o = 0);
    virtual ~EntityWidget();
    
    void draw(QPainter& p, qreal offsetX, qreal offsetY);
    
    
  private:
    UMLView *m_view;
    UMLObject *m_obj;
    UMLWidget *m_widget;  
    
    
    
  };
  
}

#endif