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
#include "umlwidgetstylepage.h"

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
    ActivityDialog(QWidget * parent, ActivityWidget * pWidget);

    bool getChangesMade() {
        return m_bChangesMade;
    }

protected slots:

    void slotOk();

    void slotApply();

    void slotShowActivityParameter();
    void slotHideActivityParameter();

protected:

    void setupPages();

    void setupGeneralPage();

    void setupStylePage();

    void setupFontPage();

    void showParameterActivity();

    void applyPage( KPageWidgetItem * );

    /**
     *   Font chooser widget for font page.
     */
    KFontChooser * m_pChooser;

    /**
     *   style page
     */
    UMLWidgetStylePage * m_pStylePage;

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
    KPageWidgetItem *pageItemGeneral,*pageItemFont,*pageItemStyle;
};

#endif
