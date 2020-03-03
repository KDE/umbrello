/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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
    explicit AutoLayoutOptionPage(QWidget* parent = 0);
    virtual ~AutoLayoutOptionPage();

    void setDefaults();
    void apply();

signals:
    void applyClicked();

protected slots:
    void slotAutoDotPathCBClicked(bool value);

};
#endif
