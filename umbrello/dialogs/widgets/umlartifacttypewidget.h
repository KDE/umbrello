/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLARTIFACTTYPEWIDGET_H
#define UMLARTIFACTTYPEWIDGET_H

#include "artifact.h"

#include <QMap>
#include <QWidget>

class QVBoxLayout;
class QGroupBox;
class QRadioButton;

class UMLArtifactTypeWidget : public QWidget
{
    Q_OBJECT
public:
    typedef QMap<UMLArtifact::Draw_Type,QRadioButton*> ButtonMap;

    explicit UMLArtifactTypeWidget(UMLArtifact *a, QWidget *parent = 0);
    ~UMLArtifactTypeWidget();

    void addToLayout(QVBoxLayout *layout);
    void apply();

protected:
    QGroupBox *m_box;
    UMLArtifact *m_object;
    ButtonMap m_buttons;

};

#endif // UMLARTIFACTTYPEWIDGET_H
