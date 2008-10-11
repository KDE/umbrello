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

#include "newassociationwidget.h"

#include "association.h"
#include "floatingtextwidget.h"
#include "newlinepath.h"
#include "umlscene.h"
#include "umlwidget.h"

namespace New
{
    WidgetRole::WidgetRole()
    {
        multiplicityWidget = changeabilityWidget = roleWidget = 0;
        umlWidget = 0;
    }

    WidgetRole::~WidgetRole()
    {
        // for now nothing, as ownership is with New::AssociationWidget
    }


    AssociationWidget::AssociationWidget(UMLWidget *widgetA, Uml::Association_Type type,
                                         UMLWidget *widgetB, UMLObject *umlObj) : WidgetBase(umlObj)
    {
        m_linePath = new New::LinePath(this);

        if (!umlObj && UMLAssociation::assocTypeHasUMLRepresentation(type)) {
            UMLObject *objectA = widgetA->umlObject();
            UMLObject *objectB = widgetB->umlObject();

            if (objectA && objectB) {
                // TODO: Check for already existing association of
                //       same type between same two widgets. (better
                //       to check before creation rather than here.)

                setUMLObject(new UMLAssociation(type, objectA, objectB));
            }
        }

        setWidgetForRole(widgetA, Uml::A);
        setWidgetForRole(widgetB, Uml::B);

        Q_ASSERT(widgetA->umlScene() == widgetB->umlScene());

        if (isCollaboration()) {
            UMLScene *scene = widgetA->umlScene();
            int collabID = scene->generateCollaborationId();
            setName('m' + QString::number(collabID));
        }
    }

    AssociationWidget::~AssociationWidget()
    {
        // for now nothing, as linePath is deleted by ~QGraphicsItem
    }

    UMLWidget* AssociationWidget::widgetForRole(Uml::Role_Type role) const
    {
        return m_widgetRole[role].umlWidget;
    }

    void AssociationWidget::setWidgetForRole(UMLWidget *widget, Uml::Role_Type role)
    {
        m_widgetRole[role].umlWidget = widget;

    }

    Uml::Association_Type AssociationWidget::associationType() const
    {
        return static_cast<UMLAssociation*>(umlObject())->getAssocType();
    }

    void AssociationWidget::setAssociationType(Uml::Association_Type type)
    {
        static_cast<UMLAssociation*>(umlObject())->setAssocType(type);
    }

    bool AssociationWidget::isCollaboration() const
    {
        Uml::Association_Type at = associationType();
        return (at == Uml::at_Coll_Message || at == Uml::at_Coll_Message_Self);
    }

#include "newassociationwidget.moc"
}
