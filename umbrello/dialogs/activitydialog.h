/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ACTIVITYDIALOG_H
#define ACTIVITYDIALOG_H

//local class includes
#include "umlwidgetcolorpage.h"

//kde class includes
#include <klineedit.h>
#include <kpagedialog.h>
#include <kfontdialog.h>
#include <ktextedit.h>

//forward declarations
class UMLView;
class ActivityWidget;
class QLabel;
class QGroupBox;
class QRadioButton;

/**
 *   Displays the properties for a @ref ActivityWidget
 *
 *   @author   Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ActivityDialog : public KPageDialog
{
    Q_OBJECT

public:
    /**
     *   Constructor
     */
    ActivityDialog( UMLView * pView, ActivityWidget * pWidget );

    /**
     *   Returns whether changes were made.
     */
    bool getChangesMade() {
        return m_bChangesMade;
    }

protected slots:

    /**
     *   Entered when OK button pressed.
     */
    void slotOk();

    /**
     *   Entered when Apply button pressed.
     */
    void slotApply();

    void slotShowActivityParameter();
    void slotHideActivityParameter();

protected:

    /**
     *   Sets up the pages of the dialog.
     */
    void setupPages();

    /**
     *   Sets up the general page of the dialog.
     */
    void setupGeneralPage();

    /**
     *   Sets up the color page.
     */
    void setupColorPage();

    /**
     *   Sets up the font selection page.
     */
    void setupFontPage();

    /**
     *   Show the Activity Parameter entry text.
     */
    void showParameterActivity();

    /**
     *     Applys changes to the given page.
     */
    void applyPage( KPageWidgetItem * );

    /**
     *   Font chooser widget for font page.
     */
    KFontChooser * m_pChooser;

    /**
     *   Color page
     */
    UMLWidgetColorPage * m_pColorPage;

    /**
     *   The widget to represent.
     */
    ActivityWidget * m_pActivityWidget;

    /**
     *   The diagram the widget is on.
     */
    UMLView * m_pView;

    /**
     *   Holds whether changes in the dialog have been made.
     */
    bool m_bChangesMade;

    struct GeneralPageWidgets {
        QLabel * nameL, * typeL , * preL , * postL;
        KLineEdit * nameLE, * typeLE , * preLE , * postLE;
        KTextEdit * docTE;
        QRadioButton * NormalRB, * ParamRB, * InvokRB; // this is to implement the parameter node activity or an invok action
        QGroupBox * docGB, * generalGB;
    }
    m_GenPageWidgets;
    KPageWidgetItem *pageItemGeneral,*pageItemFont,*pageItemColor;
};

#endif
