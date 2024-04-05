/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UseCaseWidget : public UMLWidget
{
public:
    UseCaseWidget(UMLScene * scene, UMLUseCase *o);
    virtual ~UseCaseWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);

    // For loading we can use the loadFromXMI() inherited from
    // UMLWidget.
    virtual void saveToXMI(QXmlStreamWriter& writer);

protected:
    QSizeF minimumSize() const;
};

#endif
