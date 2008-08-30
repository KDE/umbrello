/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
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
 * @author Gopala Krishna
 *
 * @see NewUMLRectWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ObjectWidget : public NewUMLRectWidget
{
    Q_OBJECT
public:
    ObjectWidget(UMLObject *o, const Uml::IDType &lid = Uml::id_None );
    virtual ~ObjectWidget();

    /**
     * @return The local ID for this object.
     *
     * This ID is used so that many objects of the same @ref UMLObject
     * instance can be on the same diagram.
     */
    Uml::IDType localID() const {
        return m_localID;
    }

    /**
     * Sets the local ID for this widget. See @ref ObjectWidget::localID
     * for more information about this id.
     */
    void setLocalID(const Uml::IDType& id);

    /// @retval True if this represents multiple instances of an object.
    bool multipleInstance() const {
        return m_multipleInstance;
    }

    /**
     * Sets whether this ObjectWidget represents multiple instance.
     */
    void setMultipleInstance(bool multiple);

    /// @retval True if widget is drawn as an actor.
    bool drawAsActor() const {
        return m_drawAsActor;
    }

    /**
     * Sets whether the object should be drawn as an actor or just a
     * rectangle.
     */
    void setDrawAsActor( bool drawAsActor );

    /// @retval True if destruction on sequence line is shown.
    bool showDestruction() const {
        return m_showDestruction;
    }

    /**
     * Sets the destruction visibility on the end of sequential line.
     */
    void setShowDestruction( bool bShow );

    /**
     * @return Y coordinate of the space between the diagram top and
     *         the upper edge of the ObjectWidget.
     */
    qreal topMargin() const;
    bool canTabUp() const;

    /**
     * @return Y coordinate of the endpoint of the sequence line (scene
     *         coords)
     */
    qreal lineEndY() const;

    /**
     * @return Y coordinate of the endpoint of the sequence line in parent
     *           that is this ObjectWidget coordinates.
     */
    qreal lineEndYInParentCoords() const;

    /**
     * Sets the y position of the bottom of the vertical line.
     *
     * @param yPosition The y coordinate for the bottom of the line.
     */
    void setLineEndY(qreal yPosition);

    qreal sequentialLineX() const;

    /**
     * Adds \a message linked to this widget to the MessageList of this
     * ObjectWidget.
     */
    void messageAdded(MessageWidget* message);

    /**
     * Removes \a message linked to this widget from the MessageList of
     * this ObjectWidget.
     */
    void messageRemoved(MessageWidget* message);

    /**
     * @return Whether a message is overlapping with another message.
     *
     * Used by MessageWidget::draw() methods.
     *
     * @param y              The top of your message.
     * @param messageWidget  A pointer to your message so it doesn't
     *                       check against itself.
     */
    bool messageOverlap(qreal y, MessageWidget* messageWidget) const;

    /// @return The SeqLineWidget of this ObjectWidget
    SeqLineWidget *sequentialLine() const {
        return m_sequentialLine;
    }

    /**
     * Adjusts the end of sequential line to nice position to accomodate
     * the MessageWidgets nicely.
     */
    void adjustSequentialLineEnd();

    virtual bool loadFromXMI( QDomElement & qElement );
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *w);
    virtual void showPropertiesDialog();

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    virtual void updateGeometry();
    virtual void updateTextItemGroups();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

private:

    /**
     * Shifts the object a little higher, provided it is still in diagram
     * limits.
     */
    void tabUp();

    /**
     * Shifts this object a little lower.
     */
    void tabDown();

    static const QSizeF ActorSize;
    static const qreal SequenceLineMargin;

    SeqLineWidget * m_sequentialLine;
    QPainterPath m_objectWidgetPath;

    // Data loaded/saved:

    /**
     * Local ID used on views.  Needed as a it can represent a class
     * that has many objects representing it.
     */
    Uml::IDType m_localID;

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

    /**
     * A list of the message widgets with an end on this widget.
     */
    MessageWidgetList m_messages;
};

#endif
