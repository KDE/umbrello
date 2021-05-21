/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef STATEDIALOG_H
#define STATEDIALOG_H

// kde class includes
#include "multipagedialogbase.h"

//forward declarations
class ActivityPage;
class DocumentationWidget;
class SelectDiagramWidget;
class StateWidget;
class KComboBox;
class KLineEdit;
class QGroupBox;
class QLabel;

/**
 * Displays the properties for a @ref StateWidget
 * @author   Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class StateDialog : public MultiPageDialogBase
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
        SelectDiagramWidget *diagramLinkWidget;
        KLineEdit * nameLE, * typeLE;
        DocumentationWidget *docWidget;
        QGroupBox * generalGB;
    }
    m_GenPageWidgets;

    KPageWidgetItem *pageStyle, *pageActivity, *pageFont, *pageGeneral;
};

#endif
