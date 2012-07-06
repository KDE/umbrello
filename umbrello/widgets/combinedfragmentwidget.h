/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef COMBINEDFRAGMENTWIDGET_H
#define COMBINEDFRAGMENTWIDGET_H

#include "floatingdashlinewidget.h"
#include "umlwidget.h"
#include "worktoolbar.h"

#include <QList>

/**
 * @short  A graphical version of a UML combined fragment.
 *
 * This widget is used in Sequence Diagrams.
 *
 * @author Hassan KOUCH <hkouch@hotmail.com>
 * @author Gopala Krishna
 *
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class CombinedFragmentWidget : public UMLWidget
{
    Q_OBJECT

public:
    enum CombinedFragmentType
    {
        Ref = 0,
        Opt,
        Break,
        Loop,
        Neg,
        Crit,
        Ass,
        Alt,
        Par
    };

    explicit CombinedFragmentWidget( CombinedFragmentType combinedfragmentType = Ref, Uml::IDType id = Uml::id_None );
    virtual ~CombinedFragmentWidget();

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

    /// @return The type of combined fragment.
    CombinedFragmentType combinedFragmentType() const {
        return m_combinedFragmentType;
    }
    void setCombinedFragmentType(CombinedFragmentType type);

    static CombinedFragmentType stringToCombinedFragementType(const QString& string);

    void askNameForWidgetType(UMLWidget* &targetWidget, const QString& dialogTitle,
                              const QString& dialogPrompt, const QString& defaultName);
    virtual bool activate();

    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    bool loadFromXMI(QDomElement& qElement);

public Q_SLOTS:
    void slotMenuSelection(QAction* action);

protected:
    void updateGeometry();
    void updateTextItemGroups();
    QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);

private:
    void setupFloatingWidget(FloatingDashLineWidget *widget);
    void updateFloatingWidgetsPosition();

    CombinedFragmentType m_combinedFragmentType;
    /// Dash lines of an alternative or parallel combined fragment
    QList<FloatingDashLineWidget*> m_dashLines ;

    QLineF m_fragmentBox[3];
    enum {
        TypeBoxIndex,
        ReferenceDiagramNameBoxIndex
    };

    enum {
        TypeItemIndex,
        FirstAlternativeItemIndex
    };
};

#endif
