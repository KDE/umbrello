/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef OBJECTNODEDIALOG_H
#define OBJECTNODEDIALOG_H

// local class includes
#include "dialogbase.h"

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
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class ObjectNodeDialog : public DialogBase
{
    Q_OBJECT
public:
    ObjectNodeDialog( UMLView * pView, ObjectNodeWidget * pWidget );

    bool getChangesMade() {
        return m_bChangesMade;
    }

protected slots:
    void slotOk();

    void slotApply();

    void slotShowState();
    void slotHideState();

protected:
    void setupPages();

    void setupGeneralPage();

    void showState();

    void applyPage( KPageWidgetItem * );

    ObjectNodeWidget *m_pObjectNodeWidget;  ///< the widget to represent
    UMLView          *m_pView;              ///< the diagram the widget is on
    bool              m_bChangesMade;       ///< holds whether changes in the dialog have been made

    struct GeneralPageWidgets {
        QLabel * nameL, * typeL , * stateL;
        KLineEdit * nameLE, * typeLE , * stateLE;
        KTextEdit * docMLE;
        QRadioButton * bufferRB, * dataRB, * flowRB;
        QGroupBox * docGB, * generalGB;
    }
    m_GenPageWidgets;

    KPageWidgetItem * pageItemGeneral, * pageItemFont, * pageItemStyle;
};

#endif
