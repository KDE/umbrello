/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef PINWIDGET_H
#define PINWIDGET_H

#include "umlwidget.h"

class FloatingTextWidget;

/**
 * This class is the graphical version of a UML Pin. A pinWidget is created
 * by a @ref UMLView.  An pinWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * The pinWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UML pin.
 * @author Hassan KOUCH <hkouch@hotmail.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class PinWidget : public UMLWidget
{
    Q_OBJECT
public:

    PinWidget(UMLScene* scene, UMLWidget* a, Uml::IDType id = Uml::id_None);
    virtual ~PinWidget();

    virtual void draw(QPainter & p, int offsetX, int offsetY);

    virtual void setName(const QString &strName);

    int getMinY();

    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    bool loadFromXMI(QDomElement& qElement);

    void mouseMoveEvent(QGraphicsSceneMouseEvent* me);

public slots:
    void slotMenuSelection(QAction* action);

private:
    UMLWidget* m_pOw;
    FloatingTextWidget* m_pName;  ///< displays the name of the signal widget

    int m_oldX;  ///< save the value to know how to move the floatingtext in x
    int m_oldY;  ///< save the value to know how to move the floatingtext in x

    int m_nY;
};

#endif
