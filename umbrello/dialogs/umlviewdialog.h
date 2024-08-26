/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/
#ifndef UMLVIEWDIALOG_H
#define UMLVIEWDIALOG_H

// qt includes

// kde includes

// app includes
#include "multipagedialogbase.h"
#include "optionstate.h"

class ClassOptionsPage;
class DiagramPropertiesPage;
class UMLScene;
class UMLView;
class UMLWidgetStylePage;
class QFontDialog;

/**
 * @author Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLViewDialog : public MultiPageDialogBase
{
    Q_OBJECT
public:
    UMLViewDialog(QWidget * pParent, UMLScene * pScene);
    ~UMLViewDialog();

    virtual void apply();

protected:
    void setupPages();

    void setupDiagramPropertiesPage();
    void setupDisplayPage();
    void setupStylePage();

    void applyPage(KPageWidgetItem*);

    void checkName();

    UMLScene*              m_pScene = nullptr;  ///< the scene to represent
    DiagramPropertiesPage* m_diagramPropertiesPage = nullptr;
    ClassOptionsPage*      m_pOptionsPage = nullptr;
    KPageWidgetItem*       m_pageStyleItem = nullptr;
    KPageWidgetItem*       m_pageFontItem = nullptr;
    KPageWidgetItem*       m_pageDisplayItem = nullptr;
    KPageWidgetItem*       m_pageDiagramItem = nullptr;

public Q_SLOTS:
    void slotOk();
    void slotApply();
};

#endif
