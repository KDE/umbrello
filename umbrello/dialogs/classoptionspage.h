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

#include <QWidget>

class ClassifierWidget;
class KComboBox;
class QCheckBox;
class QGroupBox;
class QLabel;

/**
 * A dialog page to display options for class related options.
 * This dialog is either embedded into @ref SettingsDlg,
 * @ref UMLViewDialog and @ref ClassPropDlg
 *
 * @short A dialog page to display the class related options.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see ClassPropDlg
 * @see SettingsDlg
 * @see UMLViewDialog
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassOptionsPage : public QWidget
{
public:
    ClassOptionsPage(QWidget* pParent, ClassifierWidget* pWidget);
    ClassOptionsPage(QWidget* pParent, Settings::OptionState *options, bool isDiagram=true);
    virtual ~ClassOptionsPage();

    void setDefaults();
    void apply();

    void setWidget( ClassifierWidget * pWidget );

protected:
    void init();

    void setupPage();
    void setupClassPageOption();

    void applyWidget();
    void applyOptionState();

    void insertAttribScope( const QString& type, int index = -1 );
    void insertOperationScope( const QString& type, int index = -1 );

    //GUI widgets
    QGroupBox * m_visibilityGB;
    QCheckBox * m_showVisibilityCB, * m_showAttSigCB;
    QCheckBox * m_showOpSigCB, * m_showAttsCB, * m_showOpsCB;
    QCheckBox * m_showStereotypeCB, * m_showPackageCB, * m_showPublicOnlyCB;
    QCheckBox * m_showAttribAssocsCB;
    QCheckBox * m_drawAsCircleCB;

    QGroupBox * m_scopeGB;
    QLabel * m_attributeLabel;
    QLabel * m_operationLabel;
    KComboBox * m_attribScopeCB;
    KComboBox * m_operationScopeCB;

    ClassifierWidget* m_pWidget; ///< The classifier widget to represent in the dialog page.
    Settings::OptionState *m_options; ///< The OptionState structure to represent in the dialog page.
    bool m_isDiagram; ///< Flag indicating that page is for diagram property dialog
};
#endif
