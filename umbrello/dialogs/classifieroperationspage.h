/***************************************************************************
                          classifieroperationspage.h
                             -------------------
    copyright            : (C) 2003 Luis De la Parra
  Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef CLASSIFIER_OPERATIONS_PAGE
#define CLASSIFIER_OPERATIONS_PAGE

#include "classifieroperationsbase.h"
#include <qptrlist.h>
#include <qmap.h>
#include <qpixmap.h>
#include "dialogpage.h"

class UMLClassifier;
class UMLOperation;
class UMLDoc;
class QListViewItem;

namespace Umbrello{


/** @short A Page to display / change the operations of a Classifier 
 *
 * @description A dialog Page to display / change the operations of a Classifier
 * Changes will be made to the Classifier when apply() is called, or inmediatly if
 * autoApply is true.
 * If the parent widget is null, the page will default to autoApply, but you can 
 * change this anytime.
 * @see also DialogPage
 *
*/
class ClassifierOperationsPage : public  ClassifierOperationsBase, public DialogPage
{
Q_OBJECT
public:
	/** Constructor
	* @param c The classifier being observed
	* @param doc The document (model) the class belongs to. This is needed because
	*        class attributes are not created / destroyed by the class itself, but
	*        by the document
	* @param parent The widget parent, normally a UmbrelloDialog or null
	* @param name   The name of the page*/
	ClassifierOperationsPage(UMLClassifier *c, UMLDoc *doc, QWidget *parent, const char *name = 0 );
	
	virtual ~ClassifierOperationsPage( );
	
public slots:
	/** apply changes to the object being observed*/
	virtual void apply();
	/** reset changes and restore values from observed object*/	
	virtual void cancel();
	/** Inform the page that a field has been modified. Do not use (internal) */
	virtual void pageContentsModified();
	/** Load the widget data from the UMLObject. */
	virtual void loadData();
protected:	
	virtual void moveUp( );
	virtual void moveDown( );
	virtual void createOperation( );
	virtual void editSelected( );
	virtual void deleteSelected( );
	virtual void itemSelected(QListViewItem *item);

signals:
	void pageModified( );

protected:
/** Apply changes made in the page to the classifier being observed */
	virtual void saveData();
	virtual void loadPixmaps();
	struct { QPixmap Public,
			 Protected,
			 Private;
		} m_pixmaps;
	
	void syncParams( UMLOperation *src, UMLOperation *dest );
	UMLClassifier *m_umlObject;
	UMLDoc   *m_doc;
	QPtrList<UMLOperation> m_opsList;
	QMap<QListViewItem*,UMLOperation*> m_opsMap;
};

} //namespace Umbrello

#endif

