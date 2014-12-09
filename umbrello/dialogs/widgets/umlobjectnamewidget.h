/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLOBJECTNAMEWIDGET_H
#define UMLOBJECTNAMEWIDGET_H

#include <QWidget>

class KLineEdit;

class QGridLayout;
class QLabel;

class UMLObjectNameWidget : public QWidget
{
public:
    UMLObjectNameWidget(const QString &label, const QString &text, QWidget *parent=0);
    ~UMLObjectNameWidget();

    void addToLayout(QGridLayout *layout, int row);
    QString text();
    void reset();

protected:
    QLabel *m_label;
    KLineEdit *m_editField;
    QString m_text;
};

#endif // UMLOBJECTNAMEWIDGET_H
