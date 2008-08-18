/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef OBJECTNODEDIALOG_H
#define OBJECTNODEDIALOG_H

// local class includes
#include "umlwidgetcolorpage.h"

// kde class includes
#include <kpagedialog.h>
#include <kfontdialog.h>

//forward declarations
class UMLView;
class ObjectNodeWidget;
class QGroupBox;
class QLabel;
class QRadioButton;
class KLineEdit;
class KTextEdit;

/**
 *   Displays the properties for a @ref ObjectNodeWidget
 *
 *   @author   Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class ObjectNodeDialog : public KPageDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    ObjectNodeDialog( UMLView * pView, ObjectNodeWidget * pWidget );

    /**
     * Returns whether changes were made.
     */
    bool getChangesMade() {
        return m_bChangesMade;
    }

protected slots:
    /**
     * Entered when OK button pressed.
     */
    void slotOk();

    /**
     * Entered when Apply button pressed.
     */
    void slotApply();

    void slotShowState();
    void slotHideState();

protected:
    /**
     * Sets up the pages of the dialog.
     */
    void setupPages();

    /**
     * Sets up the general page of the dialog.
     */
    void setupGeneralPage();

    /**
     * Sets up the color page.
     */
    void setupColorPage();

    /**
     * Sets up the font selection page.
     */
    void setupFontPage();

    /**
     * Show the State entry text.
     */
    void showState();

    /**
     * Applys changes to the given page.
     */
    void applyPage( KPageWidgetItem * );

    /**
     * Font chooser widget for font page.
     */
    KFontChooser * m_pChooser;

    /**
     * Color page
     */
    UMLWidgetColorPage * m_pColorPage;

    /**
     * The widget to represent.
     */
    ObjectNodeWidget * m_pObjectNodeWidget;

    /**
     * The diagram the widget is on.
     */
    UMLView * m_pView;

    /**
     * Holds whether changes in the dialog have been made.
     */
    bool m_bChangesMade;

    struct GeneralPageWidgets {
        QLabel * nameL, * typeL , * stateL;
        KLineEdit * nameLE, * typeLE , * stateLE;
        KTextEdit * docMLE;
        QRadioButton * bufferRB, * dataRB, * flowRB;
        QGroupBox * docGB, * generalGB;
    }
    m_GenPageWidgets;

    KPageWidgetItem * pageItemGeneral, * pageItemFont, * pageItemColor;
};

#endif
