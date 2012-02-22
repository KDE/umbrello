/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLWIDGETCOLORPAGE_H
#define UMLWIDGETCOLORPAGE_H

#include "optionstate.h"

#include <QtGui/QWidget>

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLWidget;
class KColorButton;
class QLabel;
class QPushButton;
class QCheckBox;
class QGroupBox;

class UMLWidgetColorPage : public QWidget
{
    Q_OBJECT
public:

    UMLWidgetColorPage( QWidget * pParent, UMLWidget * pWidget );
    UMLWidgetColorPage( QWidget * pParent, Settings::OptionState *options );

    virtual ~UMLWidgetColorPage();

    void updateUMLWidget();

public slots:

    void slotLineButtonClicked();
    void slotFillButtonClicked();
    void slotBackgroundButtonClicked();
    void slotGridDotButtonClicked();

protected:

    UMLWidget * m_pUMLWidget;  ///< the widget to set the color for

    Settings::OptionState * m_options;

    //GUI widgets
    QGroupBox * m_pColorGB;
    QLabel * m_pLineColorL;
    QLabel * m_pFillColorL;
    QLabel * m_BackgroundColorL;
    QLabel * m_GridDotColorL;
    QCheckBox * m_pUseFillColorCB;
    QPushButton * m_pLineDefaultB;
    QPushButton * m_pFillDefaultB;
    QPushButton * m_BackgroundDefaultB;
    QPushButton * m_GridDotDefaultB;
    KColorButton * m_pLineColorB;
    KColorButton * m_pFillColorB;
    KColorButton * m_BackgroundColorB;
    KColorButton * m_GridDotColorB;

private:
    void init();

};

#endif
