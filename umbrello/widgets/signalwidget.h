/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
***************************************************************************/

#ifndef SIGNALWIDGET_H
#define SIGNALWIDGET_H

#include "floatingtextwidget.h"
#include "linkwidget.h"
#include "umlwidget.h"
#include "worktoolbar.h"

#define SIGNAL_MARGIN 5
#define SIGNAL_WIDTH 45
#define SIGNAL_HEIGHT 15

/**
 * Represents a Send signal, Accept signal or Time event on an
 * Activity diagram.
 */
class SignalWidget : public UMLWidget
{
    Q_OBJECT
public:
    /// Enumeration that codes the different types of signal.
    enum SignalType
    {
        Send = 0,
        Accept,
        Time
    };

    explicit SignalWidget( UMLScene * scene, SignalType signalType = Send, Uml::IDType id = Uml::id_None );
    virtual ~SignalWidget();

    void paint(QPainter & p, int offsetX, int offsetY);

    void setX(int newX);
    void setY(int newY);

    virtual void setName(const QString &strName);
    virtual QString getName() const;

    SignalType getSignalType() const;
    void setSignalType( SignalType signalType );

    virtual void  showPropertiesDialog();

    void mouseMoveEvent(QMouseEvent *me);

    virtual bool loadFromXMI( QDomElement & qElement );
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    UMLSceneSize minimumSize();

    /**
     * Save the value of the widget to know how to move the floatingtext
     */
    int m_oldX;
    int m_oldY;

    // Only for the time event
    /**
     * This is a pointer to the Floating Text widget which displays the
     * name of the signal widget.
     */
    FloatingTextWidget* m_pName;

    SignalType m_signalType; ///< Type of signal
};

#endif
