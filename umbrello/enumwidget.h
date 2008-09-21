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

#ifndef ENUMWIDGET_H
#define ENUMWIDGET_H


#include "umlwidget.h"

/**
 * @short A uml widget to visualize enum.
 * @author Gopala Krishna A
 * @see UMLWidget
 *
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class EnumWidget : public UMLWidget
{
    Q_OBJECT
public:
    explicit EnumWidget(UMLObject* o);
    virtual ~EnumWidget();

    ///  @return True if package is shown , false otherwise.
    bool showPackage() const {
        return m_showPackage;
    }
    void setShowPackage(bool b);
    /// Toggles the status of package show.
    void toggleShowPackage() {
        setShowPackage(!m_showPackage);
    }

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *widget);

    virtual bool loadFromXMI(QDomElement& qElement);
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction *action);

protected:
    virtual void updateGeometry();
    virtual void updateTextItemGroups();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldVal);

private:
    enum {
        GroupIndex = 0
    };
    enum {
        StereoTypeItemIndex = 0,
        NameItemIndex,
        EnumLiteralStartIndex
    };

    bool m_showPackage;
    QLineF m_nameLine;
};

#endif // ENUMWIDGET_H
