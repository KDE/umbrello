/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef TOOLBARSTATEASSOCIATION_H
#define TOOLBARSTATEASSOCIATION_H

#include "basictypes.h"
#include "toolbarstatepool.h"

class QGraphicsLineItem;

/**
 * Association tool to create associations between widgets.
 * With association tool, two widgets are selected clicking with left button on
 * them and an association of the needed type (depending on the association
 * button selected) is created between the widgets. When the first widget is
 * selected, a temporary visual association that follows the cursor movement is
 * created until the second widget is selected or the association cancelled.
 *
 * Also, association tool can create association class: a classifier widget
 * which is linked to an association. To do this, the classifier must be
 * selected first and then the association must be selected. The association
 * can't be selected first.
 *
 * An association can be cancelled using right button, which also returns to
 * default tool, or with middle button, which only cancels the association
 * without changing the tool being used.
 *
 * @todo refactor with common code in ToolBarStateMessages?
 */
class ToolBarStateAssociation : public ToolBarStatePool
{
    Q_OBJECT
public:

    explicit ToolBarStateAssociation(UMLScene *umlScene);
    virtual ~ToolBarStateAssociation();

    virtual void init();

    virtual void cleanBeforeChange();

    virtual void mouseMove(QGraphicsSceneMouseEvent* ome);

public Q_SLOTS:

    virtual void slotWidgetRemoved(UMLWidget* widget);

protected:

    virtual void mouseReleaseAssociation();
    virtual void mouseReleaseWidget();
    virtual void mouseReleaseEmpty();

private:

    void setFirstWidget();
    void setSecondWidget();

    Uml::AssociationType::Enum getAssociationType();

    bool addAssociationInViewAndDoc(AssociationWidget* assoc);

    void cleanAssociation();

    /**
     * The first widget in the association.
     */
    UMLWidget* m_firstWidget;

    /**
     * The association line shown while the first widget is selected and the
     * second one wasn't selected yet.
     */
    QGraphicsLineItem* m_associationLine;

};

#endif //TOOLBARSTATEASSOCIATION_H
