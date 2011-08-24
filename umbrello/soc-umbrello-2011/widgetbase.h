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
#ifndef QGV_WIDGETBASE_H
#define QGV_WIDGETBASE_H

#include <QObject>
#include "umlnamespace.h"


class UMLObject;
class UMLDoc;

using namespace Uml;

namespace QGV {
  
  class UMLView;
  
  class WidgetBase : public QObject {
    
    Q_OBJECT
  public:
    WidgetBase(UMLView *view);
    virtual ~WidgetBase();
    
    void init(UMLView *view, Uml::Widget_Type type = Uml::wt_UMLWidget);
    
    
    Uml::Widget_Type widget_type();
    void setBaseType( Uml::Widget_Type type );
    
    UMLObject* umlObject() const;
    void setUmlObject(UMLObject *o);
    
    UMLView* view() const;
    void setView(UMLView *view);
    
    UMLDoc* umlDoc() const;
    void setUmlDoc(UMLDoc *doc);
    
    bool m_bUsesDiagramLineColour, m_bUsesDiagramLineWidth;
    QColor m_LineColour;
    
  private:
    UMLView *m_view;
    Uml::Widget_Type m_type;
    Uml::IDType m_id;
    UMLObject *m_object;
    UMLDoc *m_doc;
    int m_LineWidth;
    
  };
}

#endif