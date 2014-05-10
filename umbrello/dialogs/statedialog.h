/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef STATEDIALOG_H
#define STATEDIALOG_H

// kde class includes
#include "dialogbase.h"

//forward declarations
class ActivityPage;
class UMLView;
class StateWidget;
class KLineEdit;
class QGroupBox;
class QLabel;
class KTextEdit;

/**
 * Displays the properties for a @ref StateWidget
 * @author   Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class StateDialog : public DialogBase
{
    Q_OBJECT
public:
    StateDialog(QWidget *parent, StateWidget * pWidget);

    /**
     * Returns whether changes were made.
     */
    bool getChangesMade() {
        return m_bChangesMade;
    }

protected slots:
    void slotOk();
    void slotApply();

protected:
    void setupPages();
    void setupGeneralPage();
    void setupActivityPage();

    void applyPage(KPageWidgetItem*item);

    ActivityPage *m_pActivityPage;
    StateWidget  *m_pStateWidget;  ///< the widget to represent
    bool          m_bChangesMade;  ///< holds whether changes in the dialog have been made

    struct GeneralPageWidgets {
        QLabel * nameL, * typeL;
        KLineEdit * nameLE, * typeLE;
        KTextEdit * docMLE;
        QGroupBox * docGB, * generalGB;
    }
    m_GenPageWidgets;

    KPageWidgetItem *pageStyle, *pageActivity, *pageFont, *pageGeneral;
};

#endif
