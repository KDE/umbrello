/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ACTIVITYPAGE_H
#define ACTIVITYPAGE_H

#include "dialogpagebase.h"

//qt includes
#include <QListWidgetItem>
#include <QWidget>

class StateWidget;
class QGroupBox;
class QListWidget;
class QPushButton;
class QToolButton;

/**
 * @author Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ActivityPage : public DialogPageBase
{
    Q_OBJECT
public:

    ActivityPage(QWidget * pParent, StateWidget * pWidget);
    ~ActivityPage();

    void setupPage();

    void updateActivities();

protected:

    void enableWidgets(bool state);

    /**
     *  The widget to get the activities from.
     */
    StateWidget * m_pStateWidget;

    /**
     * GUI widgets
     */
    QListWidget* m_pActivityLW;
    QGroupBox*   m_pActivityGB;
    QToolButton* m_pUpArrowB;
    QToolButton* m_pDownArrowB;
    QToolButton* m_pTopArrowB;
    QToolButton* m_pBottomArrowB;
    QPushButton* m_pDeleteActivityButton;
    QPushButton* m_pRenameButton;

public Q_SLOTS:

    void slotMenuSelection(QAction* action);
    void slotClicked(QListWidgetItem* item);
    void slotDoubleClicked(QListWidgetItem* item);
    void slotRightButtonPressed(const QPoint& p);

    void slotTopClicked();
    void slotUpClicked();
    void slotDownClicked();
    void slotBottomClicked();
    void slotNewActivity();
    void slotDelete();
    void slotRename();
};

#endif
