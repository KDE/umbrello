/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef CLASS_PROPERTIES_PAGE
#define CLASS_PROPERTIES_PAGE

#include "classpropertiesbase.h"
#include "dialogpage.h"

class UMLClass;

/** @short A Page to display / change basic properties of a UMLClass 
 *
 *
*/

class ClassPropertiesPage : public  ClassPropertiesBase, public DialogPage
{
Q_OBJECT
public:
	ClassPropertiesPage(UMLClass *c, QWidget *parent, const char *name = 0 );
	
public slots:
/** apply changes to the object*/
	virtual void apply();
/** reset changes and restore values from object*/	
	virtual void cancel();
	
	virtual void pageContentsModified();

	virtual void loadData();

signals:
	void pageModified( );

protected:
	virtual void saveData();
	
	UMLClass *m_umlObject;	
};

#endif
