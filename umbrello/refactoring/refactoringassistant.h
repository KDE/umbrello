/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003 Luis De la Parra <lparrab@gmx.net>                 *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef REFACTORING_ASSISTANT_H
#define REFACTORING_ASSISTANT_H

#include <QtGui/QTreeWidget>
#include <QtGui/QDropEvent>

#include "icon_utils.h"

class QPoint;
class QMenu;
class UMLObject;
class UMLClassifier;
class UMLClassifierListItem;
class UMLDoc;

class RefactoringAssistant : public QTreeWidget
{
    Q_OBJECT
public:

    explicit RefactoringAssistant(UMLDoc *doc, UMLClassifier *obj = 0, QWidget *parent = 0, const QString &name = QString());
    virtual ~RefactoringAssistant();

    void refactor(UMLClassifier *obj);

signals:

//    void moved();

public slots:

    void addBaseClassifier();
    void addDerivedClassifier();
    void addInterfaceImplementation();
    void createOperation();
    void createAttribute();
    void editProperties();
    void deleteItem();

    void objectModified(const UMLObject *obj = 0);

    void operationAdded(UMLClassifierListItem *listItem);
    void operationRemoved(UMLClassifierListItem *listItem);

    void attributeAdded(UMLClassifierListItem *listItem);
    void attributeRemoved(UMLClassifierListItem *listItem);

    void itemExecuted(QTreeWidgetItem *item, int column);

protected:

    UMLObject* findUMLObject(const QTreeWidgetItem*);

    QTreeWidgetItem* findListViewItem(const UMLObject *obj);

    void editProperties(UMLObject *obj);

    void deleteItem(QTreeWidgetItem *item, UMLObject *obj);

    void addClassifier(UMLClassifier *classifier, QTreeWidgetItem *parent = 0, bool addSuper = true, bool addSub = true, bool recurse = false);

    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);

    void setVisibilityIcon(QTreeWidgetItem *item , const UMLObject *obj);

    UMLClassifier *                     m_umlObject;
    UMLDoc *                            m_doc;
    QMenu *                             m_menu;
    QMap<QTreeWidgetItem*, UMLObject*>  m_umlObjectMap;

private slots:

    void showContextMenu(const QPoint&);

private:

    QAction* createAction(const QString& text, const char * method, const Icon_Utils::IconType icon = Icon_Utils::N_ICONTYPES);

};

#endif
