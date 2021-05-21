/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ACTIVITYDIALOG_H
#define ACTIVITYDIALOG_H

#include <multipagedialogbase.h>

//forward declarations
class ActivityWidget;
class DocumentationWidget;
class KLineEdit;
class QLabel;
class QGroupBox;
class QRadioButton;
class UMLView;

/**
 * Displays the properties for a @ref ActivityWidget
 *
 * @author   Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ActivityDialog : public MultiPageDialogBase
{
    Q_OBJECT
public:
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

    void showParameterActivity();

    void applyPage(KPageWidgetItem *);

    ActivityWidget *m_pActivityWidget;  ///< the widget to represent
    UMLView        *m_pView;  ///< the diagram the widget is on
    bool            m_bChangesMade;   ///< holds whether changes in the dialog have been made

    struct GeneralPageWidgets {
        QLabel * nameL, * typeL, * preL, * postL;
        KLineEdit * nameLE, * typeLE, * preLE, * postLE;
        DocumentationWidget *docWidget;
        QRadioButton * NormalRB, * ParamRB, * InvokRB; // this is to implement the parameter node activity or an invok action
        QGroupBox * generalGB;
    }
    m_GenPageWidgets;

    KPageWidgetItem *pageItemGeneral, *pageItemFont, *pageItemStyle;
};

#endif
