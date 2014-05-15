/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef DIAGRAMPROPERTIESPAGE_H
#define DIAGRAMPROPERTIESPAGE_H

#include "dialogpagebase.h"
#include "ui_diagrampropertiespage.h"

class UMLScene;

/**
 * A dialog page to display diagram properties
 */
class DiagramPropertiesPage : public DialogPageBase, public Ui::DiagramPropertiesPage
{
    Q_OBJECT
public:
    explicit DiagramPropertiesPage(QWidget *parent, UMLScene *scene);
    virtual ~DiagramPropertiesPage();

    void setDefaults();
    void apply();

signals:
    void applyClicked();

protected:
    UMLScene *m_scene;
    bool checkUniqueDiagramName();
};
#endif
