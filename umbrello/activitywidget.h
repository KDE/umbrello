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

#ifndef ACTIVITYWIDGET_H
#define ACTIVITYWIDGET_H

#include "umlwidget.h"

/**
 * @class ActivityWidget
 *
 * This class is the graphical version of a UML Activity.  A ActivityWidget is created
 * by a @ref UMLView.  An ActivityWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * The ActivityWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UML Activity.
 * @author Paul Hensgen <phensgen@techie.com>
 * @author Gopala Krishna
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ActivityWidget : public UMLWidget
{
Q_OBJECT

public:
    enum ActivityType
    {
        Initial = 0,
        Normal,
        End,
        Final,
        Branch,
        Invok,
        Param
    };

    explicit ActivityWidget(ActivityType activityType = Normal, Uml::IDType id = Uml::id_None );
    virtual ~ActivityWidget();

    /// @return Type of activity.
    ActivityType activityType() const {
        return m_activityType;
    }
    void setActivityType( ActivityType activityType );

    /// This method get the name of the preText attribute
    QString preconditionText() const {
        return m_preconditionText;
    }
    void setPreconditionText(const QString&);

    /// This method get the name of the postText attribute
    QString postconditionText() const {
        return m_postconditionText;
    }
    void setPostconditionText(const QString&);

    virtual void showPropertiesDialog();
    void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *);

    virtual bool loadFromXMI( QDomElement & qElement );
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

protected:
    virtual void updateGeometry();
    QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);
    void updateTextItemGroups();

    /**
     * Type of activity.
     */
    ActivityType m_activityType;


    /**
     * Type of normal activity (Invok or not).
     * This function is call by the dialog box properties
     */
    bool m_NormalActivityType;

                             public Q_SLOTS:

    virtual void slotMenuSelection(QAction* action);

private:
    enum {
        TextGroupIndex = 0
    };
    enum {
        NameItemIndex = 0,
        PrecondtionItemIndex,
        PostconditionItemIndex,
        TextItemCount
    };

    QString m_preconditionText;
    QString m_postconditionText;
};

#endif
