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
class QMoveEvent;
class QResizeEvent;
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
class MessageWidget : public UMLWidget, public LinkWidget
{
    Q_OBJECT
public:
    friend class MessageWidgetController;

    MessageWidget(UMLScene * scene, ObjectWidget* a, ObjectWidget* b,
                  int y, Uml::Sequence_Message_Type sequenceMessageType,
                  Uml::IDType id = Uml::id_None);
    MessageWidget(UMLScene * scene, Uml::Sequence_Message_Type sequenceMessageType,
                  Uml::IDType id = Uml::id_None);
    MessageWidget(UMLScene * scene, ObjectWidget* a, int xclick, int yclick,
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

    virtual void constrainTextPos(int &textX, int &textY, int textWidth, int textHeight,
                          Uml::TextRole tr);

    //---------- End LinkWidget Interface methods implemementation.

    QString sequenceNumber() const;
    void setSequenceNumber(const QString &sequenceNumber);

    /**
     * Returns whether the message is synchronous or asynchronous
     */
    Uml::Sequence_Message_Type sequenceMessageType() const {
        return m_sequenceMessageType;
    }

    /**
     * Check to see if the given ObjectWidget is involved in the message.
     *
     * @param w The ObjectWidget to check for.
     * @return  true - if is contained, false - not contained.
     */
    bool contains(ObjectWidget * w);

    ObjectWidget* objectWidget(Uml::Role_Type role);
    void setObjectWidget(ObjectWidget * ow, Uml::Role_Type role) ;

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

    /**
     * Calculate the geometry of the widget.
     */
    void calculateWidget();

    /**
     * Activates a MessageWidget.  Connects its m_pOw[] pointers
     * to UMLObjects and also send signals about its FloatingTextWidget.
     */
    virtual bool activate(IDChangeLog * Log = 0);

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
     * Calculates and sets the size of the widget for a lost message
     */
    void calculateDimensionsLost();

    /**
     * Calculates and sets the size of the widget for a found message
     */
    void calculateDimensionsFound();

    /**
     * Calls drawSynchronous() or drawAsynchronous()
     */
    void paint(QPainter& p, int offsetX, int offsetY);

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
     * Draws a solid arrow line and a stick arrow head
     * and a circle
     */
    void drawLost(QPainter& p, int offsetX, int offsetY);

     /**
     * Draws a circle and a solid arrow line and a stick arrow head
     */
    void drawFound(QPainter& p, int offsetX, int offsetY);

    void setTextPosition();

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

    virtual void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);
    virtual bool loadFromXMI(QDomElement & qElement);

    /**
    * Set the xclicked
    */
    void setxclicked (int xclick);

    /**
    * Set the xclicked
    */
    void setyclicked (int yclick);

    /**
    * Return the xclicked
    */
    int getxclicked() const {
        return xclicked;
    }

    ListPopupMenu* setupPopupMenu(ListPopupMenu *menu = 0);

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
    int constrainX(int textX, int textWidth, Uml::TextRole tr);

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
     * Update the UMLWidget::m_resizable flag according to the
     * charactersitics of this message.
     */
    void updateResizability();

     /**
     * Sets the size.
     * If m_scene->getSnapComponentSizeToGrid() is true, then
     * set the next larger size that snaps to the grid.
     */
//     void setSize(int width,int height);

    // Data loaded/saved
    QString m_SequenceNumber;
    QString m_CustomOp;
    /**
     * Whether the message is synchronous or asynchronous
     */
    Uml::Sequence_Message_Type m_sequenceMessageType;

private:
    void moveEvent(QMoveEvent *m);
    void resizeEvent(QResizeEvent *re);

    void init();

    ObjectWidget * m_pOw[2];
    FloatingTextWidget * m_pFText;
    int m_nY;

    int xclicked;
    int yclicked;

    /**
     * The following variables are used by loadFromXMI() as an intermediate
     * store. activate() resolves the IDs, i.e. after activate() the variables
     * m_pOw[] and m_pFText can be used.
     */
    Uml::IDType m_widgetAId, m_widgetBId, m_textId;

public slots:
    void slotWidgetMoved(Uml::IDType id);
    void slotMenuSelection(QAction* action);

signals:
    /**
     * emitted when the message widget is moved up or down
     * slots into ObjectWidget::slotMessageMoved()
     */
    void sigMessageMoved();
};

#endif
