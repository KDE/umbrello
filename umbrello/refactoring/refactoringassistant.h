/***************************************************************************
                          refactoringassistant.h
                             -------------------
    copyright            : (C) 2003 Luis De la Parra
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 
 #ifndef REFACTORING_ASSISTANT
 #define REFACTORING_ASSISTANT
 
 
 #include <klistview.h>
 #include <qpixmap.h>
 #include <map>
 
 
 class UMLClass;
 class UMLObject;
 class UMLClassifier;
 class UMLAttribute;
 class UMLOperation;
 class UMLDoc;
 
 class QPopupMenu;
 class QPoint;
 
 
 namespace Umbrello{
 
class RefactoringAssistant : public KListView
{
        Q_OBJECT
public:
	typedef std::map<QListViewItem*, UMLObject*> UMLObjectMap;
	
	RefactoringAssistant( UMLDoc *doc, UMLClassifier *obj = 0, QWidget *parent = 0, const char *name = 0 );
	virtual ~RefactoringAssistant();
	
	void refactor( UMLClassifier *obj );
	
public slots:

	void addBaseClassifier();
	void addDerivedClassifier();
	void addInterfaceImplementation();
	void createOperation( );
	void createAttribute( );
	void editProperties( );
	
	void umlObjectModified( const UMLObject *obj = 0 );
	
	void operationAdded( UMLOperation *op );
	void operationRemoved( UMLOperation *op );
	
	void attributeAdded( UMLAttribute *att );
	void attributeRemoved( UMLAttribute *att );
	
	void itemExecuted( QListViewItem *item );
	void showContextMenu( KListView*, QListViewItem*, const QPoint&);
		
protected:
	struct { QPixmap Public,
			 Protected,
			 Private,
			 Generalization,
			 Subclass;
		} m_pixmaps;
		
	UMLObject* findUMLObject( const QListViewItem* );
	QListViewItem* findListViewItem( const UMLObject *obj );
	void editProperties( UMLObject *obj );
	void addClassifier( UMLClassifier *classifier, QListViewItem *parent = 0, bool addSuper = true, bool addSub = true, bool recurse = false );
	void loadPixmaps();
	virtual bool acceptDrag(QDropEvent *event) const;
	virtual void movableDropEvent (QListViewItem* parent, QListViewItem* afterme);
	void setVisibilityIcon( QListViewItem *item , const UMLObject *obj );
	UMLClassifier *m_umlObject;
	UMLDoc *m_doc;
	QPopupMenu *m_menu;	
	UMLObjectMap m_umlObjectMap;
	
 
 };


}
 
 #endif
 
