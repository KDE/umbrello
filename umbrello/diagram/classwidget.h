#ifndef CLASSWIDGET_H
#define CLASSWIDGET_H

#include "umlwidget.h"
#include <qstringlist.h>

class Diagram;
class UMLClass;

namespace Umbrello{



class ClassWidget : public UMLWidget
{
Q_OBJECT

public:
	ClassWidget(Diagram *diagram, uint id, UMLClass *object);
	
	virtual ~ClassWidget();
	
public slots:
	virtual void umlObjectModified();
	virtual void editProperties();
	
protected:
	void calculateSize();
	virtual	void drawShape(QPainter &);
	
	int m_nameDisplayOpts;
	int m_attsDisplayOpts;
	int m_opsDisplayOpts;
	
	//Cache texts	
	QString m_name;
	QStringList m_atts;
	QStringList m_ops;
};

} //end of namespace Umbrello

#endif // CLASSWIDGET_H
