/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ENUMWIDGET_H
#define ENUMWIDGET_H

#include "umlwidget.h"

#define ENUM_MARGIN 5

/**
 * Defines a graphical version of the enum.  Most of the functionality
 * will come from the @ref UMLWidget class from which class inherits from.
 *
 * @short A graphical version of an enum.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class EnumWidget : public UMLWidget
{
    Q_OBJECT
public:
    explicit EnumWidget(UMLScene *scene, UMLObject* o);
    virtual ~EnumWidget();

    bool showPackage() const;
    void setShowPackage(bool _status);
    void toggleShowPackage();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);

    virtual bool loadFromXMI(QDomElement& qElement);
    virtual void saveToXMI(QXmlStreamWriter& writer);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    QSizeF minimumSize() const;

    bool m_showPackage;
};

#endif
