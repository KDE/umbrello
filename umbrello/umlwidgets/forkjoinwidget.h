/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2005-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef FORKJOINWIDGET_H
#define FORKJOINWIDGET_H

//app includes
#include "boxwidget.h"

/**
 * @short Displays a fork/join plate in a state diagram.
 *
 * @author Oliver Kellogg  <okellogg@users.sourceforge.net>
 *
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ForkJoinWidget : public BoxWidget
{
    Q_OBJECT
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
public:
    explicit ForkJoinWidget(UMLScene * scene, Qt::Orientation ori = Qt::Horizontal, Uml::ID::Type id = Uml::ID::None);
    virtual ~ForkJoinWidget();

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation ori);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);

    virtual bool loadFromXMI(QDomElement & qElement);
    virtual void saveToXMI(QXmlStreamWriter& writer);

    virtual bool showPropertiesDialog();

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    QSizeF minimumSize() const;

    virtual void paintSelected(QPainter * p, int offsetX = nullptr, int offsetY = 0);

    void constrain(qreal& width, qreal& height);

private:
    Qt::Orientation m_orientation;   ///< whether to draw the plate horizontally or vertically
};

#endif
