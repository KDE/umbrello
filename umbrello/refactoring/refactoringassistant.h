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
	RefactoringAssistant( UMLDoc *doc, UMLClassifier *obj = 0, QWidget *parent = 0, const char *name = 0 );
	virtual ~RefactoringAssistant();
	
	void setObject( UMLClassifier *obj );
public slots:
	void itemExecuted( QListViewItem *item );
	void showContextMenu( KListView*, QListViewItem*, const QPoint&);
	void addSuperClassifier();
	void addDerivedClassifier();
	void addInterfaceImplementation();
	
protected:
	struct { QPixmap Public,
			 Protected,
			 Private,
			 Generalization,
			 Subclass;
		} m_pixmaps;
		
	void addClassifier( UMLClassifier *classifier, QListViewItem *parent = 0, bool addSuper = true, bool addSub = true, bool recurse = false );
	void loadPixmaps();
	virtual bool acceptDrag(QDropEvent *event) const;
	virtual void movableDropEvent (QListViewItem* parent, QListViewItem* afterme);
	UMLClassifier *m_umlObject;
	UMLDoc *m_doc;
	QPopupMenu *m_menu;

	std::map<QListViewItem*, UMLObject*> m_umlObjectMap;
	
 
 };


}
 
 #endif
 
