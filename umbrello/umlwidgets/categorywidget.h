/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CATEGORYWIDGET_H
#define CATEGORYWIDGET_H

#include "umlwidget.h"

#define UC_MARGIN 5
#define UC_RADIUS 30

class UMLCategory;

/**
 * This class is the graphical version of a UMLCategory.  A CategoryWidget is created
 * by a @ref UMLView.  An CategoryWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * If the Category class that this CategoryWidget is displaying is deleted, the @ref UMLView will
 * make sure that this instance is also deleted.
 *
 * The CategoryWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UMLCategory.
 * @author Sharan Rao
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class CategoryWidget : public UMLWidget
{
    Q_OBJECT
public:
    CategoryWidget(UMLScene * scene, UMLCategory *o);
    virtual ~CategoryWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    void saveToXMI(QXmlStreamWriter& writer);
    // For loading we can use the loadFromXMI() inherited from UMLWidget.

protected:
    QSizeF minimumSize() const;

public Q_SLOTS:
    void slotMenuSelection(QAction* action);

};

#endif
