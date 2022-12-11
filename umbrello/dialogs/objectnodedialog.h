/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef OBJECTNODEDIALOG_H
#define OBJECTNODEDIALOG_H

// local class includes
#include "multipagedialogbase.h"

//forward declarations
class DocumentationWidget;
class UMLView;
class ObjectNodeWidget;
class QGroupBox;
class QLabel;
class QRadioButton;
class QLineEdit;

/**
 *   Displays the properties for a @ref ObjectNodeWidget
 *
 *   @author   Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ObjectNodeDialog : public MultiPageDialogBase
{
    Q_OBJECT
public:
    ObjectNodeDialog(QWidget *parent, ObjectNodeWidget * pWidget);

    bool getChangesMade() const {
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

    void applyPage(KPageWidgetItem *);

    ObjectNodeWidget *m_pObjectNodeWidget;  ///< the widget to represent
    UMLView          *m_pView;              ///< the diagram the widget is on
    bool              m_bChangesMade;       ///< holds whether changes in the dialog have been made

    struct GeneralPageWidgets {
        QLabel * nameL, * typeL, * stateL;
        QLineEdit * nameLE, * typeLE, * stateLE;
        QRadioButton * bufferRB, * dataRB, * flowRB;
        QGroupBox * generalGB;
        DocumentationWidget *docWidget;
    }
    m_GenPageWidgets;

    KPageWidgetItem * pageItemGeneral, * pageItemFont, * pageItemStyle;
};

#endif
