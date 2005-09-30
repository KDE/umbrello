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

#include "kstartuplogo.h"
#include <kapplication.h>
#include <kstandarddirs.h>
#include <qtimer.h>

KStartupLogo::KStartupLogo(QWidget * parent, const char *name)
        : QWidget(parent,name, Qt::WStyle_NoBorder | Qt::WStyle_Customize | Qt::WDestructiveClose )
,m_bReadyToHide(false) {
    //pm.load(locate("appdata", "pics/startlogo.png"));
    KStandardDirs * dirs = KGlobal::dirs();
    QString dataDir = dirs -> findResourceDir("data", "umbrello/pics/object.png");
    dataDir += "/umbrello/pics/";
    QPixmap pm(dataDir + "startlogo.png");
    setBackgroundPixmap(pm);
    setGeometry(QApplication::desktop()->width()/2-pm.width()/2,
                QApplication::desktop()->height()/2-pm.height()/2,
                pm.width(),pm.height());

    timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), this, SLOT(timerDone()) );
    timer->start(2000, true);
}

KStartupLogo::~KStartupLogo() {
    delete timer;
}

void KStartupLogo::mousePressEvent( QMouseEvent*) {
    // for the haters of raising startlogos
    if (m_bReadyToHide)
        hide();
}

void KStartupLogo::timerDone() {
    this->hide();
}

void KStartupLogo::setHideEnabled(bool bEnabled) {
    m_bReadyToHide = bEnabled;
}
#include "kstartuplogo.moc"
