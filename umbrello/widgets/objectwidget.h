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
#include "umlwidget.h"

class MessageWidget;
class SeqLineWidget;

/**
 * Displays an instance UMLObject of a concept.
 *
 * The local ID is needed as it can represent a class
 * that has many objects representing it.
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

    void setLocalID(const Uml::IDType& id);
    Uml::IDType localID() const;

    void setMultipleInstance(bool multiple);
    bool multipleInstance() const;

    void setDrawAsActor(bool drawAsActor);
    bool drawAsActor() const;

    void setShowDestruction(bool bShow);
    bool showDestruction() const;

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

    SeqLineWidget *sequentialLine() const;
    void adjustSequentialLineEnd();

    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    virtual bool loadFromXMI(QDomElement& qElement);

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

    SeqLineWidget* m_sequentialLine;
    QPainterPath   m_objectWidgetPath;

    Uml::IDType m_localID;    ///< local ID used on views

    bool m_multipleInstance;  ///< draw an object as a multiple object instance
    bool m_drawAsActor;       ///< object should be drawn as an Actor or an Object
    bool m_showDestruction;   ///< show object destruction on sequence diagram line

    MessageWidgetList m_messages;   ///< message widgets with an end on this widget
};

#endif
