/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#ifndef NEWENUMWIDGET_H
#define NEWENUMWIDGET_H


#include "newumlrectwidget.h"

class TextItemGroup;

/**
 * @short A uml widget to visualize enum.
 * @author Gopala Krishna A
 * @see NewUMLRectWidget
 *
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class NewEnumWidget : public NewUMLRectWidget
{
    Q_OBJECT
public:
    /**
     * Constructs an instance of NewEnumWidget.
     * @param o The NewUMLObject this will be representing.
     */
    explicit NewEnumWidget(UMLObject* o);
    ~NewEnumWidget();

    QSizeF sizeHint(Qt::SizeHint which);

    ///  @return True if package is shown , false otherwise.
    bool showPackage() const {
        return m_showPackage;
    }
    void setShowPackage(bool b);
    /// Toggles the status of package show.
    void toggleShowPackage() {
        setShowPackage(!m_showPackage);
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *widget);

    bool loadFromXMI(QDomElement& qElement);
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

public slots:
    void slotMenuSelection(QAction *action);

protected:
    void updateGeometry();
    void sizeHasChanged(const QSizeF& oldSize);

private:
    enum {
        StereoTypeItemIndex = 0,
        NameItemIndex = 1,
        EnumLiteralStartIndex
    };

    static const qreal Margin;

    QSizeF m_minimumSize;
    bool m_showPackage;

    TextItemGroup *m_textItemGroup;
};

#endif // NEWENUMWIDGET_H
