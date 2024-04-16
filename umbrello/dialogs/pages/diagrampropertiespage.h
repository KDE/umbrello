/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

Q_SIGNALS:
    void applyClicked();

protected:
    UMLScene *m_scene;
    bool checkUniqueDiagramName();
};
#endif
