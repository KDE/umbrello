/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef PRECONDITIONWIDGET_H
#define PRECONDITIONWIDGET_H

#include "umlwidget.h"

class ObjectWidget;

/**
 * @short  A graphical version of a UML Precondition (new in UML 2.0).
 *
 * This class is the graphical version of a UML Precondition.  A PreconditionWidget is created
 * by a @ref UMLView.  An PreconditionWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * The PreconditionWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 *
 * @author Florence Mattler <florence.mattler@libertysurf.fr>
 *
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class PreconditionWidget : public UMLWidget
{
    Q_OBJECT
public:
    PreconditionWidget(UMLScene* scene, ObjectWidget* a, Uml::ID::Type id = Uml::ID::None);
    virtual ~PreconditionWidget();

    void paint(QPainter  *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);

    int minY() const;
    int maxY() const;

    bool activate(IDChangeLog *Log = nullptr);
    void resolveObjectWidget(IDChangeLog* log);

    virtual void saveToXMI(QXmlStreamWriter& writer);
    virtual bool loadFromXMI(QDomElement& qElement);

    ObjectWidget *objectWidget() const;
    void setObjectWidget(ObjectWidget *objectWidget);

public slots:
    void slotMenuSelection(QAction* action);
    void slotWidgetMoved(Uml::ID::Type id);

protected:
    QSizeF minimumSize() const;

private:
    void calculateWidget();
    void calculateDimensions();

    ObjectWidget* m_objectWidget;
    int m_nY;

    Uml::ID::Type m_widgetAId;
};

#endif
