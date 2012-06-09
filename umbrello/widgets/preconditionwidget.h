/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

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
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class PreconditionWidget : public UMLWidget
{
    Q_OBJECT
public:
    PreconditionWidget(UMLScene* scene, ObjectWidget* a, Uml::IDType id = Uml::id_None);
    virtual ~PreconditionWidget();

    int minY() const;
    int maxY() const;

    void paint(QPainter& p, int offsetX, int offsetY);

    bool activate(IDChangeLog* Log = 0);

    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    virtual bool loadFromXMI(QDomElement& qElement);

public slots:
    void slotMenuSelection(QAction* action);
    void slotWidgetMoved(Uml::IDType id);

protected:
    UMLSceneSize minimumSize();

private:
    void calculateWidget();
    void calculateDimensions();

    ObjectWidget* m_objectWidget;
    int m_nY;
};

#endif
