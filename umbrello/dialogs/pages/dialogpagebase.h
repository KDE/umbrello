/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DIALOGPAGEBASE_H
#define DIALOGPAGEBASE_H

// qt class includes
#include <QWidget>

/**
 * Base class for property dialogs
 *
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 *
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class DialogPageBase : public QWidget
{
    Q_OBJECT
public:
    explicit DialogPageBase(QWidget *parent);
    virtual ~DialogPageBase();
    virtual bool isModified();

protected:
    bool m_isModified;
    virtual void keyPressEvent(QKeyEvent *event);
};

#endif
