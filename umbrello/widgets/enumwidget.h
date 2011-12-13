/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ENUMWIDGET_H
#define ENUMWIDGET_H


#include "umlwidget.h"

class UMLView;

#define ENUM_MARGIN 5

/**
 * Defines a graphical version of the enum.  Most of the functionality
 * will come from the @ref UMLWidget class from which class inherits from.
 *
 * @short A graphical version of an enum.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class EnumWidget : public UMLWidget {
public:
    EnumWidget(UMLView* view, UMLObject* o);
    ~EnumWidget();

    // TODO: is this a generic pattern and should be moved to a base class ? 
    bool getShowPackage() const;
    void setShowPackage(bool _status);
    void toggleShowPackage();

    void draw(QPainter& p, int offsetX, int offsetY);

    bool loadFromXMI(QDomElement& qElement);
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

public slots:
    void slotMenuSelection(QAction* action);

protected:
    QSize calculateSize();

    bool m_bShowPackage;

private:
    void init();

};

#endif
