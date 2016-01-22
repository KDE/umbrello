/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2015      Tzvetelin Katchov <katchov@gmail.com>         *
 *   copyright (C) 2002-2015                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLENUMLITERALDIALOG_H
#define UMLENUMLITERALDIALOG_H

#include "singlepagedialogbase.h"

class QGroupBox;
class QLabel;
class UMLEnumLiteral;
class KLineEdit;

/**
 * @author Tzvetelin Katchov
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
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
