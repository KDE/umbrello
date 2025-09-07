/*
    SPDX-FileCopyrightText: 2014 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "birdviewdockwidget.h"

#include <QResizeEvent>

/**
 * Constructor.
 */
BirdViewDockWidget::BirdViewDockWidget(const QString& title, QWidget* parent, Qt::WindowFlags flags)
  : QDockWidget(title, parent, flags)
{
}

/**
 * Handle resize event of the dock widget.
 * Emits size changed signal.
 */
void BirdViewDockWidget::resizeEvent(QResizeEvent *event)
{
    Q_EMIT sizeChanged(event->size());
}

