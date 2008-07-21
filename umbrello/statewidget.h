/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
***************************************************************************/

#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include "newumlrectwidget.h"

class TextItemGroup;

/**
 * This class is the graphical version of a UML State.
 *
 * The StateWidget class inherits from the @ref NewUMLRectWidget class
 * which adds most of the functionality to this class.
 *
 * @short  A graphical version of a UML State.
 * @author Paul Hensgen <phensgen@techie.com>
 * @author Gopala Krishna
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class StateWidget : public NewUMLRectWidget
{
    Q_OBJECT
public:
    /// Enumeration that codes the different types of state.
    enum StateType
    {
        Initial = 0,
        Normal,
        End
    };

    explicit StateWidget(StateType stateType = Normal, Uml::IDType id = Uml::id_None );
    virtual ~StateWidget();

	virtual QSizeF sizeHint(Qt::SizeHint which);
    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

	/// @return Type of state.
    StateType stateType() const {
		return m_stateType;
	}
	void setStateType( StateType stateType );

    bool addActivity( const QString &activity );
	bool removeActivity( const QString &activity );
	bool renameActivity( const QString &activity, const QString &newName );

	QStringList activities() const;
	void setActivities( QStringList & list );

	virtual void showPropertiesDialog();

	virtual bool loadFromXMI( QDomElement & qElement );
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

protected:
	virtual void updateGeometry();
	virtual void sizeHasChanged(const QSizeF& oldSize);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

private:
	static const qreal Margin;
	static const QSizeF MinimumEllipseSize;

	/// Type of state
	StateType m_stateType;

	/// Store the minimum size
	QSizeF m_minimumSize;

	/**
	 * List of activities for the state stored as TextItems in
	 * TextItemGroup.
	 */
    TextItemGroup *m_textItemGroup;

	/// Indicies of text items in m_textItemGroup
	enum {
		NameItemIndex = 0,
		ActivityStartIndex = 1
	};
};

#endif
