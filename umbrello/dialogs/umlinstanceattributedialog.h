/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016-2017                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLINSTANCEATTRIBUTEDIALOG_H
#define UMLINSTANCEATTRIBUTEDIALOG_H

#include <QWidget>
#include "singlepagedialogbase.h"
#include "attribute.h"

namespace Ui {
class UMLInstanceAttributeDialog;
}

class UMLInstanceAttributeDialog : public SinglePageDialogBase
{
    Q_OBJECT

public:
    explicit UMLInstanceAttributeDialog(QWidget *parent, UMLAttribute *pAttribute);
    ~UMLInstanceAttributeDialog();

private:
    Ui::UMLInstanceAttributeDialog *ui;
    UMLAttribute *m_pAttribute;

protected:
    bool apply();

public slots:
    void slotNameChanged(const QString &text);
};

#endif // UMLINSTANCEATTRIBUTEDIALOG_H
