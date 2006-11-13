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

#ifndef UMLVIEWDIALOG_H
#define UMLVIEWDIALOG_H
//kde includes
#include <kdialogbase.h>
//app includes
#include "classoptionspage.h"
#include "umlwidgetcolorpage.h"
#include "../optionstate.h"

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLView;
class ClassWidget;

class QCheckBox;
class QLabel;
class QLineEdit;
class QGroupBox;
class QTextEdit;
class QSpinBox;
class KFontChooser;
class DiagramPropertiesPage;


class UMLViewDialog : public KDialogBase {
    Q_OBJECT
public:
    /**
    *   Constructor
    */
    UMLViewDialog( QWidget * pParent, UMLView * pView );

    /**
    *   Deconstructor
    */
    ~UMLViewDialog();
protected:
    enum Page
    {
        General = 0,
        Color,
        Font,
        Class
    };

    /**
    *   Sets up the dialog pages.
    */
    void setupPages();

    /**
    *   Sets up the general Diagram Properties Page
    */
    void setupDiagramPropertiesPage();

    /**
    *   Sets up the Class page
    */
    void setupClassPage();

    /**
    *   Sets up the color page.
    */
    void setupColorPage();

    /**
    *   Sets up font page.
    */
    void setupFontPage();

    /**
    *   Applys the properties of the given page.
    */
    void applyPage( Page page );

    /**
    *   Checks whether the name is unique and sets it if it is.
    */
    void checkName();

    /**
    *   The view to represent.
    */
    UMLView * m_pView;

    Settings::OptionState m_options;

    KFontChooser * m_pChooser;
    DiagramPropertiesPage *m_diagramProperties;
    ClassOptionsPage * m_pOptionsPage;
    UMLWidgetColorPage * m_pColorPage;

    //GUI widgets
    QLabel * m_pNameL, * m_pSpinXL, * m_pSpinYL;
    QLineEdit * m_pNameLE;
    QGroupBox * m_pDocGB, * m_pValuesGB;
    QCheckBox * m_pSnapToGridCB, * m_pShowSnapCB;
    QTextEdit * m_pDocTE;
    QSpinBox * m_pSnapXSB, * m_pSnapYSB;
    QSpinBox * m_pLineWidthSB;
public slots:
    void slotOk();
    void slotApply();
};

#endif
