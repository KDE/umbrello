/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef MESSAGEWIDGETPROPERTIESDIALOG_H
#define MESSAGEWIDGETPROPERTIESDIALOG_H

#include "multipagedialogbase.h"

class MessageWidget;

/**
 * Based off of AssociationPropertiesDialog class
 * @author Brian Thomas <Brian.A.Thomas@gsfc.nasa.gov>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class MessageWidgetPropertiesDialog : public MultiPageDialogBase
{
    Q_OBJECT
public:
    MessageWidgetPropertiesDialog(QWidget *parent, MessageWidget *widget);
    ~MessageWidgetPropertiesDialog();

protected:
    void setupPages();

protected slots:
    void slotOk();
    void slotApply();

private:
    MessageWidget  *m_widget;

};

#endif
