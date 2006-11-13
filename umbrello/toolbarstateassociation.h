/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef TOOLBARSTATEASSOCIATION_H
#define TOOLBARSTATEASSOCIATION_H

#include "toolbarstatepool.h"

class QCanvasLine;

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
class ToolBarStateAssociation : public ToolBarStatePool {
    Q_OBJECT
public:

    /**
     * Creates a new ToolBarStateAssociation.
     *
     * @param umlView The UMLView to use.
     */
    ToolBarStateAssociation(UMLView *umlView);

    /**
     * Destroys this ToolBarStateAssociation.
     * Deletes the association line.
     */
    virtual ~ToolBarStateAssociation();

    /**
     * Goes back to the initial state.
     */
    virtual void init();

    /**
     * Called when the current tool is changed to use another tool.
     * Executes base method and cleans the association.
     */
    virtual void cleanBeforeChange();

    /**
     * Called when a mouse event happened.
     * It executes the base method and then updates the position of the
     * association line, if any.
     */
    virtual void mouseMove(QMouseEvent* ome);

public slots:

    /**
     * A widget was removed from the UMLView.
     * If the widget removed was the current widget, the current widget is set
     * to 0.
     * Also, if it was the first widget, the association is cleaned.
     */
    virtual void slotWidgetRemoved(UMLWidget* widget);

protected:

    /**
     * Called when the release event happened on an association.
     * If the button pressed isn't left button, the association being created is
     * cleaned. If it is left button, and the first widget is set and is a
     * classifier widget, it creates an association class. Otherwise, the
     * association being created is cleaned.
     */
    virtual void mouseReleaseAssociation();

    /**
     * Called when the release event happened on a widget.
     * If the button pressed isn't left button, the association is cleaned. If
     * it is left button, sets the first widget or the second, depending on
     * whether the first widget is already set or not.
     */
    virtual void mouseReleaseWidget();

    /**
     * Called when the release event happened on an empty space.
     * Cleans the association.
     */
    virtual void mouseReleaseEmpty();

private:

    /**
     * Sets the first widget in the association using the current widget.
     * If the widget can't be associated using the current type of association,
     * an error is shown and the widget isn't set.
     * Otherwise, the temporal visual association is created and the mouse
     * tracking is enabled, so move events will be delivered.
     */
    void setFirstWidget();

    /**
     * Sets the second widget in the association using the current widget and
     * creates the association.
     * If the association between the two widgets using the current type of
     * association, an error is shown and the association cancelled.
     * Otherwise, the association is created and added to the view, and the tool
     * is changed to the default tool.
     *
     * @todo Why change to the default tool? Shouldn't it better to stay on
     *       association and let the user change with a right click? The tool to
     *       create widgets doesn't change to default after creating a widget
     */
    void setSecondWidget();

    /**
     * Returns the association type of this tool.
     *
     * @return The association type of this tool.
     */
    Uml::Association_Type getAssociationType();

    /**
     * Adds an AssociationWidget to the association list and creates the
     * corresponding UMLAssociation in the current UMLDoc.
     * If the association can't be added, is deleted.
     *
     * @param association The AssociationWidget to add.
     */
    void addAssociationInViewAndDoc(AssociationWidget* association);

    /**
     * Cleans the first widget and the temporal association line, if any.
     * Both are set to null, and the association line is also deleted.
     */
    void cleanAssociation();

    /**
     * The first widget in the association.
     */
    UMLWidget* m_firstWidget;

    /**
     * The association line shown while the first widget is selected and the
     * second one wasn't selected yet.
     */
    QCanvasLine* m_associationLine;

};

#endif //TOOLBARSTATEASSOCIATION_H
