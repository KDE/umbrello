/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/


#ifndef CLASSOPTIONSPAGE_H
#define CLASSOPTIONSPAGE_H

#include "optionstate.h"

#include <QWidget>

class ClassifierWidget;
class EntityWidget;
class UMLScene;
class KComboBox;
class QCheckBox;
class QGroupBox;
class QLabel;

/**
 * A dialog page to display options for class related options.
 * This dialog is either embedded into @ref SettingsDialog,
 * @ref UMLViewDialog and @ref ClassPropertiesDialog
 *
 * @short A dialog page to display the class related options.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see ClassPropertiesDialog
 * @see SettingsDialog
 * @see UMLViewDialog
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ClassOptionsPage : public QWidget
{
public:
    ClassOptionsPage(QWidget* pParent, ClassifierWidget* pWidget);
    ClassOptionsPage(QWidget* pParent, UMLScene *scene);
    ClassOptionsPage(QWidget* pParent, Settings::OptionState *options, bool isDiagram=true);
    ClassOptionsPage(QWidget* pParent, EntityWidget* widget);
    virtual ~ClassOptionsPage();

    void setDefaults();
    void apply();

    void setWidget(ClassifierWidget * pWidget);

protected:
    void init();

    void setupPage();
    void setupPageFromScene();
    void setupClassPageOption();
    void setupPageFromEntityWidget();

    void applyWidget();
    void applyOptionState();
    void applyScene();
    void applyEntityWidget();

    KComboBox * createShowStereotypeCB(QGroupBox * grpBox);
    void insertAttribScope(const QString& type, int index = -1);
    void insertOperationScope(const QString& type, int index = -1);

    //GUI widgets
    QGroupBox * m_visibilityGB;
    QCheckBox * m_showVisibilityCB, * m_showAttSigCB;
    QCheckBox * m_showOpSigCB, * m_showAttsCB, * m_showOpsCB;
    KComboBox * m_showStereotypeCB;
    QCheckBox * m_showPackageCB, * m_showPublicOnlyCB;
    QCheckBox * m_showAttribAssocsCB;
    QCheckBox * m_showDocumentationCB;
    QCheckBox * m_drawAsCircleCB;

    QGroupBox * m_scopeGB;
    QLabel * m_attributeLabel;
    QLabel * m_operationLabel;
    KComboBox * m_attribScopeCB;
    KComboBox * m_operationScopeCB;

    ClassifierWidget* m_pWidget; ///< The classifier widget to represent in the dialog page.
    EntityWidget* m_entityWidget; ///< The entity widget to represent in the dialog page.
    UMLScene* m_scene; ///< The scene to represent in the dialog page.
    Settings::OptionState *m_options; ///< The OptionState structure to represent in the dialog page.
    bool m_isDiagram; ///< Flag indicating that page is for diagram property dialog
};
#endif
