/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2008                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef NEWASSOCIATIONWIDGET_H
#define NEWASSOCIATIONWIDGET_H

#include "widgetbase.h"

class FloatingTextWidget;
class UMLWidget;

namespace New
{
    class LinePath;

    struct WidgetRole
    {
        FloatingTextWidget *multiplicityWidget;
        FloatingTextWidget *changeabilityWidget;
        FloatingTextWidget *roleWidget;

        UMLWidget *umlWidget;

        WidgetRole();
        ~WidgetRole();
    };

    class AssociationWidget : public WidgetBase
    {
        Q_OBJECT
    public:
        AssociationWidget(UMLWidget *widgetA, Uml::Association_Type type,
                          UMLWidget *widgetB, UMLObject *obj = 0);
        virtual ~AssociationWidget();

        QString multiplicity(Uml::Role_Type role) const;
        void setMultiplicity(const QString& text, Uml::Role_Type role);

        Uml::Visibility visibility(Uml::Role_Type role) const;
        void setVisibility(Uml::Visibility v, Uml::Role_Type role);

        Uml::Changeability_Type changeability(Uml::Role_Type role) const;
        void setChangeability(Uml::Changeability_Type c, Uml::Role_Type role);

        UMLWidget* widgetForRole(Uml::Role_Type role) const;
        void setWidgetForRole(UMLWidget *widget, Uml::Role_Type role);

        Uml::Association_Type associationType() const;
        void setAssociationType(Uml::Association_Type type);

        bool isCollaboration() const;

    private:
        New::LinePath *m_linePath;
        WidgetRole m_widgetRole[2];
    };
}

#endif
