/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef OBJECTWIDGET_H
#define OBJECTWIDGET_H

#include "messagewidgetlist.h"
#include "umlscene.h"
#include "umlwidget.h"

class MessageWidget;
class SeqLineWidget;

/**
 * Displays an instance UMLObject of a concept.
 *
 * The local ID is needed as a it can represent a class
 * that has many objects representing it.
 *
 * @short Displays an instance of a Concept.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class ObjectWidget : public UMLWidget
{
    Q_OBJECT
public:
    ObjectWidget(UMLScene * scene, UMLObject *o, Uml::ID::Type lid = Uml::ID::None);
    virtual ~ObjectWidget();

    virtual void setX(UMLSceneValue x);
    virtual void setY(UMLSceneValue y);

    UMLSceneValue centerX();

    void setLocalID(Uml::ID::Type id);
    Uml::ID::Type localID() const;

    void setMultipleInstance(bool multiple);
    bool multipleInstance() const;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    bool activate(IDChangeLog* ChangeLog = 0);

    virtual void moveEvent(QGraphicsSceneMouseEvent *m);

    void cleanup();

    void showPropertiesDialog();

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

    bool messageOverlap(UMLSceneValue y, MessageWidget* messageWidget);

    SeqLineWidget *sequentialLine() const;

    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    virtual bool loadFromXMI(QDomElement& qElement);

public slots:
    void slotMenuSelection(QAction* action);
    virtual void slotFillColorChanged(Uml::ID::Type viewID);
    void slotMessageMoved();

protected:
    UMLSceneSize minimumSize();

    void paintActor(QPainter *p);
    void paintObject(QPainter *p);

private:
    void tabUp();
    void tabDown();

    SeqLineWidget* m_pLine;
    Uml::ID::Type  m_nLocalID; ///< local ID used on views
    bool m_multipleInstance;   ///< draw an object as a multiple object
    bool m_drawAsActor;        ///< object should be drawn as an Actor or an Object
    bool m_showDestruction;    ///< show object destruction on sequence diagram line

    MessageWidgetList m_messages;   ///< message widgets with an end on this widget

};

#endif
