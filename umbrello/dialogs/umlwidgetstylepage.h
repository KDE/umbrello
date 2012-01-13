/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLWIDGETSTYLEPAGE_H
#define UMLWIDGETSTYLEPAGE_H

#include "optionstate.h"

#include <QtGui/QWidget>

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class WidgetBase;
class KColorButton;
class KIntSpinBox;
class QLabel;
class QPushButton;
class QCheckBox;
class QGroupBox;

class UMLWidgetStylePage : public QWidget
{
    Q_OBJECT
public:

    UMLWidgetStylePage( QWidget * pParent, WidgetBase * pWidget );
    UMLWidgetStylePage( QWidget * pParent, Settings::OptionState *options );

    virtual ~UMLWidgetStylePage();

    void updateUMLWidget();

public slots:

    void slotTextButtonClicked();
    void slotLineButtonClicked();
    void slotFillButtonClicked();
    void slotBackgroundButtonClicked();
    void slotGridDotButtonClicked();
    void slotLineWidthButtonClicked();

protected:

    WidgetBase * m_pUMLWidget;  ///< the widget to set the style for

    Settings::OptionState * m_options;

    //GUI widgets
    QGroupBox * m_pColorGB;
    QGroupBox * m_pStyleGB;
    QLabel * m_pTextColorL;
    QLabel * m_pLineColorL;
    QLabel * m_pFillColorL;
    QLabel * m_BackgroundColorL;
    QLabel * m_GridDotColorL;
    QLabel * m_lineWidthL;
    QCheckBox * m_pUseFillColorCB;
    QPushButton * m_pTextDefaultB;
    QPushButton * m_pLineDefaultB;
    QPushButton * m_pFillDefaultB;
    QPushButton * m_BackgroundDefaultB;
    QPushButton * m_GridDotDefaultB;
    QPushButton * m_lineWidthDefaultB;
    KColorButton * m_pTextColorB;
    KColorButton * m_pLineColorB;
    KColorButton * m_pFillColorB;
    KColorButton * m_BackgroundColorB;
    KColorButton * m_GridDotColorB;
    KIntSpinBox * m_lineWidthB;

private:
    void init();

};

#endif
