/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
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
class NewUMLRectWidget;
class KColorButton;
class QLabel;
class QPushButton;
class QCheckBox;
class QGroupBox;

class UMLWidgetColorPage : public QWidget
{
    Q_OBJECT
public:

    /**
    *   Constructor - Observe a NewUMLRectWidget
    */
    UMLWidgetColorPage( QWidget * pParent, NewUMLRectWidget * pWidget );

    /**
     *   Constructor - Observe an OptionState structure
     */
    UMLWidgetColorPage( QWidget * pParent, Settings::OptionState *options );

    /**
     *   destructor
     */
    virtual ~UMLWidgetColorPage();

    void updateUMLWidget();

public slots:

    void slotLineButtonClicked();

    void slotFillButtonClicked();

protected:

    /**
     *   The widget to set the color for.
     */
    NewUMLRectWidget * m_pUMLWidget;

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
