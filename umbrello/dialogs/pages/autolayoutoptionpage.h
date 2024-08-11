/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef AUTOLAYOUTOPTIONPAGE_H
#define AUTOLAYOUTOPTIONPAGE_H

#include "dialogpagebase.h"
#include "ui_autolayoutoptionpage.h"

/**
 * A dialog page to display auto layouts options
 * 
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 *
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class AutoLayoutOptionPage : public DialogPageBase, private Ui::AutoLayoutOptionPage
{
    Q_OBJECT
public:
    explicit AutoLayoutOptionPage(QWidget *parent = nullptr);
    virtual ~AutoLayoutOptionPage();

    void setDefaults();
    void apply();

Q_SIGNALS:
    void applyClicked();

protected Q_SLOTS:
    void slotAutoDotPathCBClicked(bool value);

};
#endif
