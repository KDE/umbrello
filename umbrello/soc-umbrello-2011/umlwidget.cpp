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
#include "umlwidget.h"
#include "umlobject.h"
#include "umlwidgetcontroller.h"
#include "listpopupmenu.h"
#include "umldoc.h"
#include "uml.h"
#include "umlview.h"
#include "qmenu.h"
#include "kmenu.h"

#include <QDrag>
#include <QMimeData>

namespace QGV {
  

UMLWidget::UMLWidget(UMLView* view, UMLObject* o, UMLWidgetController* widgetController /* = 0*/): WidgetBase(view), 
m_menu(0), 
m_ismenuembedded(false)
{
  m_view = UMLApp::app()->current_View();
  m_view->scene()->addItem(this);
  m_object = o;
  m_widgetcontroller = widgetController;
  init();
  m_object = o;
  if (m_object) {
      connect(m_object, SIGNAL(modified()), this, SLOT(updateWidget()));
      m_id = m_object->id();
  }
  
}


UMLWidget::UMLWidget(UMLView* view, IDType id /* = Uml::id_None */, UMLWidgetController* widgetController): QGraphicsRectItem(view->sceneRect()),
  WidgetBase(UMLApp::app()->current_View())
{
  m_widgetcontroller = widgetController;
  m_id = id;
  m_view = UMLApp::app()->current_View();
  init();
}


UMLWidget::~UMLWidget()
{

}

void UMLWidget::init()
{
    m_id = Uml::id_None;
    //m_bIsInstance = false;
    if (m_view) {
        m_bUseFillColour = true;
        m_bUsesDiagramFillColour = true;
        m_bUsesDiagramUseFillColour = true;
        const Settings::OptionState& optionState = m_view->scene()->options();
        m_FillColour = optionState.uiState.fillColor;
        m_Font       = optionState.uiState.font;
        m_bShowStereotype = optionState.classState.showStereoType;
    } else {
        m_bUseFillColour = false;
        m_bUsesDiagramFillColour = false;
        m_bUsesDiagramUseFillColour = false;
        m_bShowStereotype = false;
    }
    
    for (int i = 0; i < (int)FT_INVALID; ++i)
       m_pFontMetrics[(UMLWidget::FontType)i] = 0;

    m_bResizable = true;

    m_bSelected = false;
    //m_bStartMove = false;
    m_bActivated = false;
    //m_bIgnoreSnapToGrid = false;
    //m_bIgnoreSnapComponentSizeToGrid = false;
    //m_pMenu = 0;
    m_doc = UMLApp::app()->document();
    m_posx = 0;
    setView(m_view);
    setVisible(true);
    m_view->scene()->addItem(this);
    setFlag(ItemIsMovable, true);
    setFlag(ItemIsSelectable, true);
    setFlag(ItemIsPanel, true);
    setFlag(ItemIsFocusable, true);
    setFlag(ItemSendsGeometryChanges, true);
    setAcceptDrops(true);        
    
 

    // connect( m_pView, SIGNAL(sigColorChanged(int)), this, SLOT(slotColorChanged(int)));
    m_object = NULL;
    //setZ(m_origZ = 2);  // default for most widgets

}

void UMLWidget::setView(UMLView* view)
{
  m_view = view;
}
  
Uml::Widget_Type UMLWidget::widget_type()
{
  return m_type;
}

void UMLWidget::setType(Widget_Type type)
{
 m_type = type;
}

void UMLWidget::setId(IDType id)
{
  m_id = id;
}

IDType UMLWidget::id() const
{
  return m_id;
}

int UMLWidget::onWidget(const QPointF & p)
{
    const qreal w = rect().width();
    const qreal h = rect().height();
    const qreal left = p.x();
    const qreal right = left + w;
    const qreal top = p.y();
    const qreal bottom = top + h;
    if (p.x() < left || p.x() > right ||
            p.y() < top || p.y() > bottom)   // Qt coord.sys. origin in top left corner
        return 0;
    return (w + h) / 2;
}

void UMLWidget::mouseMoveEvent(QGraphicsSceneMouseEvent* me)
{
    //m_widgetcontroller->mouseMoveEvent(me);
    //QGraphicsRectItem::mouseMoveEvent(me);
}

void UMLWidget::mousePressEvent(QGraphicsSceneMouseEvent *me)
{
    //m_widgetcontroller->mousePressEvent(me);
    if(me->button() == Qt::LeftButton){
      qDebug() << "mousePressEvent at UMLWidget";
      setCursor(Qt::ClosedHandCursor);
      
      me->accept();
      QMimeData *data = new QMimeData();
      data->setObjectName("UMLWidget");
      data->setColorData(Qt::black);
      
      QDrag *drag = new QDrag(me->widget());
      drag->setMimeData(data);
      drag->exec(Qt::MoveAction);
    }
    
}

void UMLWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *me)
{
    //m_widgetcontroller->mousePressEvent(me);
    //QGraphicsRectItem::mouseReleaseEvent(me);
}

void UMLWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* me)
{
    //m_widgetcontroller->mouseDoubleClickEvent(me);
    //QGraphicsRectItem::mouseDoubleClickEvent(me);
}

