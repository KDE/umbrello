// vim: set tabstop=4 shiftwidth=4 noexpandtab:
/*
Gwenview: an image viewer
SPDX-FileCopyrightText: 2007 Aurélien Gâteau <agateau@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later

*/
#ifndef STATUSBARTOOLBUTTON_H
#define STATUSBARTOOLBUTTON_H

// Qt
#include <QToolButton>

/**
 * A thin tool button which can be grouped with another and look like one solid
 * bar:
 *
 * (button1 | button2)
 */
class StatusBarToolButton : public QToolButton
{
    Q_OBJECT
public:
    enum GroupPosition {
        NotGrouped = 0,
        GroupLeft = 1,
        GroupRight = 2,
        GroupCenter = 3
    };

    explicit StatusBarToolButton(QWidget *parent = nullptr);

    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;

    void setGroupPosition(StatusBarToolButton::GroupPosition groupPosition);

protected:
    virtual void paintEvent(QPaintEvent* event);

private:
    GroupPosition mGroupPosition;
};

#endif /* STATUSBARTOOLBUTTON_H */
