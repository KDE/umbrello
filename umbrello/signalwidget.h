/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
***************************************************************************/

#ifndef SIGNALWIDGET_H
#define SIGNALWIDGET_H
#include <qpainter.h>
#include <qstringlist.h>
#include "umlwidget.h"
#include "worktoolbar.h"
#include "floatingtextwidget.h"
#include "linkwidget.h"

#define SIGNAL_MARGIN 5
#define SIGNAL_WIDTH 45
#define SIGNAL_HEIGHT 15

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

    /**
     * Creates a Signal widget.
     *
     * @param view              The parent of the widget.
     * @param signalType        The type of Signal.
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
    explicit SignalWidget( UMLScene * scene, SignalType signalType = Send, Uml::IDType id = Uml::id_None );

    /**
     * destructor
     */
    virtual ~SignalWidget();

    /**
     * Overrides the standard paint event.
     */
    void draw(QPainter & p, int offsetX, int offsetY);

     /**
     * Overrides the UMLWidget method.
     */
    void setX(qreal newX);
     /**
     * Overrides the UMLWidget method.
     */
    void setY(qreal newY);

    /**
     * Sets the name of the signal.
     */
    virtual void setName(const QString &strName);

    /**
     * Returns the name of the Signal.
     */
    virtual QString getName() const;

    /**
     * Returns the type of Signal.
     */
    SignalType getSignalType() const;

    /**
     * Sets the type of Signal.
     */
    void setSignalType( SignalType signalType );

    void slotMenuSelection(QAction* action);

    /**
     * Show a properties dialog for a SignalWidget.
     *
     */
    virtual void  showProperties();


    /**
     * Overrides mouseMoveEvent.
     */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *me);

    /**
     * Creates the "signalwidget" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads a "signalwidget" XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

protected:
    /**
     * Overrides method from UMLWidget
     */
    QSizeF calculateSize();

    /**
     * Type of signal.
     */
    SignalType m_SignalType;

    /**
     * Save the value of the widget to know how to move the floatingtext
     */
    qreal m_oldX;
    qreal m_oldY;

    // Only for the time event
    /**
     * This is a poqrealer to the Floating Text widget which displays the
     * name of the signal widget.
     */
    FloatingTextWidget* m_pName;

};

#endif
