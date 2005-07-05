/*
 *  copyright (C) 2000
 *  Michael Edwardes <mte @users.sourceforge.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef KSTARTUPLOGO_H
#define KSTARTUPLOGO_H

#include <qwidget.h>


/**
 * Displays a startup splash screen.
 * This class is mostly borrowed from another project, probably KMyMoney2.
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class KStartupLogo : public QWidget  {
    Q_OBJECT
public:
    KStartupLogo(QWidget *parent=0, const char *name=0);
    ~KStartupLogo();
    void setHideEnabled(bool bEnabled);

protected:
    virtual void mousePressEvent( QMouseEvent*);
    bool m_bReadyToHide;
    QTimer* timer;

public slots:
    void timerDone();

};

#endif





