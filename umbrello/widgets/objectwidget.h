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
 * The local ID is needed as a it can represent a class
 * that has many objects representing it.
 *
 * @short Displays an instance of a Concept.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ObjectWidget : public UMLWidget
{
    Q_OBJECT
public:
    ObjectWidget(UMLScene * scene, UMLObject *o, Uml::IDType lid = Uml::id_None);
    virtual ~ObjectWidget();

    virtual void setX(int x);
    virtual void setY(int y);

    void setLocalID(Uml::IDType id);
    Uml::IDType localID() const;

    void setMultipleInstance(bool multiple);
    bool multipleInstance() const;

    bool activate(IDChangeLog* ChangeLog = 0);

    void paint(QPainter & p, int offsetX, int offsetY);

    virtual void moveEvent(QMoveEvent *m);

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

    bool messageOverlap(int y, MessageWidget* messageWidget);

    SeqLineWidget *sequentialLine() const;

    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    virtual bool loadFromXMI(QDomElement& qElement);

public slots:
    void slotMenuSelection(QAction* action);
    virtual void slotFillColorChanged(Uml::IDType viewID);
    void slotMessageMoved();

protected:
    UMLSceneSize minimumSize();

    void drawActor(QPainter & p, int offsetX, int offsetY);
    void drawObject(QPainter & p, int offsetX, int offsetY);

private:
    void tabUp();
    void tabDown();

    SeqLineWidget * m_pLine;

    Uml::IDType m_nLocalID;   ///< local ID used on views

    bool m_multipleInstance;  ///< draw an object as a multiple object
    bool m_drawAsActor;       ///< object should be drawn as an Actor or an Object
    bool m_showDestruction;   ///< show object destruction on sequence diagram line

    MessageWidgetList m_messages;   ///< message widgets with an end on this widget

};

#endif
