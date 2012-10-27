/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef AUTOLAYOUTOPTIONPAGE_H
#define AUTOLAYOUTOPTIONPAGE_H

#include "ui_autolayoutoptionpage.h"

#include <QWidget>

/**
 * A dialog page to display auto layouts options
 * 
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 *
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AutoLayoutOptionPage : public QWidget, private Ui::AutoLayoutOptionPage
{
    Q_OBJECT
public:
    AutoLayoutOptionPage(QWidget* parent);
    virtual ~AutoLayoutOptionPage();

    void setDefaults();
    void apply();

signals:
    void applyClicked();

protected slots:
    void slotAutoDotPathCBClicked(bool value);

};
#endif
