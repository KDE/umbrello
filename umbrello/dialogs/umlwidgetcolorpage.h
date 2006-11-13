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

#ifndef UMLWIDGETCOLORPAGE_H
#define UMLWIDGETCOLORPAGE_H

#include <qwidget.h>
#include "../optionstate.h"

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLWidget;
class UMLView;
class KColorButton;
class QLabel;
class QPushButton;
class QCheckBox;
class QGroupBox;

class UMLWidgetColorPage : public QWidget {
    Q_OBJECT
public:

    /**
    *   Constructor - Observe a UMLWidget
    */
    UMLWidgetColorPage( QWidget * pParent, UMLWidget * pWidget );

    /**
    *   Constructor - Observe an OptionState structure
    */
    UMLWidgetColorPage( QWidget * pParent, Settings::OptionState *options );

    /**
    *   destructor
    */
    virtual ~UMLWidgetColorPage();

    /**
    *   Updates the @ref UMLWidget with the dialog properties.
    */
    void updateUMLWidget();

public slots:
    /**
    *   Sets the default line color when default line button
    *   clicked.
    */
    void slotLineButtonClicked();

    /**
    *   Sets the default fill color when default fill button
    *   clicked.
    */
    void slotFillButtonClicked();

protected:
    /**
    *   The widget to set the color for.
    */
    UMLWidget * m_pUMLWidget;

    Settings::OptionState *m_options;

    //GUI widgets
    QGroupBox * m_pColorGB;
    QLabel * m_pLineColorL, * m_pFillColorL;
    QCheckBox * m_pUseFillColorCB;
    QPushButton * m_pLineDefaultB, * m_pFillDefaultB;
    KColorButton * m_pLineColorB, * m_pFillColorB;
private:
    void init();

};

#endif
