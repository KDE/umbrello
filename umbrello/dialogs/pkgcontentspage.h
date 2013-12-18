/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/
#ifndef PKGCONTENTSPAGE_H
#define PKGCONTENTSPAGE_H

#include <QWidget>

class ListPopupMenu;
class UMLPackage;
class QGroupBox;
class QListWidget;
class QListWidgetItem;

/**
 * @short The page shows all the objects that belong to a @ref UMLPackage.
 * @author Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class PkgContentsPage : public QWidget
{
    Q_OBJECT
public:
    PkgContentsPage(QWidget *parent, UMLPackage *pkg);
    ~PkgContentsPage();

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
