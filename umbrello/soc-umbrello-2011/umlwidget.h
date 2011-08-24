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
#ifndef QGV_UMLWIDGET_H
#define QGV_UMLWIDGET_H


#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QtGui/QFont>
#include <QDomElement>
#include <qpainter.h>

#include "umlnamespace.h"
#include "widgetbase.h"

class UMLDoc;
class UMLObject;
class ListPopupMenu;
class UMLWidgetController;
class QFontMetrics;
class QMenu;


using namespace Uml;

namespace QGV {
  
    
  class UMLView;
  
  class UMLWidget : public WidgetBase, public QGraphicsRectItem {
	  

  
  Q_OBJECT
  Q_ENUMS(FontType)
  public:
	  
    enum FontType {
        FT_NORMAL = 0,
        FT_BOLD  = 1,
        FT_ITALIC = 2,
        FT_UNDERLINE = 3,
        FT_BOLD_ITALIC = 4,
        FT_BOLD_UNDERLINE = 5,
        FT_ITALIC_UNDERLINE = 6,
        FT_BOLD_ITALIC_UNDERLINE = 7,
        FT_INVALID = 8
    };
    
    UMLWidget(UMLView * view, UMLObject * o, UMLWidgetController *widgetController = 0);
    UMLWidget(UMLView * view, Uml::IDType id = Uml::id_None, UMLWidgetController *widgetController = 0);
    virtual ~UMLWidget();
    
    void init();
    
    void setView(UMLView *view);
    
    Uml::Widget_Type widget_type();
    void setType(Uml::Widget_Type type);
    
    QFontMetrics &fontMetrics(UMLWidget::FontType ft);
    
    void setDefaultFontMetrics(UMLWidget::FontType ft);
    void setDefaultFontMetrics(UMLWidget::FontType ft, QPainter &painter);
    
    void setId(IDType id);
    IDType id() const;
    
    int onWidget(const QPointF & p);
    
   
  void mouseMoveEvent(QGraphicsSceneMouseEvent* me);
  void mousePressEvent(QGraphicsSceneMouseEvent *me);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *me);
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *me);
  
  void dragEnterEvent(QGraphicsSceneDragDropEvent *e);
  void dragLeaveEvent(QGraphicsSceneDragDropEvent *e);
  void dropEvent(QGraphicsSceneDragDropEvent *e);
  void dragMoveEvent(QGraphicsSceneDragDropEvent *e);
  
  virtual void drawSelected(QPainter * p, int offsetX, int offsetY);
  
  void setFont(QFont font);
  QFont font() const;
  void setFontMetrics(UMLWidget::FontType fontType, QFontMetrics fm);
  
  bool m_bUseFillColour, m_bShowStereotype, m_bResizable,
  m_bUsesDiagramFillColour, m_bUsesDiagramUseFillColour, 
  m_bSelected, m_bActivated, m_ismenuembedded;

  //virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );
  //virtual bool loadFromXMI( QDomElement & qElement );
  
  QString name() const;
  UMLView *view() const;
  UMLWidgetController *widgetController() const;
  ListPopupMenu *m_menu;
  QSizeF calculateSize();  
  void constrain(qreal& width, qreal& height);
  ListPopupMenu *setupPopupMenu(ListPopupMenu* menu = 0);
  bool UseFillColour();
  QRectF rect();
  
  public slots:
    void slotRemovePopupMenu();
    virtual void slotMenuSelection(QAction* action);
  
  private:
    UMLView *m_view;
    UMLObject *m_object;
    UMLWidgetController *m_widgetcontroller;
    IDType m_id;
    Uml::Widget_Type m_type;
    bool m_bUsesDiagramLineColour, m_bUsesDiagramLineWidth, m_menuIsEmbedded;
    QColor m_LineColour;
    int m_LineWidth;
    qreal m_posx;
    UMLDoc *m_doc;
    QColor m_FillColour;
    QFont m_Font;    
    QFontMetrics  *m_pFontMetrics[FT_INVALID];
    QString m_text;
    
    
  };
  
}

#endif