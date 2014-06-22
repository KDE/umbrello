/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/
#ifndef UMLVIEWDIALOG_H
#define UMLVIEWDIALOG_H

// qt includes

// kde includes

// app includes
#include "dialogbase.h"
#include "optionstate.h"

class ClassOptionsPage;
class DiagramPropertiesPage;
class UMLScene;
class UMLView;
class UMLWidgetStylePage;
class KFontChooser;

/**
 * @author Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class UMLViewDialog : public DialogBase
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
    void setupFontPage();

    void applyPage(KPageWidgetItem*);

    void checkName();

    UMLScene*              m_pScene;  ///< the scene to represent
    Settings::OptionState  m_options;
    KFontChooser*          m_pChooser;
    DiagramPropertiesPage* m_diagramPropertiesPage;
    ClassOptionsPage*      m_pOptionsPage;
    UMLWidgetStylePage*    m_pStylePage;
    KPageWidgetItem*       m_pageStyleItem;
    KPageWidgetItem*       m_pageFontItem;
    KPageWidgetItem*       m_pageDisplayItem;
    KPageWidgetItem*       m_pageDiagramItem;

public slots:
    void slotOk();
    void slotApply();
};

#endif
