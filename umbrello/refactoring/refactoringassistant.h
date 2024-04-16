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

    explicit RefactoringAssistant(UMLDoc   *doc, UMLClassifier *obj = nullptr, QWidget *parent = nullptr, const QString &name = QString());
    virtual ~RefactoringAssistant();

    void refactor(UMLClassifier *obj);

Q_SIGNALS:

//    void moved();

public Q_SLOTS:

    void addBaseClassifier();
    void addDerivedClassifier();
    void addInterfaceImplementation();
    void createOperation();
    void createAttribute();
    void editProperties();
    void deleteItem();

    void objectModified();

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

    void addClassifier(UMLClassifier  *classifier, QTreeWidgetItem *parent = nullptr, bool addSuper = true, bool addSub = true, bool recurse = false);

    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);

    void setVisibilityIcon(QTreeWidgetItem *item, const UMLObject *obj);

    UMLClassifier *                     m_umlObject;
    UMLDoc *                            m_doc;
    QMenu *                             m_menu;
    QMap<QTreeWidgetItem*, UMLObject*>  m_umlObjectMap;
    QSet<UMLClassifier *>               m_alreadySeen; ///< recursive guard

private Q_SLOTS:

    void showContextMenu(const QPoint&);

private:

    QAction* createAction(const QString& text, const char * method, const Icon_Utils::IconType icon = Icon_Utils::N_ICONTYPES);

};

#endif
