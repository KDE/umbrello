/*
    SPDX-License-Identifier: GPL-3.0-or-later

    SPDX-FileCopyrightText: 2015 Tzvetelin Katchov <katchov@gmail.com>
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLENUMLITERALDIALOG_H
#define UMLENUMLITERALDIALOG_H

#include "singlepagedialogbase.h"

class QGroupBox;
class QLabel;
class UMLEnumLiteral;
class KLineEdit;

/**
 * @author Tzvetelin Katchov
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLEnumLiteralDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:
    UMLEnumLiteralDialog(QWidget * pParent, UMLEnumLiteral * pEnumLiteral);
    ~UMLEnumLiteralDialog();

protected:

    void setupDialog();

    bool apply();

    /**
     *   The EnumLiteral to represent
     */
    UMLEnumLiteral * m_pEnumLiteral;

    //GUI Widgets
    QGroupBox * m_pValuesGB;
    QLabel * m_pNameL, * m_pValueL;
    KLineEdit * m_pNameLE, * m_pValueLE;

public slots:
    void slotNameChanged(const QString &);
};

#endif
