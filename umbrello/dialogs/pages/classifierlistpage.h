/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CLASSIFIERLISTPAGE_H
#define CLASSIFIERLISTPAGE_H

//app includes
#include "dialogpagebase.h"
#include "umlclassifierlistitemlist.h"
#include "umlobject.h"

class CodeTextEdit;
class UMLClassifier;
class UMLDoc;
class KTextEdit;
class QGroupBox;
class QHBoxLayout;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QToolButton;
class QVBoxLayout;

/**
 * A dialog page to display classifier list properties.  This is not normally setup
 * by you.  It is used by the @ref ClassPropertiesDialog and new class wizard.
 *
 * @short A dialog page to display classifier properties.
 * @author Paul Hensgen, Jonathan Riddell
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ClassifierListPage : public DialogPageBase
{
    Q_OBJECT
public:

    ClassifierListPage(QWidget* parent,
                       UMLClassifier* classifier, UMLDoc* doc, UMLObject::ObjectType type);
    virtual ~ClassifierListPage();

    void apply();

private:

    void setupPage();

    void setupListGroup(int margin);
    void setupDocumentationGroup(int margin);
    void setupMoveButtons(QHBoxLayout* parentLayout);
    void setupActionButtons(const QString& itemType, QVBoxLayout* parentLayout);

    void enableWidgets(bool state);

    void saveCurrentItemDocumentation();

    virtual UMLClassifierListItemList getItemList();

    bool addToClassifier(UMLClassifierListItem* classifier, int position = -1);

    bool takeItem(UMLClassifierListItem* listItem,
                  bool seekPeerBefore, int &peerIndex);

    void deleteMenu();

    void printItemList(const QString &prologue);

    QGroupBox*     m_docGB;
    QGroupBox*     m_pItemListGB;
    KTextEdit*     m_docTE;
    CodeTextEdit*  m_pCodeTE;
    QListWidget*   m_pItemListLB;

    QToolButton*   m_pTopArrowB;
    QToolButton*   m_pUpArrowB;
    QToolButton*   m_pDownArrowB;
    QToolButton*   m_pBottomArrowB;

    UMLDoc*        m_doc;
    UMLClassifierListItem* m_pOldListItem;

protected:

    void reloadItemListBox();

    void hideArrowButtons(bool hide);

    UMLObject::ObjectType m_itemType;
    UMLClassifier* m_pClassifier;
    UMLClassifierListItem* m_pLastObjectCreated;

    QPushButton* m_pNewClassifierListItemButton;
    QPushButton* m_pDeleteListItemButton;
    QPushButton* m_pPropertiesButton;

    bool m_bSigWaiting;

protected slots:

    void slotActivateItem(QListWidgetItem* item);

    void slotMenuSelection(QAction* action);

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
