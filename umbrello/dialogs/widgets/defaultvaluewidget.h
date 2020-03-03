/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2018-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef DEFAULTVALUEWIDGET_H
#define DEFAULTVALUEWIDGET_H

#include <QWidget>

class UMLObject;

class QGridLayout;
class QString;

/**
 * Widget for showing/editing default values
 *
 * Depending on the given UML data type the widget shows
 * either a simple edit line or a list box with enum
 * literals from which the user can select entries.
 *
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 */
class DefaultValueWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DefaultValueWidget(UMLObject *type, const QString &value, QWidget *parent = nullptr);
    ~DefaultValueWidget();

    void addToLayout(QGridLayout *layout, int row);

    QString value() const;

public slots:
    void setType(UMLObject *type);
    void setType(const QString &type);

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

private:
    class Private;
    Private *m_d;
};

#endif // DEFAULTVALUEWIDGET_H
