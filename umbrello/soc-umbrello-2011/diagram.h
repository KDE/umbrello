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
#ifndef DIAGRAM_H
#define DIAGRAM_H

#include <QObject>

#include "basictypes.h"
#include "messagewidgetlist.h"
#include "associationwidgetlist.h"
#include "umlwidgetlist.h"

class UMLEntity;
class UMLForeignKeyConstraint;
class UMLAssociation;
class UMLClassifier;
class UMLAttribute;
class MessageWidget;
class IDChangeLog;
class UMLDoc;
class UMLWidget;

namespace QGV {

class UMLView;
class UMLScene;
class UMLWidget;

  
class Diagram : public QObject, public QGraphicsItem {
  
Q_OBJECT
public:
  Diagram(UMLView *view);
  ~Diagram();
  
  UMLView *umlview() const;
  Uml::DiagramType typeDiagram() const;
  
  void setUmlView(UMLView *view);
  void setTypeDiagram(Uml::DiagramType t);
  
  
  QString name() const;
  void setName(QString name);
  QString documentation() const;
  void setDocumentation(QString doc);
  
  QPointF pos() const;
  void setPos(QPointF pos);  
    
  void setId(Uml::IDType id);
  Uml::IDType id() const;
  
  void setLocalId(Uml::IDType id);
  Uml::IDType localId() const;
  
  int selectCount(bool filterText) const;
  UMLWidgetList_new selectedWidgets() const;
  
  void removeWidget(UMLWidget * o);
  void setupNewWidget(UMLWidget *w);
  UMLWidget *getWidgetAt(const QPointF& p);
  
  bool isOpen() const;
  void setIsOpen(bool open);
  
  virtual QRectF boundingRect() const;
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

  void clearSelected();
  void setCreateObject(bool bCreate);
  void addObject(UMLObject *object);
  bool createObject() const;
  
  void activate();
  
public slots:
  void slotObjectCreated(UMLObject *obj);
  void slotObjectRemoved(UMLObject * o);
  void activateAfterLoad(bool bUseLog);
  bool addWidget(UMLWidget * pWidget , bool isPasteOperation = false);
  UMLWidget *findWidget(Uml::IDType id);
  
signals:
  void sigWidgetRemoved(UMLWidget*);
  
private:
  UMLView *m_view;
  UMLScene *m_scene;
  Uml::DiagramType m_type;
  QString m_name;
  QString m_documentation;
  QPointF m_pos;
  Uml::IDType m_id;
  Uml::IDType m_localId; 
  UMLWidgetList_new m_widgetlist;
  AssociationWidgetList m_associationlist;
  MessageWidgetList m_messagewidgetlist;
  IDChangeLog * m_pIDChangesLog;
  bool m_paste, m_isopen, m_createobject, m_activated;
  UMLDoc *m_doc;
  
  
};

}

#endif