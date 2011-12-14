/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef TOOLBARSTATEASSOCIATION_H
#define TOOLBARSTATEASSOCIATION_H

#include "basictypes.h"
#include "toolbarstatepool.h"

/**
 * Association tool to create associations between widgets.
 * With association tool, two widgets are selected clicking with left button on
 * them and an association of the needed type (depending on the association
 * button selected) is created between the widgets. When the first widget is
 * selected, a temporal visual association that follows the cursor movement is
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

    ToolBarStateAssociation(UMLScene *umlScene);
    virtual ~ToolBarStateAssociation();

    virtual void init();

    virtual void cleanBeforeChange();

    virtual void mouseMove(UMLSceneMouseEvent* ome);

public slots:

    virtual void slotWidgetRemoved(UMLWidget* widget);

protected:

    virtual void mouseReleaseAssociation();
    virtual void mouseReleaseWidget();
    virtual void mouseReleaseEmpty();

private:

    void setFirstWidget();
    void setSecondWidget();

    Uml::AssociationType getAssociationType();

    void addAssociationInViewAndDoc(AssociationWidget* assoc);

    void cleanAssociation();

    /**
     * The first widget in the association.
     */
    UMLWidget* m_firstWidget;

    /**
     * The association line shown while the first widget is selected and the
     * second one wasn't selected yet.
     */
    UMLSceneLine* m_associationLine;

};

#endif //TOOLBARSTATEASSOCIATION_H