void UMLWidget::drawSelected(QPainter * p, int offsetX, int offsetY)
{
    qreal w = rect().width();
    qreal h = rect().height();
    int s = 4;
    QBrush brush(Qt::blue);
    p->fillRect(offsetX, offsetY, s,  s, brush);
    p->fillRect(offsetX, offsetY + h - s, s, s, brush);
    p->fillRect(offsetX + w - s, offsetY, s, s, brush);

    // Draw the resize anchor in the lower right corner.
    if (m_bResizable) {
        brush.setColor(Qt::red);
        const int right = offsetX + w;
        const int bottom = offsetY + h;
        p->drawLine(right - s, offsetY + h - 1, offsetX + w - 1, offsetY + h - s);
        p->drawLine(right - (s*2), bottom - 1, right - 1, bottom - (s*2));
        p->drawLine(right - (s*3), bottom - 1, right - 1, bottom - (s*3));
    } else {
        p->fillRect(offsetX + w - s, offsetY + h - s, s, s, brush);
    }
}

void UMLWidget::setFontMetrics(UMLWidget::FontType fontType, QFontMetrics fm)
{
    delete m_pFontMetrics[fontType];
    m_pFontMetrics[fontType] = new QFontMetrics(fm);
}


QFont UMLWidget::font() const
{
    return m_Font;
}

void UMLWidget::setFont(QFont font)
{
    m_Font = font;
    //forceUpdateFontMetrics(0);
    if (m_doc->loading())
        return;
    update();
}

QString UMLWidget::name() const
{
    if (m_object)
        return m_object->name();
    return m_text;
}

UMLView* UMLWidget::view() const
{
  return m_view;
}

UMLWidgetController* UMLWidget::widgetController() const
{
  return m_widgetcontroller;
}

void UMLWidget::constrain(qreal& width, qreal& height)
{
    const QSizeF minSize = calculateSize();
    if (width < minSize.width())
        width = minSize.width();
    if (height < minSize.height())
        height = minSize.height();
}

QSizeF UMLWidget::calculateSize()
{
    return QSizeF(20, 20);
}

ListPopupMenu *UMLWidget::setupPopupMenu(ListPopupMenu* menu)
{
    slotRemovePopupMenu();

    if (menu) {
        m_menu = menu;
        m_menuIsEmbedded = true;
        return m_menu;
    }

    m_menuIsEmbedded = false;
   
    //if in a multi- selection to a specific m_pMenu for that
    // NEW: ask UMLView to count ONLY the widgets and not their floatingtextwidgets
    int count = m_view->scene()->selectedItems().count();
    //a MessageWidget when selected will select its text widget and vice versa
    //so take that into account for popup menu.

    // determine multi state
    bool multi = (m_bSelected && count > 1);

    // if multiple selected items have the same type
    bool unique = false;

    // if multiple items are selected, we have to check if they all have the same
    // base type
    if (multi == true){
        //unique = m_view->checkUniqueSelection();
    }

    // create the right click context menu
    m_menu = new ListPopupMenu(m_view, this, multi, unique);

    // disable the "view code" menu for simple code generators
    if (UMLApp::app()->isSimpleCodeGeneratorActive()){
        m_menu->setActionEnabled(ListPopupMenu::mt_ViewCode, false);
    }

    connect(m_menu, SIGNAL(triggered(QAction*)), SLOT(slotMenuSelection(QAction*)));

    return m_menu;
}

bool UMLWidget::UseFillColour()
{
  return m_bUseFillColour;
}

void UMLWidget::slotMenuSelection(QAction* action)
{

}

void UMLWidget::slotRemovePopupMenu()
{

}

void UMLWidget::dragEnterEvent(QGraphicsSceneDragDropEvent* e)
{
  qDebug() << "dragEnterEvent.";
  e->acceptProposedAction(); 
  
}

void UMLWidget::dragLeaveEvent(QGraphicsSceneDragDropEvent* e)
{
  qDebug() << "dragLeaveEvent.";
}

void UMLWidget::dropEvent(QGraphicsSceneDragDropEvent* e)
{
  qDebug() << "dropEvent.";
  e->acceptProposedAction();
}

void UMLWidget::dragMoveEvent(QGraphicsSceneDragDropEvent* e)
{
  qDebug() << "dragMoveEvent.";
  update();
}

QRectF UMLWidget::rect() {
    return QRectF(QPointF(0, 0), calculateSize());
}

QFontMetrics &UMLWidget::fontMetrics(UMLWidget::FontType ft)
{
    if (m_pFontMetrics[ft] == 0) {
        setDefaultFontMetrics(ft);
    }
    return *m_pFontMetrics[ft];
}

void UMLWidget::setDefaultFontMetrics(UMLWidget::FontType ft)
{
    //setupFontType(m_Font, fontType);
    setFontMetrics(ft, QFontMetrics(m_Font));
}

void UMLWidget::setDefaultFontMetrics(UMLWidget::FontType ft, QPainter& painter)
{
	setFontMetrics(ft, QFontMetrics(m_Font));
}

  
}