/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#ifndef UMLVIEWDIALOG_H
#define UMLVIEWDIALOG_H

// qt includes

// kde includes
#include <kpagedialog.h>

// app includes
#include "optionstate.h"
#include "ui_diagrampropertiespage.h"

class ClassOptionsPage;
class UMLView;
class UMLWidgetStylePage;
class KFontChooser;

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class DiagramPropertiesPage : public QWidget, public Ui::DiagramPropertiesPage
{
public:
    DiagramPropertiesPage(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

class UMLViewDialog : public KPageDialog
{
    Q_OBJECT
public:
    UMLViewDialog(QWidget * pParent, UMLView * pView);
    ~UMLViewDialog();

protected:
    void setupPages();

    void setupDiagramPropertiesPage();
    void setupClassPage();
    void setupStylePage();
    void setupFontPage();

    void applyPage( KPageWidgetItem* );

    void checkName();

    UMLView*               m_pView;   ///< the view to represent
    Settings::OptionState  m_options;
    KFontChooser*          m_pChooser;
    DiagramPropertiesPage* m_diagramProperties;
    ClassOptionsPage*      m_pOptionsPage;
    UMLWidgetStylePage*    m_pStylePage;
    KPageWidgetItem*       m_pageStyleItem;
    KPageWidgetItem*       m_pageFontItem;
    KPageWidgetItem*       m_pageDisplayItem;
    KPageWidgetItem*       m_pageGeneralItem;

public slots:
    void slotOk();
    void slotApply();
};

#endif
