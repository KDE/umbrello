/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ACTIVITYWIDGET_H
#define ACTIVITYWIDGET_H

#include "umlwidget.h"

/**
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
    Q_ENUMS(ActivityType)
    Q_PROPERTY(ActivityType activityType READ activityType WRITE setActivityType DESIGNABLE false)
    Q_PROPERTY(QString preconditionText READ preconditionText WRITE setPreconditionText DESIGNABLE false)
    Q_PROPERTY(QString postconditionText READ postconditionText WRITE setPostconditionText DESIGNABLE false)

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

    ActivityType activityType() const;
    QString activityTypeStr() const;
    void setActivityType( ActivityType activityType );

    QString preconditionText() const;
    void setPreconditionText(const QString&);

    QString postconditionText() const;
    void setPostconditionText(const QString&);

    virtual void showPropertiesDialog();
    void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *);

    virtual bool loadFromXMI( QDomElement & qElement );
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    virtual void updateGeometry();
    QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);
    void updateTextItemGroups();

    ActivityType m_activityType; ///< Type of activity.

    /**
     * Type of normal activity (Invok or not).
     * This function is call by the dialog box properties
     */
    bool m_NormalActivityType;

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
