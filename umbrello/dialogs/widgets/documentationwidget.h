/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
