/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef CLASS_ATTRIBUTES_PAGE
#define CLASS_ATTRIBUTES_PAGE

#include "classattributesbase.h"
#include <qptrlist.h>
#include "dialogpage.h"

class UMLClass;
class UMLAttribute;
class UMLDoc;
class QListViewItem;

/** @short A Page to display / change the attributes of a UMLClass 
 *
 *
*/

class ClassAttributesPage : public  ClassAttributesBase, public DialogPage
{
Q_OBJECT
public:
	ClassAttributesPage(UMLClass *c, UMLDoc *doc, QWidget *parent, const char *name = 0 );
	
public slots:
/** apply changes to the object*/
	virtual void apply();
/** reset changes and restore values from object*/	
	virtual void cancel();
	
	virtual void pageContentsModified();

	virtual void loadData();
	
	virtual void moveUp( );
	virtual void moveDown( );
	virtual void createAttribute( );
	virtual void editSelected( );
	virtual void deleteSelected( );
	virtual void itemSelected(QListViewItem *item);

signals:
	void pageModified( );

protected:	
	virtual void saveData();
	virtual void loadPixmaps();
	struct { QPixmap Public,
			 Protected,
			 Private;
		} m_pixmaps;
	
	UMLClass *m_umlObject;
	UMLDoc   *m_doc;
	QPtrList<UMLAttribute> m_newAtts;
	QPtrList<UMLAttribute> m_deletedAtts;
	QPtrList<UMLAttribute> m_modifiedAtts;
};

#endif
