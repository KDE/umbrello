/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef CLASS_OPERATIONS_PAGE
#define CLASS_OPERATIONS_PAGE

#include "classoperationsbase.h"

class UMLClass;
class UMLDoc;
class QListViewItem;

/** @short A Page to display / change the operations of a UMLClass 
 *
 *
*/

class ClassOperationsPage : public  ClassOperationsBase
{
Q_OBJECT
public:
	ClassOperationsPage(UMLClass *c,  UMLDoc *doc, QWidget *parent, const char *name = 0 );
	
public slots:
/** apply changes to the object*/
	virtual void apply();
/** reset changes and restore values from object*/	
	virtual void cancel();
	
	virtual void modified();

	virtual void loadData();
	
	virtual void moveUp( );
	virtual void moveDown( );
	virtual void createOperation( );
	virtual void editSelected( );
	virtual void deleteSelected( );
	virtual void itemSelected(QListViewItem *item);
protected:	
	virtual void saveData();
	
	UMLClass *m_umlObject;
	UMLDoc   *m_doc;
};

#endif
