/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#ifndef UMLVIEWDIALOG_H
#define UMLVIEWDIALOG_H

// qt includes

// kde includes
#include <kpagedialog.h>

// app includes
#include "classoptionspage.h"
#include "umlwidgetcolorpage.h"
#include "optionstate.h"
#include "ui_diagrampropertiespage.h"

class UMLView;
class KFontChooser;

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class DiagramPropertiesPage : public QWidget, public Ui::DiagramPropertiesPage
{
public:
    DiagramPropertiesPage( QWidget *parent ) : QWidget( parent ) {
        setupUi( this );
  }
};


class UMLViewDialog : public KPageDialog
{
    Q_OBJECT
public:
    /**
     * Constructor
     */
    UMLViewDialog( QWidget * pParent, UMLView * pView );

    /**
     * Deconstructor
     */
    ~UMLViewDialog();

protected:

    /**
     * Sets up the dialog pages.
     */
    void setupPages();

    /**
     * Sets up the general Diagram Properties Page
     */
    void setupDiagramPropertiesPage();

    /**
     * Sets up the Class page
     */
    void setupClassPage();

    /**
     * Sets up the color page.
     */
    void setupColorPage();

    /**
    *   Sets up font page.
    */
    void setupFontPage();

    /**
     * Applys the properties of the given page.
     */
    void applyPage( KPageWidgetItem* );

    /**
     * Checks whether the name is unique and sets it if it is.
     */
    void checkName();

    /**
     * The view to represent.
     */
    UMLView * m_pView;

    Settings::OptionState m_options;

    KFontChooser * m_pChooser;
    DiagramPropertiesPage * m_diagramProperties;
    ClassOptionsPage * m_pOptionsPage;
    UMLWidgetColorPage * m_pColorPage;
    KPageWidgetItem *m_pageColorItem, *m_pageFontItem, *m_pageDisplayItem, *m_pageGeneralItem;

public slots:
    void slotOk();
    void slotApply();
};

#endif
