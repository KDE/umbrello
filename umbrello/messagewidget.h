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
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class MessageWidget : public UMLWidget, public LinkWidget {
    Q_OBJECT
public:
    friend class MessageWidgetController;

    /**
     * Constructs a MessageWidget.
     *
     * @param view      The parent to this class.
     * @param a The role A widget for this message.
     * @param b The role B widget for this message.
     * @param y The vertical position to display this message.
     * @param sequenceMessageType Whether synchronous or asynchronous
     * @param id        A unique id used for deleting this object cleanly.
     *              The default (-1) will prompt generation of a new ID.
     */
    MessageWidget(UMLView * view, ObjectWidget* a, ObjectWidget* b,
                  int y, Uml::Sequence_Message_Type sequenceMessageType,
                  Uml::IDType id = Uml::id_None);

    /**
     * Constructs a MessageWidget.
     *
     * @param view              The parent to this class.
     * @param sequenceMessageType The Uml::Sequence_Message_Type of this message widget
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
    MessageWidget(UMLView * view, Uml::Sequence_Message_Type sequenceMessageType, Uml::IDType id = Uml::id_None);

    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * Standard deconstructor.
     */
    virtual ~MessageWidget();

    /**
     * Write property of QString m_SequenceNumber.
     */
    void setSequenceNumber( const QString &sequenceNumber );

    /**
     * Read property of QString m_SequenceNumber.
     */
    QString getSequenceNumber() const;

    /**
     * Returns whether the message is synchronous or asynchronous
     */
    Uml::Sequence_Message_Type getSequenceMessageType() const {
        return m_sequenceMessageType;
    }

    /**
     * Check to see if the given ObjectWidget is involved in the message.
     *
     * @param w The ObjectWidget to check for.
     * @return  true - if is contained, false - not contained.
     */
    bool contains(ObjectWidget * w);

    /**
     * Returns the related widget on the given side.
     *
     * @return  The ObjectWidget we are related to.
     */
    ObjectWidget* getWidget(Uml::Role_Type role);

    /**
     * Sets the related widget on the given side.
     *
     * @param ow        The ObjectWidget we are related to.
     * @param role      The Uml::Role_Type to be set for the ObjectWidget
     */
    void setWidget(ObjectWidget * ow, Uml::Role_Type role) ;

    /**
     * Returns the text widget it is related to.
     *
     * @return  The text widget we are related to.
     */
    FloatingTextWidget * getFloatingTextWidget() {
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

    /**
     * Implements operation from LinkWidget.
     * Required by FloatingTextWidget.
     */
    void lwSetFont (QFont font);

    /**
     * Overrides operation from LinkWidget.
     * Required by FloatingTextWidget.
     * @todo Move to LinkWidget.
     */
    UMLClassifier *getOperationOwner();

    /**
     * Implements operation from LinkWidget.
     * Motivated by FloatingTextWidget.
     */
    UMLOperation *getOperation();

    /**
     * Implements operation from LinkWidget.
     * Motivated by FloatingTextWidget.
     */
    void setOperation(UMLOperation *op);

    /**
     * Overrides operation from LinkWidget.
     * Required by FloatingTextWidget.
     */
    QString getCustomOpText();

    /**
     * Overrides operation from LinkWidget.
     * Required by FloatingTextWidget.
     */
    void setCustomOpText(const QString &opText);

    /**
     * Overrides operation from LinkWidget.
     * Required by FloatingTextWidget.
     *
     * @param ft        The text widget which to update.
     */
    void setMessageText(FloatingTextWidget *ft);

    /**
     * Overrides operation from LinkWidget.
     * Required by FloatingTextWidget.
     *
     * @param ft        The text widget which to update.
     * @param newText   The new text to set.
     */
    void setText(FloatingTextWidget *ft, const QString &newText);

    /**
     * Overrides operation from LinkWidget.
     * Required by FloatingTextWidget.
     *
     * @param seqNum    The new sequence number string to set.
     * @param op                The new operation string to set.
     */
    void setSeqNumAndOp(const QString &seqNum, const QString &op);

    /**
     * Overrides operation from LinkWidget.
     * Required by FloatingTextWidget.
     *
     * @param seqNum    Return this MessageWidget's sequence number string.
     * @param op                Return this MessageWidget's operation string.
     */
    UMLClassifier * getSeqNumAndOp(QString& seqNum, QString& op);

    /**
     * Calculate the geometry of the widget.
     */
    void calculateWidget();

    /**
     * Activates a MessageWidget.  Connects its m_pOw[] pointers
     * to UMLObjects and also send signals about its FloatingTextWidget.
     */
    bool activate(IDChangeLog * Log = 0);

    /**
     * Calculates the size of the widget by calling
     * calculateDimenstionsSynchronous(),
     * calculateDimenstionsAsynchronous(), or
     * calculateDimensionsCreation()
     */
    void calculateDimensions();

    /**
     * Calculates and sets the size of the widget for a synchronous message
     */
    void calculateDimensionsSynchronous();

    /**
     * Calculates and sets the size of the widget for an asynchronous message
     */
    void calculateDimensionsAsynchronous();

    /**
     * Calculates and sets the size of the widget for a creation message
     */
    void calculateDimensionsCreation();

    /**
     * Calls drawSynchronous() or drawAsynchronous()
     */
    void draw(QPainter& p, int offsetX, int offsetY);

    /**
     * Draws the calling arrow with filled in arrowhead, the
     * timeline box and the returning arrow with a dashed line and
     * stick arrowhead.
     */
    void drawSynchronous(QPainter& p, int offsetX, int offsetY);

    /**
     * Draws a solid arrow line and a stick arrow head.
     */
    void drawAsynchronous(QPainter& p, int offsetX, int offsetY);

    /**
     * Draws a solid arrow line and a stick arrow head to the
     * edge of the target object widget instead of to the
     * sequence line.
     */
    void drawCreation(QPainter& p, int offsetX, int offsetY);

    /**
     * Sets the text position relative to the sequence message.
     */
    void setTextPosition();

    /**
     * Constrains the FloatingTextWidget X and Y values supplied.
     * Overrides operation from LinkWidget.
     *
     * @param textX             Candidate X value (may be modified by the constraint.)
     * @param textY             Candidate Y value (may be modified by the constraint.)
     * @param textWidth Width of the text.
     * @param textHeight        Height of the text.
     * @param tr                Uml::Text_Role of the text.
     */
    void constrainTextPos(int &textX, int &textY, int textWidth, int textHeight,
                          Uml::Text_Role tr);

    /**
     * Used to cleanup any other widget it may need to delete.
     */
    void cleanup();

    /**
     * Sets the state of whether the widget is selected.
     *
     * @param _select   True if the widget is selected.
     */
    void setSelected(bool _select);

    /**
     * Returns the minimum height this widget should be set at on
     * a sequence diagrams.  Takes into account the widget positions
     * it is related to.
     */
    int getMinY();

    /**
     * Returns the maximum height this widget should be set at on
     * a sequence diagrams.  Takes into account the widget positions
     * it is related to.
     */
    int getMaxY();

    /**
     * Overrides operation from UMLWidget.
     *
     * @param p Point to be checked.
     *
     * @return Non-zero if the point is on a part of the MessageWidget.
     *         NB In case of a synchronous message, the empty space
     *         between call line and return line does not count, i.e. if
     *         the point is located in that space the function returns 0.
     */
    int onWidget(const QPoint & p);

    /**
     * Saves to the "messagewidget" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads from the "messagewidget" XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

protected:
    /**
     * Shortcut for calling m_pFText->setLink() followed by
     * this->setTextPosition().
     */
    void setLinkAndTextPos();

    /**
     * Returns the textX arg with constraints applied.
     * Auxiliary to setTextPosition() and constrainTextPos().
     */
    int constrainX(int textX, int textWidth, Uml::Text_Role tr);

    /**
     * Draw an arrow pointing in the given direction.
     * The arrow head is not solid, i.e. it is made up of two lines
     * like so:  --->
     * The direction can be either Qt::LeftArrow or Qt::RightArrow.
     */
    static void drawArrow( QPainter& p, int x, int y, int w,
                           Qt::ArrowType direction, bool useDottedLine = false );

    /**
     * Draw a solid (triangular) arrowhead pointing in the given direction.
     * The direction can be either Qt::LeftArrow or Qt::RightArrow.
     */
    static void drawSolidArrowhead(QPainter& p, int x, int y, Qt::ArrowType direction);

    /**
     * Update the UMLWidget::m_bResizable flag according to the
     * charactersitics of this message.
     */
    void updateResizability();

    // Data loaded/saved
    QString m_SequenceNumber;
    QString m_CustomOp;
    /**
     * Whether the message is synchronous or asynchronous
     */
    Uml::Sequence_Message_Type m_sequenceMessageType;

private:
    void moveEvent(QMoveEvent */*m*/);
    void resizeEvent(QResizeEvent */*re*/);

    ObjectWidget * m_pOw[2];
    FloatingTextWidget * m_pFText;
    int m_nY;
    /**
     * The following variables are used by loadFromXMI() as an intermediate
     * store. activate() resolves the IDs, i.e. after activate() the variables
     * m_pOw[] and m_pFText can be used.
     */
    Uml::IDType m_widgetAId, m_widgetBId, m_textId;

public slots:
    void slotWidgetMoved(Uml::IDType id);
    void slotMenuSelection(int sel);
signals:
    /**
     * emitted when the message widget is moved up or down
     * slots into ObjectWidget::slotMessageMoved()
     */
    void sigMessageMoved();
};

#endif
