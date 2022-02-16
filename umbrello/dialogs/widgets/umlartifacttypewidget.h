/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
