/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DATATYPEWIDGET_H
#define DATATYPEWIDGET_H

#include "umlwidget.h"

class UMLClassifier;

#define DATATYPE_MARGIN 5

/**
 * Defines a graphical version of the datatype.  Most of the functionality
 * will come from the @ref UMLWidget class from which class inherits from.
 *
 * @short A graphical version of an datatype.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class DatatypeWidget : public UMLWidget
{
    Q_OBJECT
public:
    DatatypeWidget(UMLScene *scene, UMLClassifier *d);
    virtual ~DatatypeWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    bool loadFromXMI(QDomElement& qElement);
    void saveToXMI(QXmlStreamWriter& writer);

public Q_SLOTS:

protected:
    QSizeF minimumSize() const;

};

#endif
