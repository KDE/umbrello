/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/
#ifndef PACKAGECONTENTSPAGE_H
#define PACKAGECONTENTSPAGE_H

#include "dialogpagebase.h"

class UMLPackage;
class QGroupBox;
class QListWidget;
class QListWidgetItem;

/**
 * @short The page shows all the objects that belong to a @ref UMLPackage.
 * @author Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class PackageContentsPage : public DialogPageBase
{
    Q_OBJECT
public:
    PackageContentsPage(QWidget *parent, UMLPackage *pkg);
    ~PackageContentsPage();

private:
    UMLPackage * m_package;
    QListWidget * m_contentLW;
    QGroupBox * m_contentGB;

    void fillListBox();

public slots:
    void slotDoubleClick(QListWidgetItem * item);
    void slotShowContextMenu(const QPoint&);
    void slotMenuSelection(QAction* action);
};

#endif
