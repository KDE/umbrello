
#ifndef DIA_DIAGRAMVIEW_H
#define DIA_DIAGRAMVIEW_H

#include <qcanvas.h>
#include <map>
#include <typeinfo>

#include "../umlnamespace.h"
#include "../worktoolbar.h"

class QPopupMenu;


namespace Umbrello {

class Diagram;

class DiagramView : public QCanvasView
{
Q_OBJECT

public:
	DiagramView( Diagram *diagram, WorkToolBar *toolbar, 
		QWidget *parent = 0, const char *name= 0, WFlags f = 0);
		
	virtual ~DiagramView();
	
	Diagram* diagram() const;
	
public slots:
	virtual void setTool(WorkToolBar::EditTool tool );
	
protected:

	virtual void contentsMousePressEvent( QMouseEvent *e );
	virtual void contentsMouseReleaseEvent(QMouseEvent *e );
	virtual void contentsMouseMoveEvent( QMouseEvent *e );
	virtual void contentsDoubleClickEvent( QMouseEvent *e );
	virtual void contentsDragEnterEvent(QDragEnterEvent *e);
	virtual void contentsContextMenuEvent(QContextMenuEvent *e);
	virtual void contentsDropEvent(QDropEvent *e);
	
	virtual void focusInEvent(QFocusEvent *);
	

	WorkToolBar *toolBar;
	WorkToolBar::EditTool m_tool;
	QPopupMenu *m_contextMenu;
	
	QPoint m_savedPosition;
	QPoint m_selectFrom, m_selectTo;
	QCanvasRectangle *m_selectionRect;
	QPtrList<QCanvasLine> *m_linePath;
	
	long m_currentAction;
	static std::map<WorkToolBar::EditTool, const std::type_info*> toolMap;
private:
	void initToolMap();
};


};

#endif

