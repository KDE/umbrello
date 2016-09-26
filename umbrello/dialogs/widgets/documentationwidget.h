/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef DOCUMENTATIONWIDGET_H
#define DOCUMENTATIONWIDGET_H

#include <QWidget>

#include "ui_documentationwidget.h"

class UMLObject;
class UMLWidget;

class DocumentationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DocumentationWidget(QWidget *parent = 0);
    explicit DocumentationWidget(UMLWidget *w, QWidget *parent = 0);
    ~DocumentationWidget();

    void apply();
    void setUMLObject(UMLObject *o);

private:
    Ui::DocumentationWidget *ui;

protected:
    UMLObject *m_object;
    UMLWidget *m_widget;
    void init(const QString &text);
};

#endif // DOCUMENTATIONWIDGET_H
