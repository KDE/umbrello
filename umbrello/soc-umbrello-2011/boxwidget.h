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

#ifndef QGV_BOXWIDGET
#define QGV_BOXWIDGET
//qt includes
#include <qpainter.h>
//app includes
#include "umlwidget.h"

#include <QDomDocument>

class UMLObject;
class UMLWidgetController;
class UMLDoc;

namespace QGV {
  
  class UMLView;
  class UMLWidget;
  class Diagram;
  
  class BoxWidget : public UMLWidget {
   
  public:
    BoxWidget(UMLView * view, Uml::IDType id = Uml::id_None);
    virtual ~BoxWidget();
    
    void draw(QPainter& p, int offsetX, int offsetY);
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

  private:
    int m_origZ;
    UMLView *m_view;
    Diagram *m_diagram;
    
  };
  
}

#endif
