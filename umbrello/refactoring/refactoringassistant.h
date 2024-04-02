/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Luis De la Parra <lparrab@gmx.net>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef REFACTORING_ASSISTANT_H
#define REFACTORING_ASSISTANT_H

#include "icon_utils.h"

#include <QDropEvent>
#include <QTreeWidget>

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

    explicit RefactoringAssistant(UMLDoc *doc, UMLClassifier *obj = nullptr, QWidget *parent = nullptr, const QString &name = QString());
    virtual ~RefactoringAssistant();

    void refactor(UMLClassifier *obj);

    Q_SLOT void addBaseClassifier();
    Q_SLOT void addDerivedClassifier();
    Q_SLOT void addInterfaceImplementation();
    Q_SLOT void createOperation();
    Q_SLOT void createAttribute();
    Q_SLOT void editProperties();
    Q_SLOT void deleteItem();

    Q_SLOT void objectModified();

    Q_SLOT void operationAdded(UMLClassifierListItem *listItem);
    Q_SLOT void operationRemoved(UMLClassifierListItem *listItem);

    Q_SLOT void attributeAdded(UMLClassifierListItem *listItem);
    Q_SLOT void attributeRemoved(UMLClassifierListItem *listItem);

    Q_SLOT void itemExecuted(QTreeWidgetItem *item, int column);

protected:

    UMLObject* findUMLObject(const QTreeWidgetItem*);

    QTreeWidgetItem* findListViewItem(const UMLObject *obj);

    void editProperties(UMLObject *obj);

    void deleteItem(QTreeWidgetItem *item, UMLObject *obj);

    void addClassifier(UMLClassifier *classifier, QTreeWidgetItem *parent = nullptr, bool addSuper = true, bool addSub = true, bool recurse = false);

    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);

    void setVisibilityIcon(QTreeWidgetItem *item, const UMLObject *obj);

    UMLClassifier *                     m_umlObject;
    UMLDoc *                            m_doc;
    QMenu *                             m_menu;
    QMap<QTreeWidgetItem*, UMLObject*>  m_umlObjectMap;
    QSet<UMLClassifier *>               m_alreadySeen; ///< recursive guard

private:
    Q_SLOT void showContextMenu(const QPoint&);

    QAction* createAction(const QString& text, const char * method, const Icon_Utils::IconType icon = Icon_Utils::N_ICONTYPES);

};

#endif
