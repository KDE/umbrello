/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include "umlwidget.h"
#include "linkwidget.h"

// forward declarations
class FloatingTextWidget;
class ObjectWidget;
class QResizeEvent;
class UMLOperation;

/**
 * Used to display a message on a sequence diagram.  The message
 * could be between two objects or a message that calls itself on
 * an object.  This class will only display the line that is
 * required and the text will be setup by the @ref FloatingTextWidget
 * widget that is passed in the constructor.  A message can be
 * synchronous (calls a method and gains control back on return,
 * as happens in most programming languages) or asynchronous
 * (calls a method and gains back control immediately).
 *
 * @short Displays a message.
 * @author Paul Hensgen
 * @see UMLWidget
 * @see ObjectWidget
 * @see FloatingTextWidget
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class MessageWidget : public UMLWidget, public LinkWidget
{
    Q_OBJECT
public:
    MessageWidget(UMLScene * scene, ObjectWidget* a, ObjectWidget* b,
                  int y, Uml::SequenceMessage::Enum sequenceMessageType,
                  Uml::ID::Type id = Uml::ID::None);
    MessageWidget(UMLScene * scene, Uml::SequenceMessage::Enum sequenceMessageType,
                  Uml::ID::Type id = Uml::ID::None);
    MessageWidget(UMLScene * scene, ObjectWidget* a, int xclick, int yclick,
                  Uml::SequenceMessage::Enum sequenceMessageType,
                  Uml::ID::Type id = Uml::ID::None);
    virtual ~MessageWidget();

    virtual void setY(qreal y);

    //---------- LinkWidget Interface methods implementation from here on.

    virtual void lwSetFont (QFont font);
    virtual UMLClassifier *operationOwner();

    virtual UMLOperation *operation();
    virtual void setOperation(UMLOperation *op);

    virtual QString customOpText();
    virtual void setCustomOpText(const QString &opText);

    virtual void setMessageText(FloatingTextWidget *ft);
    virtual void setText(FloatingTextWidget *ft, const QString &newText);

    virtual QString lwOperationText();
    virtual UMLClassifier *lwClassifier();
    virtual void setOperationText(const QString &op);

    virtual void constrainTextPos(qreal &textX, qreal &textY, qreal textWidth, qreal textHeight,
                                  Uml::TextRole::Enum tr);

    //---------- End LinkWidget Interface methods implementation.

    /// @return Whether the message is synchronous or asynchronous
    Uml::SequenceMessage::Enum sequenceMessageType() const {
        return m_sequenceMessageType;
    }

    bool hasObjectWidget(ObjectWidget * w);

    ObjectWidget* objectWidget(Uml::RoleType::Enum role);
    void setObjectWidget(ObjectWidget * ow, Uml::RoleType::Enum role);

    bool isSelf() const;

    /**
     * Returns the text widget it is related to.
     *
     * @return  The text widget we are related to.
     */
    FloatingTextWidget * floatingTextWidget() {
        return m_pFText;
    }

    /**
     * Sets the text widget it is related to.
     *
     * @param f The text widget we are related to.
     */
    void setFloatingTextWidget(FloatingTextWidget * f) {
        m_pFText = f;
    }

    void calculateWidget();

    virtual bool activate(IDChangeLog *Log = nullptr);
    void resolveObjectWidget(IDChangeLog* log);

    void calculateDimensions();
    void calculateDimensionsSynchronous();
    void calculateDimensionsAsynchronous();
    void calculateDimensionsCreation();
    void calculateDimensionsDestroy();
    void calculateDimensionsLost();
    void calculateDimensionsFound();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);

    void setTextPosition();

    void cleanup();

    void setSelected(bool _select);

    int getMinY();
    int getMaxY();

    virtual QSizeF minimumSize() const;

    UMLWidget* onWidget(const QPointF& p);

    virtual void resizeWidget(qreal newW, qreal newH);

    virtual void saveToXMI(QXmlStreamWriter& writer);
    virtual bool loadFromXMI(QDomElement & qElement);

    void setxclicked(int xclick);
    void setyclicked(int yclick);

    /**
     * Return the xclicked
     */
    int getxclicked() const {
        return m_xclicked;
    }

    virtual bool showPropertiesDialog();

protected:
    virtual void moveWidgetBy(qreal diffX, qreal diffY);
    virtual void constrainMovementForAllWidgets(qreal &diffX, qreal &diffY);

    virtual bool isInResizeArea(QGraphicsSceneMouseEvent *me);

    void setLinkAndTextPos();

    int constrainX(int textX, int textWidth, Uml::TextRole::Enum tr);

    static void paintArrow(QPainter *p, int x, int y, int w,
                           Qt::ArrowType direction, bool useDottedLine = false);
    static void paintSolidArrowhead(QPainter *p, int x, int y, Qt::ArrowType direction);

    void updateResizability();

    void paintSynchronous(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void paintAsynchronous(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void paintCreation(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void paintDestroy(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void paintLost(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void paintFound(QPainter *painter, const QStyleOptionGraphicsItem *option);

    // Data loaded/saved
    QString m_CustomOp;
    /**
     * Whether the message is synchronous or asynchronous
     */
    Uml::SequenceMessage::Enum m_sequenceMessageType;

private:
    void resizeEvent(QResizeEvent *re);

    qreal constrainPositionY(qreal diffY);

    void init();

    QPointer<ObjectWidget> m_pOw[2];
    FloatingTextWidget * m_pFText;

    int m_xclicked;
    int m_yclicked;

    /**
     * The following variables are used by loadFromXMI() as an intermediate
     * store. activate() resolves the IDs, i.e. after activate() the variables
     * m_pOw[] and m_pFText can be used.
     */
    Uml::ID::Type m_widgetAId, m_widgetBId, m_textId;

public slots:
    void slotWidgetMoved(Uml::ID::Type id);
    void slotMenuSelection(QAction* action);

signals:
    /**
     * emitted when the message widget is moved up or down
     * slots into ObjectWidget::slotMessageMoved()
     */
    void sigMessageMoved();
};

#endif
