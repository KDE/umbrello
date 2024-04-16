/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef OBJECTWIDGET_H
#define OBJECTWIDGET_H

#include "messagewidgetlist.h"
#include "umlwidget.h"

class MessageWidget;
class SeqLineWidget;
class UMLScene;

/**
 * Displays an instance UMLObject of a concept.
 *
 * The local ID is needed as it can represent a class
 * that has many objects representing it.
 *
 * @short Displays an instance of a Concept.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ObjectWidget : public UMLWidget
{
    Q_OBJECT
public:
    ObjectWidget(UMLScene *scene, UMLObject *o);
    virtual ~ObjectWidget();

    virtual void setX(qreal x);
    virtual void setY(qreal y);

    qreal centerX();

    void setMultipleInstance(bool multiple);
    bool multipleInstance() const;

    void setSelected(bool state);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    bool activate(IDChangeLog *ChangeLog = nullptr);

    void cleanup();

    virtual bool showPropertiesDialog();

    void setDrawAsActor(bool drawAsActor);
    bool drawAsActor() const;

    void setShowDestruction(bool bShow);
    bool showDestruction() const;

    int topMargin();

    void setEndLine(int yPosition);
    int getEndLineY();

    void messageAdded(MessageWidget* message);
    void messageRemoved(MessageWidget* message);

    bool canTabUp();

    bool messageOverlap(qreal y, MessageWidget* messageWidget);

    virtual void setLineColorCmd(const QColor &color);

    SeqLineWidget *sequentialLine() const;

    virtual void resizeWidget(qreal newW, qreal newH);

    virtual void saveToXMI(QXmlStreamWriter& writer);
    virtual bool loadFromXMI(QDomElement& qElement);

public Q_SLOTS:
    void slotMenuSelection(QAction* action);
    virtual void slotFillColorChanged(Uml::ID::Type viewID);
    void slotMessageMoved();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *me);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *me);

    QSizeF minimumSize() const;

    virtual void moveEvent(QGraphicsSceneMouseEvent *event);
    virtual void moveWidgetBy(qreal diffX, qreal diffY);
    virtual void constrainMovementForAllWidgets(qreal &diffX, qreal &diffY);

    virtual QCursor resizeCursor() const;

    void paintActor(QPainter *p);
    void paintObject(QPainter *p);

private:
    void tabUp();
    void tabDown();

    void moveDestructionBy(qreal diffY);

    SeqLineWidget* m_pLine;
    bool m_multipleInstance;   ///< draw an object as a multiple object
    bool m_drawAsActor;        ///< object should be drawn as an Actor or an Object
    bool m_showDestruction;    ///< show object destruction on sequence diagram line
    bool m_isOnDestructionBox;  ///< true when a click occurred on the destruction box
    MessageWidgetList m_messages;   ///< message widgets with an end on this widget

    friend class SeqLineWidget;
};

#endif
