/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ACTIVITYDIALOG_H
#define ACTIVITYDIALOG_H

//qt class includes
#include <qlabel.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qgroupbox.h>

//kde class includes
#include <kdialogbase.h>
#include <kfontdialog.h>

//local class includes
#include "umlwidgetcolorpage.h"

//forward declarations
class UMLView;
class ActivityWidget;

/**
 *   Displays the properties for a @ref ActivityWidget
 *
 *   @author   Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class ActivityDialog : public KDialogBase {
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

protected:

    enum Page
    {
        GeneralPage = 0,
        ColorPage,
        FontPage
    };

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
    *     Applys changes to the given page.
    */
    void applyPage( Page page );

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
        QLabel * nameL, * typeL;
        QLineEdit * nameLE, * typeLE;
        QMultiLineEdit * docMLE;

        QGroupBox * docGB, * generalGB;
    }
    m_GenPageWidgets;
};

#endif
