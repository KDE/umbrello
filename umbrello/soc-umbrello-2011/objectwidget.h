/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef QGV_OBJECTWIDGET_H
#define QGV_OBJECTWIDGET_H

#include "soc-umbrello-2011/umlwidget.h"
#include "basictypes.h"

class UMLObject;
class UMLDoc;
class UMLWidgetController;

namespace QGV {

  class UMLView;
  class UMLWidget;
  
  class ObjectWidget : public UMLWidget {
    
  public:
    ObjectWidget(UMLView* view, UMLObject* o, UMLWidgetController* widgetController = 0);
    ObjectWidget(UMLView* view, IDType id = Uml::id_None, UMLWidgetController* widgetController = 0);
    virtual ~ObjectWidget();
    
    void setLocalId(Uml::IDType id);
    Uml::IDType localId() const;
    
    Uml::IDType id() const;
    void setId(Uml::IDType id);
   
  private:
    UMLView *m_view;
    UMLObject *m_object;
    Uml::IDType m_id;
    Uml::IDType m_localId;
    
  };
}

#endif