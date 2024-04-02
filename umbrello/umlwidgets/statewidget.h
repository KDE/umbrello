/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include "umlwidget.h"

#include <QPainter>
#include <QStringList>

#define STATE_MARGIN 5
#define STATE_WIDTH 30
#define STATE_HEIGHT 10

/**
 * This class is the graphical version of a UML State.
 *
 * A StateWidget is created by a @ref UMLView.  A StateWidget belongs to
 * only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to is destroyed,
 * it will be automatically deleted.
 *
 * The StateWidget class inherits from the @ref UMLWidget class
 * which adds most of the functionality to this class.
 *
 * @short  A graphical version of a UML State.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class StateWidget : public UMLWidget
{
    Q_OBJECT
    Q_ENUMS(StateType)
public:
    /// Enumeration that codes the different types of state.
    enum StateType
    {
        Initial = 0,     // Pseudostate
        Normal,
        End,
        Fork,            // Pseudostate
        Join,            // Pseudostate
        Junction,        // Pseudostate
        DeepHistory,     // Pseudostate
        ShallowHistory,  // Pseudostate
        Choice,          // Pseudostate
        //Terminate        // Pseudostate
        //EntryPoint       // Pseudostate
        //ExitPoint        // Pseudostate
        Combined         // Pseudostate
    };

    explicit StateWidget(UMLScene * scene, StateType stateType = Normal, Uml::ID::Type id = Uml::ID::None);
    virtual ~StateWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    StateType stateType() const;
    QString stateTypeStr() const;
    void setStateType(StateType stateType);

    bool addActivity(const QString &activity);
    bool removeActivity(const QString &activity);
    bool renameActivity(const QString &activity, const QString &newName);

    QStringList activities() const;
    void setActivities(const QStringList &list);

    bool drawVertical() const;
    void setDrawVertical(bool to = true);

    virtual bool showPropertiesDialog();

    virtual bool loadFromXMI(QDomElement & qElement);
    virtual void saveToXMI(QXmlStreamWriter& writer);

protected:
    QSizeF minimumSize() const;
    QSizeF maximumSize();
    void setAspectRatioMode();
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

private:
    StateType   m_stateType;   ///< Type of state.
    bool m_drawVertical;   ///< whether to draw the fork/join horizontally or vertically
    QStringList m_Activities;  ///< List of activities for the state.
    QSizeF m_size;       ///< widget size used by combined state
};

#endif
