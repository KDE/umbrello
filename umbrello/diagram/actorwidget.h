
#ifndef DIA_UMLACTOR_H
#define DIA_UMLACTOR_H

#include "umlwidget.h"
#include <qstringlist.h>

class Diagram;
class UMLActor;



namespace Umbrello{



class ActorWidget : public UMLWidget
{
Q_OBJECT

public:
	ActorWidget(Diagram *diagram, uint id, UMLActor *object);
	
	virtual ~ActorWidget();
	
public slots:
	virtual void umlObjectModified();
	
protected:
	void calculateSize();
	virtual	void drawShape(QPainter &);
	
	QString m_name;
	
	
};

} //end of namespace Umbrello

#endif
