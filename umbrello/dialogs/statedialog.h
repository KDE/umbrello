/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef STATEDIALOG_H
#define STATEDIALOG_H

// local class includes
#include "umlwidgetstylepage.h"
#include "activitypage.h"

// kde class includes
#include "dialogbase.h"

//forward declarations
class UMLView;
class StateWidget;
class KLineEdit;
class QGroupBox;
class QLabel;
class KTextEdit;

/**
 * Displays the properties for a @ref StateWidget
 * @author   Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class StateDialog : public DialogBase
{
    Q_OBJECT

public:

    /**
     * Constructor
     */
    StateDialog( UMLView * pView, StateWidget * pWidget );

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

    void applyPage( KPageWidgetItem*item );

    /**
     * Activity page.
     */
    ActivityPage * m_pActivityPage;

    /**
     * The widget to represent.
     */
    StateWidget * m_pStateWidget;

    /**
     * The diagram the widget is on.
     */
    UMLView * m_pView;

    /**
     * Holds whether changes in the dialog have been made.
     */
    bool m_bChangesMade;

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
