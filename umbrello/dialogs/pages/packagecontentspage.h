/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/
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

    Q_SLOT void slotDoubleClick(QListWidgetItem * item);
    Q_SLOT void slotShowContextMenu(const QPoint&);
    Q_SLOT void slotMenuSelection(QAction* action);

private:
    UMLPackage * m_package;
    QListWidget * m_contentLW;
    QGroupBox * m_contentGB;

    void fillListBox();
};

#endif
