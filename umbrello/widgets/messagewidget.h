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

    virtual void constrainTextPos(UMLSceneValue &textX, UMLSceneValue &textY, UMLSceneValue textWidth, UMLSceneValue textHeight,
                                  Uml::TextRole tr);

    //---------- End LinkWidget Interface methods implemementation.

    QString sequenceNumber() const;
    void setSequenceNumber(const QString &sequenceNumber);

    /// @return Whether the message is synchronous or asynchronous
    Uml::Sequence_Message_Type sequenceMessageType() const {
        return m_sequenceMessageType;
    }

    bool hasObjectWidget(ObjectWidget * w);

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

    void calculateWidget();

    virtual bool activate(IDChangeLog * Log = 0);

    void calculateDimensions();
    void calculateDimensionsSynchronous();
    void calculateDimensionsAsynchronous();
    void calculateDimensionsCreation();
    void calculateDimensionsLost();
    void calculateDimensionsFound();

    void draw(QPainter& p, int offsetX, int offsetY);

    void setTextPosition();

    void cleanup();

    void setSelected(bool _select);

    int getMinY();
    int getMaxY();

    UMLSceneValue onWidget(const UMLScenePoint & p);

    virtual void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);
    virtual bool loadFromXMI(QDomElement & qElement);

    void setxclicked(int xclick);
    void setyclicked(int yclick);

    /**
     * Return the xclicked
     */
    int getxclicked() const {
        return xclicked;
    }

    ListPopupMenu* setupPopupMenu(ListPopupMenu *menu = 0);

protected:
    void setLinkAndTextPos();

    int constrainX(int textX, int textWidth, Uml::TextRole tr);

    static void drawArrow( QPainter& p, int x, int y, int w,
                           Qt::ArrowType direction, bool useDottedLine = false );
    static void drawSolidArrowhead(QPainter& p, int x, int y, Qt::ArrowType direction);

    void updateResizability();

    void drawSynchronous(QPainter& p, int offsetX, int offsetY);
    void drawAsynchronous(QPainter& p, int offsetX, int offsetY);
    void drawCreation(QPainter& p, int offsetX, int offsetY);
    void drawLost(QPainter& p, int offsetX, int offsetY);
    void drawFound(QPainter& p, int offsetX, int offsetY);

    // Data loaded/saved
    QString m_SequenceNumber;
    QString m_CustomOp;
    /**
     * Whether the message is synchronous or asynchronous
     */
    Uml::Sequence_Message_Type m_sequenceMessageType;

private:
    void moveEvent(UMLSceneMouseEvent *m);
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
