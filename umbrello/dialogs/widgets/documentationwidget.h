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

class AssociationWidget;
class CodeTextEdit;
class UMLObject;
class UMLWidget;

class KTextEdit;

class QGridLayout;
class QGroupBox;

class DocumentationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DocumentationWidget(UMLObject *o, QWidget *parent = 0);
    explicit DocumentationWidget(UMLWidget *w, QWidget *parent = 0);
    explicit DocumentationWidget(AssociationWidget *w, QWidget *parent = 0);
    ~DocumentationWidget();

    void apply();

protected:
    QGroupBox *m_box;
    KTextEdit *m_editField;
    CodeTextEdit *m_codeEditField;
    UMLObject *m_object;
    UMLWidget *m_widget;
    AssociationWidget *m_assocWidget;
    void init(const QString &text);
};

#endif // DOCUMENTATIONWIDGET_H
