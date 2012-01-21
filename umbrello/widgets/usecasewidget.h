/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef USECASEWIDGET_H
#define USECASEWIDGET_H

#include "umlwidget.h"

class UMLUseCase;

#define UC_MARGIN 5
#define UC_WIDTH 60
#define UC_HEIGHT 30


/**
 * This class is the graphical version of a UMLUseCase. The
 * UseCaseWidget class inherits from the @ref UMLWidget class
 * which adds most of the functionality to this class.
 
 * A UseCaseWidget is created  by a @ref UMLView.  An UseCaseWidget belongs to only one 
 * @ref UMLView instance. When the @ref UMLView  instance that this class belongs to, 
 * it will be automatically deleted.
 *
 * If the @ref UMLUseCase class that this UseCaseWidget is displaying is deleted, the @ref UMLView will
 * make sure that this instance is also deleted.
 *
 * The UseCaseWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UMLUseCase.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UseCaseWidget : public UMLWidget
{
public:
    UseCaseWidget(UMLScene * scene, UMLUseCase *o);
    virtual ~UseCaseWidget();

    virtual void paint(QPainter & p, int offsetX, int offsetY);

    // For loading we can use the loadFromXMI() inherited from
    // UMLWidget.
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

protected:
    UMLSceneSize minimumSize();
};

#endif
