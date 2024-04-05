/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef VISIBILITYENUMWIDGET_H
#define VISIBILITYENUMWIDGET_H

#include "basictypes.h"

#include <QMap>
#include <QWidget>

class AssociationWidget;
class UMLObject;

class QVBoxLayout;
class QGroupBox;
class QRadioButton;

class VisibilityEnumWidget : public QWidget
{
    Q_OBJECT
public:
    typedef QMap<Uml::Visibility::Enum,QString> TextMap;
    typedef QMap<Uml::Visibility::Enum,QRadioButton*> ButtonMap;

    explicit VisibilityEnumWidget(UMLObject *o, QWidget *parent = nullptr);
    VisibilityEnumWidget(AssociationWidget *a, Uml::RoleType::Enum role, QWidget *parent = nullptr);
    ~VisibilityEnumWidget();

    void addToLayout(QVBoxLayout *layout);
    void apply();

protected:
    TextMap m_texts;
    QGroupBox *m_box;
    ButtonMap m_buttons;
    UMLObject *m_object;
    AssociationWidget *m_widget;
    Uml::RoleType::Enum m_role;

    void init(const QString &title);
};

#endif // VISIBILITYENUMWIDGET_H
