/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include "umlwidget.h"
#include "linkwidget.h"

// forward declarations
class FloatingTextWidget;
class ObjectWidget;
class UMLOperation;
class MessageWidgetController;

/**
 * Used to display a message on a sequence diagram.  The message could
 * be between two objects or a message that calls itself on an object.
 * This class will only display the line that is required and the text
 * will be setup by the @ref FloatingTextWidget widget that is passed
 * in the constructor.  A message can be synchronous (calls a method
 * and gains control back on return, as happens in most programming
 * languages) or asynchronous (calls a method and gains back control
 * immediately).
 *
 * @short Displays a message.
 * @author Paul Hensgen
 * @author Gopala Krishna
 *
 * @see UMLWidget
 * @see ObjectWidget
 * @see FloatingTextWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class MessageWidget : public UMLWidget, public LinkWidget
{
    Q_OBJECT
public:
    MessageWidget(ObjectWidget* a, ObjectWidget* b,
                  Uml::Sequence_Message_Type sequenceMessageType,
                  Uml::IDType id = Uml::id_None);
    MessageWidget(Uml::Sequence_Message_Type sequenceMessageType,
                  Uml::IDType id = Uml::id_None);
    MessageWidget(ObjectWidget* a, const QPointF& clickedPos,
                  Uml::Sequence_Message_Type sequenceMessageType,
                  Uml::IDType id = Uml::id_None);
    virtual ~MessageWidget();

    //---------- LinkWidget Interface methods implemementation from now on.

    virtual void lwSetFont (QFont font);
    virtual UMLClassifier *operationOwner();

    virtual UMLOperation *operation();
    virtual void setOperation(UMLOperation *op);

    virtual QString customOpText();
    virtual void setCustomOpText(const QString &opText);

    virtual void setMessageText(FloatingTextWidget *ft);
    virtual void setText(FloatingTextWidget *ft, const QString &newText);

    virtual UMLClassifier* seqNumAndOp(QString& seqNum, QString& op);
    virtual void setSeqNumAndOp(const QString &seqNum, const QString &op);

    virtual void constrainTextPos(qreal &textX, qreal &textY, qreal textWidth, qreal textHeight,
                                  Uml::TextRole tr);

    //---------- End LinkWidget Interface methods implemementation.

    QString sequenceNumber() const;
    void setSequenceNumber(const QString &sequenceNumber);

    /// @return Whether the message is synchronous or asynchronous
    Uml::Sequence_Message_Type sequenceMessageType() const {
        return m_sequenceMessageType;
    }

    bool hasObjectWidget(ObjectWidget * w) const;

    bool isSelf() const;

    /// @return  The ObjectWidget we are related to for given \a role.
    ObjectWidget* objectWidget(Uml::Role_Type role) const {
        return m_objectWidgets[role];
    }
    void setObjectWidget(ObjectWidget * ow, Uml::Role_Type role) ;

    /// @return  The floating text widget we are related to.
    FloatingTextWidget * floatingTextWidget() const{
        return m_floatingTextWidget;
    }
    void setFloatingTextWidget(FloatingTextWidget *f);

    void handleObjectMove(ObjectWidget *wid);

    void setTextPosition();
    void setLinkAndTextPos();

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    qreal minY() const;
    qreal maxY() const;

    virtual void setupContextMenuActions(ListPopupMenu &menu);

    virtual void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);
    virtual bool loadFromXMI(QDomElement & qElement);

protected:
    virtual void updateGeometry();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

private:
    static const qreal SynchronousBoxWidth;
    static const qreal FoundLostCircleRadius;
    static const qreal ArrowWidth;
    static const qreal ArrowHeight;
    static const qreal SelfLoopBoxWidth;

    qreal constrainedX(qreal textX, qreal textWidth, Uml::TextRole tr) const;

    void updateResizability();

    void drawSynchronous(QPainter *painter);
    void drawAsynchronous(QPainter *painter);
    void drawCreation(QPainter *painter);
    void drawLost(QPainter *painter);
    void drawFound(QPainter *painter);

    void init();

    ObjectWidget *       m_objectWidgets[2];
    FloatingTextWidget * m_floatingTextWidget;
    QString              m_sequenceNumber;
    QString              m_customOperation;

    /**
     * This is the static point on diagram used for found/lost message
     * types.
     */
    QPointF m_clickedPoint;

    /**
     * A state variable to keep track of initialization which has to
     * happen only once when the scene is set for the first time.
     *
     * @note There is an identicle variable in WidgetBase called
     *       m_isSceneSetBefore. But we can't use it as that variable
     *       is for base intialization and hence that is delibarately
     *       made private.
     */
    bool m_isMsgSceneSetBefore;

    /// Whether the message is synchronous or asynchronous
    Uml::Sequence_Message_Type m_sequenceMessageType;

public Q_SLOTS:
    void slotMenuSelection(QAction* action);

private Q_SLOTS:
    void slotDelayedInit();
};

#endif
