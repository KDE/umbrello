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

#include <QMetaEnum>

#include "umlwidget.h"

/**
 * Represents a Send signal, Accept signal or Time event on an
 * Activity diagram.
 */
class SignalWidget : public UMLWidget
{
    Q_OBJECT
    Q_ENUMS(SignalType)
public:
    /// Enumeration that codes the different types of signal.
    enum SignalType
    {
        Send = 0,
        Accept,
        Time
    };

    explicit SignalWidget( SignalType signalType = Send, Uml::IDType id = Uml::id_None );
    virtual ~SignalWidget();

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    SignalType signalType() const;
    QString signalTypeStr() const;
    void setSignalType( SignalType signalType );

    virtual void  showPropertiesDialog();
    virtual bool loadFromXMI( QDomElement & qElement );
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    virtual void updateGeometry();
    virtual void updateTextItemGroups();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);

private:
    enum {
        GroupIndex
    };
    static const QSizeF MinimumSize;

    SignalType m_signalType; ///< Type of signal
    QPainterPath m_signalPath; ///< Path representing current drawing.
};

#endif
