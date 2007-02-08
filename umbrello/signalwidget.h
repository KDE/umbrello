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

#define SIGNAL_MARGIN 5
#define SIGNAL_WIDTH 30
#define SIGNAL_HEIGHT 10


class SignalWidget : public UMLWidget {
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
    SignalWidget( UMLView * view, SignalType signalType = Send, Uml::IDType id = Uml::id_None );

    /**
     * destructor
     */
    virtual ~SignalWidget();

    /**
     * Overrides the standard paint event.
     */
    void draw(QPainter & p, int offsetX, int offsetY);

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

    /**
     * Show a properties dialog for a SignalWidget.
     *
     * @return  True if we modified the signal.
     */
    bool showProperties();

    /**
     * Creates the <signalwidget> XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads a <signalwidget> XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

protected:
    /**
     * Overrides method from UMLWidget
     */
    QSize calculateSize();

    /**
     * Type of signal.
     */
    SignalType m_SignalType;

};

#endif
