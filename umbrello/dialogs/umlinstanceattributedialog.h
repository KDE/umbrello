/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016-2021                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLINSTANCEATTRIBUTEDIALOG_H
#define UMLINSTANCEATTRIBUTEDIALOG_H

#include <QWidget>
#include "singlepagedialogbase.h"
#include "instanceattribute.h"

namespace Ui {
class UMLInstanceAttributeDialog;
}

class UMLInstanceAttributeDialog : public SinglePageDialogBase
{
    Q_OBJECT

public:
    explicit UMLInstanceAttributeDialog(QWidget *parent, UMLInstanceAttribute *pInstanceAttr);
    ~UMLInstanceAttributeDialog();

private:
    Ui::UMLInstanceAttributeDialog *ui;
    UMLInstanceAttribute *m_pInstanceAttr;

protected:
    bool apply();
};

#endif // UMLINSTANCEATTRIBUTEDIALOG_H
