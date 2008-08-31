/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CLASSIFIERLISTPAGE_H
#define CLASSIFIERLISTPAGE_H

//app includes
#include "listpopupmenu.h"
#include "umlclassifierlistitemlist.h"

//kde includes
#include <ktextedit.h>
#include <karrowbutton.h>
#include <kdialogbuttonbox.h>

//qt  includes
#include <QtGui/QWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QListWidget>

class UMLObject;
class UMLClassifier;
class UMLDoc;


/**
 * A dialog page to display classifier list properties.  This is not normally setup
 * by you.  It is used by the @ref ClassPropDlg and new class wizard.
 *
 * @short A dialog page to display classifier properties.
 * @author Paul Hensgen, Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassifierListPage : public QWidget
{
    Q_OBJECT
public:

    /**
     *  Sets up the ClassifierListPage.
     *  @param parent      The parent to the ClassAttPage.
     *  @param classifier  The Concept to display the properties of.
     *  @param doc         The UMLDoc document
     *  @param type        The type of listItem this handles
     */
    ClassifierListPage(QWidget* parent, UMLClassifier* classifier, UMLDoc* doc, Uml::Object_Type type);

    /**
     *  Standard deconstructor.
     */
    ~ClassifierListPage();

    void updateObject();

private:

    void setupPage();

    void setupListGroup(int margin);

    void setupDocumentationGroup(int margin);

    void setupMoveButtons(QHBoxLayout* parentLayout);

    void setupActionButtons(const QString& itemType, QVBoxLayout* parentLayout);

    void enableWidgets(bool state);

    void saveCurrentItemDocumentation();

    UMLClassifierListItemList getItemList();

    bool addClassifier(UMLClassifierListItem* classifier, int position = -1);

    bool takeItem(UMLClassifierListItem* listitem,
                  bool seekPeerBefore, int &peerIndex);

    void deleteMenu();

    void printItemList(const QString &prologue);

    QGroupBox* m_pDocGB;
    QGroupBox* m_pItemListGB;
    KTextEdit* m_pDocTE;
    KTextEdit* m_pCodeTE;
    QListWidget* m_pItemListLB;

    KArrowButton* m_pTopArrowB;
    KArrowButton* m_pUpArrowB;
    KArrowButton* m_pDownArrowB;
    KArrowButton* m_pBottomArrowB;

    UMLDoc* m_pDoc;
    ListPopupMenu* m_pMenu;
    UMLClassifierListItem* m_pOldListItem;

protected:

    void reloadItemListBox();

    void hideArrowButtons(bool hide);

    virtual int calculateNewIndex(Uml::Object_Type ot);

    virtual int relativeIndexOf(QListWidgetItem* item) {
        return m_pItemListLB->row(item);
    }

    Uml::Object_Type m_itemType;
    UMLClassifier* m_pClassifier;
    UMLClassifierListItem* m_pLastObjectCreated;

    QPushButton* m_pNewClassifierListItemButton;
    QPushButton* m_pDeleteListItemButton;
    QPushButton* m_pPropertiesButton;

    bool m_bSigWaiting;

public slots:

    void slotClicked(QListWidgetItem* item);

    void slotPopupMenuSel(QAction* action);

    void slotListItemCreated(UMLObject* object);
    void slotListItemModified();
    void slotRightButtonPressed(const QPoint& p);

    void slotDoubleClick(QListWidgetItem* item);

    void slotTopClicked();

    void slotUpClicked();

    void slotDownClicked();

    void slotBottomClicked();

    void slotNewListItem();

    void slotDelete();

    void slotProperties();
};

#endif
