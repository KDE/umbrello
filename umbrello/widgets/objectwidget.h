/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef OBJECTWIDGET_H
#define OBJECTWIDGET_H

#define O_MARGIN 5
#define O_WIDTH 40
#define A_WIDTH 20
#define A_HEIGHT 40
#define A_MARGIN 5

#include "messagewidgetlist.h"
#include "messagewidget.h"


class SeqLineWidget;

/**
 * Displays an instance UMLObject of a concept.
 *
 * @short Displays an instance of a Concept.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ObjectWidget : public UMLWidget
{
    Q_OBJECT
public:
    /**
     * Creates an ObjectWidget.
     *
     * @param scene              The parent to this object.
     * @param o         The object it will be representing.
     * @param lid               The local id for the object.
     */
    ObjectWidget(UMLScene * scene, UMLObject *o, Uml::IDType lid = Uml::id_None );

    /**
     * destructor
     */
    virtual ~ObjectWidget();

    /**
     * Sets the x-coordinate.
     * Reimplements the method from UMLWidget.
     *
     * @param x The x-coordinate to be set.
     */
    virtual void setX( int x );

    /**
     * Sets the y-coordinate.
     * Reimplements the method from UMLWidget.
     *
     * @param y The y-coordinate to be set.
     */
    virtual void setY( int y );

    /**
     * Returns the local ID for this object.  This ID is used so that
     * many objects of the same @ref UMLObject instance can be on the
     * same diagram.
     *
     * @return  The local ID.
     */
    Uml::IDType localID() const {
        return m_nLocalID;
    }

    /**
     * Returns the instance name.
     *
     * @return  The instance name.
     */
    QString instanceName() const {
        return m_InstanceName;
    }

    /**
     * Sets the instance name.
     *
     * @param name              The name to set the instance name to.
     */
    void setInstanceName(const QString &name) {
        m_InstanceName = name;
    }

    /**
     * Returns whether object is representing a multi-object.
     *
     * @return  True if object is representing a multi-object.
     */
    bool multipleInstance() const {
        return m_multipleInstance;
    }

    /**
     * Sets whether representing a multi-instance object.
     *
     * @param multiple  Object state. true- multi, false - single.
     */
    void setMultipleInstance(bool multiple);

    /**
     * Sets the local id of the object.
     *
     * @param id                The local id of the object.
     */
    void setLocalID(Uml::IDType id) {
        m_nLocalID = id;
    }

    /**
     * Activate the object after serializing it from a QDataStream
     */
    bool activate(IDChangeLog* ChangeLog = 0);

    /**
     * Override default method.
     */
    void paint(QPainter & p, int offsetX, int offsetY);

    /**
     * Overrides the standard operation.
     */
    virtual void moveEvent(QMoveEvent *m);

    /**
     * Used to cleanup any other widget it may need to delete.
     */
    void cleanup();

    /**
     * Show a properties dialog for an ObjectWidget.
     */
    void showPropertiesDialog();

    /**
     * Returns whether to draw as an Actor or not.
     *
     * @return  True if widget is drawn as an actor.
     */
    bool drawAsActor() const {
        return m_drawAsActor;
    }

    /**
     * Sets whether to draw as an Actor.
     *
     * @param drawAsActor       True if widget shall be drawn as an actor.
     */
    void setDrawAsActor( bool drawAsActor );

    /**
     * Sets whether to show deconstruction on sequence line.
     *
     * @param bShow             True if destruction on line shall be shown.
     */
    void setShowDestruction( bool bShow );

    /**
     * Returns whether to show deconstruction on sequence line.
     *
     * @return  True if destruction on sequence line is shown.
     */
    bool showDestruction() const {
        return m_showDestruction;
    }

    /**
     * Returns the top margin constant (Y axis value)
     *
     * @return  Y coordinate of the space between the diagram top
     *          and the upper edge of the ObjectWidget.
     */
    int topMargin();

    /**
     * Sets the y position of the bottom of the vertical line.
     *
     * @param yPosition The y coordinate for the bottom of the line.
     */
    void setEndLine(int yPosition);

    /**
     * Returns the end Y co-ord of the seq. line.
     *
     * @return  Y coordinate of the endpoint of the sequence line.
     */
    int getEndLineY();

    /**
     * Add a message widget to the list.
     *
     * @param message   Pointer to the MessageWidget to add.
     */
    void messageAdded(MessageWidget* message);

    /**
     * Remove a message widget from the list.
     *
     * @param message   Pointer to the MessageWidget to remove.
     */
    void messageRemoved(MessageWidget* message);

    /**
     * Returns whether or not the widget can be moved vertically up.
     *
     * @return  True if widget can be moved upwards vertically.
     */
    bool canTabUp();

    /**
     * Returns whether a message is overlapping with another message.
     * Used by MessageWidget::draw() methods.
     *
     * @param y         The top of your message.
     * @param messageWidget     A pointer to your message so it doesn't
     *                  check against itself.
     */
    bool messageOverlap(int y, MessageWidget* messageWidget);

    /**
     * Return the SeqLineWidget.
     * Returns a non NULL pointer if this ObjectWidget is part of a
     * sequence diagram.
     */
    SeqLineWidget *getSeqLine();

    /**
     * Saves to the "objectwidget" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads from a "objectwidget" XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

public slots:
    /**
     * Handles a popup menu selection.
     */
    void slotMenuSelection(QAction* action);

    /**
     * Handles a color change signal.
     */
    virtual void slotColorChanged(Uml::IDType viewID);

    /**
     * Called when a message widget with an end on this object has
     * moved up or down.
     * Sets the bottom of the line to a nice position.
     */
    void slotMessageMoved();

protected:
    SeqLineWidget * m_pLine;

    /**
     * Overrides method from UMLWidget
     */
    UMLSceneSize minimumSize();

    /**
     * Draw the object as an actor.
     */
    void drawActor(QPainter & p, int offsetX, int offsetY);

    /**
     * Draw the object as an object (default).
     */
    void drawObject(QPainter & p, int offsetX, int offsetY);

    /**
     * Move the object up on a sequence diagram.
     */
    void tabUp();

    /**
     * Move the object down on a sequence diagram.
     */
    void tabDown();

    // Data loaded/saved:

    /**
     * Instance name of object.
     */
    QString m_InstanceName;

    /**
     * Local ID used on views.  Needed as a it can represent a class
     * that has many objects representing it.
     */
    Uml::IDType m_nLocalID;

    /**
     * Determines whether to draw an object as a multiple object
     * instance.
     */
    bool m_multipleInstance;

    /**
     * Determines whether the object should be drawn as an Actor or
     * an Object.
     */
    bool m_drawAsActor;

    /**
     * Determines whether to show object destruction on sequence
     * diagram line.
     */
    bool m_showDestruction;

private:
    /**
     * Initializes the key attributes of the class.
     */
    void init();

    /**
     * A list of the message widgets with an end on this widget.
     */
    MessageWidgetList messageWidgetList;
};

#endif
