/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLWIDGETSTYLEPAGE_H
#define UMLWIDGETSTYLEPAGE_H

#include "optionstate.h"

#include <QWidget>

class SelectLayoutTypeWidget;
class UMLScene;
class WidgetBase;
class KColorButton;
#if QT_VERSION < 0x050000
class KIntSpinBox;
#endif
class QLabel;
class QPushButton;
class QCheckBox;
class QGroupBox;
#if QT_VERSION >= 0x050000
class QSpinBox;
#endif

/**
 * @author Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLWidgetStylePage : public QWidget
{
    Q_OBJECT
public:

    UMLWidgetStylePage(QWidget * pParent, WidgetBase * pWidget);
    UMLWidgetStylePage(QWidget * pParent, Settings::OptionState *options);
    UMLWidgetStylePage(QWidget * pParent, UMLScene *scene);

    virtual ~UMLWidgetStylePage();

    void apply();

public slots:

    void slotTextButtonClicked();
    void slotLineButtonClicked();
    void slotFillButtonClicked();
    void slotBackgroundButtonClicked();
    void slotGridDotButtonClicked();
    void slotLineWidthButtonClicked();

protected:

    WidgetBase * m_pUMLWidget;  ///< the widget to set the style for
    UMLScene *m_scene;

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
    SelectLayoutTypeWidget *m_layoutTypeW;

#if QT_VERSION >= 0x050000
    QSpinBox * m_lineWidthB;
#else
    KIntSpinBox * m_lineWidthB;
#endif

private:
    void init();

};

#endif
