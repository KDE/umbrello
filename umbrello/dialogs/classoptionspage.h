/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/


#ifndef CLASSOPTIONSPAGE_H
#define CLASSOPTIONSPAGE_H

#include "optionstate.h"

#include <QtGui/QWidget>

class ClassifierWidget;
class QCheckBox;
class QGroupBox;

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
class ClassOptionsPage : public QWidget
{
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

    void updateUMLWidget();

    void setWidget( ClassifierWidget * pWidget ) {
        m_pWidget = pWidget;
    }

protected:

    void init();

    void setupPage();

    void setupClassPageOption();

    void updateWidget();

    void updateOptionState();

    //GUI widgets
    QGroupBox * m_pVisibilityGB;
    QCheckBox * m_pShowVisibilityCB, * m_pShowAttSigCB;
    QCheckBox * m_pShowOpSigCB, * m_pShowAttsCB, * m_pShowOpsCB;
    QCheckBox * m_pShowStereotypeCB, * m_pShowPackageCB, * m_pShowPublicOnlyCB;
    QCheckBox * m_pShowAttribAssocsCB;
    QCheckBox * m_pDrawAsCircleCB;

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
