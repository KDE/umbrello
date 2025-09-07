/*
    SPDX-FileCopyrightText: 2014 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#pragma once

#include <QDockWidget>

/**
 * @brief The class BirdViewDockWidget contains the bird view.
 *
 * BirdViewDockWidget is a subclass of QDockWidget.
 * It reimplements the resize event handler by emitting a size changed signal.
 *
 * @author Andi Fischer
 */
class BirdViewDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit BirdViewDockWidget(const QString& title, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags(0));

Q_SIGNALS:
    void sizeChanged(const QSize& size);

protected:
    virtual void resizeEvent(QResizeEvent *event);
};
