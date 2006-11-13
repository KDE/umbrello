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


#ifndef CLASSOPTIONSPAGE_H
#define CLASSOPTIONSPAGE_H

#include <qwidget.h>
#include <qgroupbox.h>
#include <qcheckbox.h>

#include "../optionstate.h"

class ClassifierWidget;

/**
 * A dialog page to display options for a @ref UMLWidget and its
 * children.  This is not normally called by you.  It is used by
 * the @ref ClassPropDlg.
 *
 * @short A dialog page to display the options for a UMLWidget.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see ClassPropDlg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassOptionsPage : public QWidget {
public:
    /**
     * Constructor - observe and modify a Widget
     */
    ClassOptionsPage(QWidget* pParent, ClassifierWidget* pWidget);

    /**
     * Constructor - observe and modify an OptionState structure
     */
    ClassOptionsPage(QWidget* pParent, Settings::OptionState *options );

    /**
     * destructor
     */
    virtual ~ClassOptionsPage();

    /**
     * Updates the widget with the dialog page properties.
     */
    void updateUMLWidget();

    /**
     * Sets the widget to be used.  Only used by @ref UMLView to set
     * settings of a widget.
     */
    void setWidget( ClassifierWidget * pWidget ) {
        m_pWidget = pWidget;
    }

protected:
    /**
     * Initialize optional items
     */
    void init();

    /**
     * Creates the page with the correct options for the class/interface
     */
    void setupPage();

    /**
     * Creates the page based on the OptionState
     */
    void setupClassPageOption();

    /**
     * Sets the ClassifierWidget's properties to those selected in this dialog page.
     */
    void updateWidget();

    /**
     * Sets the OptionState to the values selected in this dialog page.
     */
    void updateOptionState();

    //GUI widgets
    QGroupBox * m_pVisibilityGB;
    QCheckBox * m_pShowVisibilityCB, * m_pShowAttSigCB;
    QCheckBox * m_pShowOpSigCB, * m_pShowAttsCB, * m_pShowOpsCB;
    QCheckBox * m_pShowStereotypeCB, * m_pShowPackageCB;
    QCheckBox* m_pDrawAsCircleCB;

    /**
     * The classifier widget to represent in the dialog page.
     */
    ClassifierWidget* m_pWidget;
    /**
     * The OptionState structure to represent in the dialog page.
     */
    Settings::OptionState *m_options;
};
#endif
