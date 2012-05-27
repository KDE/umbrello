/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef OBJECTWIDGET_H
#define OBJECTWIDGET_H

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
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ObjectWidget : public UMLWidget
{
    Q_OBJECT
    Q_PROPERTY(bool multipleInstance READ multipleInstance WRITE setMultipleInstance)
    Q_PROPERTY(bool drawAsActor READ drawAsActor WRITE setDrawAsActor)
    Q_PROPERTY(bool showDestruction READ showDestruction WRITE setShowDestruction)

public:
    ObjectWidget(UMLObject *o, const Uml::IDType &lid = Uml::id_None);
    virtual ~ObjectWidget();

    Uml::IDType localID() const {
        return m_localID;
    }
    void setLocalID(const Uml::IDType& id);

    /// @retval True if this represents multiple instances of an object.
    bool multipleInstance() const {
        return m_multipleInstance;
    }
    void setMultipleInstance(bool multiple);

    /// @retval True if widget is drawn as an actor.
    bool drawAsActor() const {
        return m_drawAsActor;
    }
    void setDrawAsActor(bool drawAsActor);

    /// @retval True if destruction on sequence line is shown.
    bool showDestruction() const {
        return m_showDestruction;
    }
    void setShowDestruction( bool bShow );

    qreal topMargin() const;
    bool canTabUp() const;

    qreal lineEndY() const;
    qreal lineEndYInParentCoords() const;
    void setLineEndY(qreal yPosition);

    void alignPreconditionWidgets();

    qreal sequentialLineX() const;

    void messageAdded(MessageWidget* message);
    void messageRemoved(MessageWidget* message);

    bool messageOverlap(qreal y, MessageWidget* messageWidget) const;

    /// @return The SeqLineWidget of this ObjectWidget
    SeqLineWidget *sequentialLine() const {
        return m_sequentialLine;
    }
    void adjustSequentialLineEnd();

    virtual bool loadFromXMI(QDomElement & qElement);
    virtual void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);

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
    void tabUp();
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
