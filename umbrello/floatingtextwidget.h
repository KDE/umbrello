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

#ifndef FLOATINGTEXTWIDGET_H
#define FLOATINGTEXTWIDGET_H

#include "umlwidget.h"
#include "linkwidget.h"
#include "clipboard/idchangelog.h"

class UMLScene;


/**
 * @short Displays a line of text or an operation.
 *
 * This is a multipurpose class.  In its simplest form it will display a
 * line of text.
 * It can also be setup to be the text for an operation with regard to the
 * @ref MessageWidget on the sequence diagram.
 * It is also used for the text required for an association.
 *
 * The differences between all these different uses will be the popup menu
 * that is associated with it.
 *
 *
 * @author Paul Hensgen <phensgen@techie.com>
 * @author Gopala Krishna
 * @see NewUMLRectWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 *
 * @todo Implement constraints
 */
class FloatingTextWidget : public NewUMLRectWidget
{
    Q_OBJECT
public:

    /** sometimes the x/y values get numbers of <0 and >10000 - which
        is probably due to a bug somewhere in calculating the
        position.  ->workaround till problem is found: define min and
        max limits => if x or y is outside of interval, the position
        is reset ( e.g. by AssociationWidget::resetTextPositions() )
     */
    static const qreal restrictPositionMin;
    static const qreal restrictPositionMax;


    explicit FloatingTextWidget(Uml::Text_Role role = Uml::tr_Floating,
                                Uml::IDType id = Uml::id_None);
    virtual ~FloatingTextWidget();

    QString text() const;

    /**
     * Set the main body of text to display.
     *
     * @param t The text to display.
     */
    void setText(const QString &t);

    QString preText() const;

    /**
     * Set some text to be prepended to the main body of text.
     * @param t The text to prepend to main body which is displayed.
     */
    void setPreText(const QString &t);

    QString postText() const;

    /**
     * Set some text to be appended to the main body of text.
     * @param t The text to apppended to main body which is displayed.
     */
    void setPostText(const QString &t);

    /**
     * Use to get the total text (prepended + main body + appended)
     * currently displayed by the widget.
     */
    QString displayText() const;

    /**
     * Shows a dialogbox to change text of this widget and also ensures
     * validity of the set text.
     */
    void showChangeTextDialog();

    /**
     * Shows a dialogbox for the user to choose an operation, creates a
     * new operation if the user enters a new one and updates linkWidget
     * status.
     */
    void showOperationDialog();
    virtual void showPropertiesDialog();

    /// @return The LinkWidget this floating text is related to.
    LinkWidget* link() const {
        return m_linkWidget;
    }

    /**
     * Sets the link widget linked to this FloatingTextWidget to \a link
     * and also sets the link as its parent.
     *
     * @param link The LinkWidget to be linked (0 for removing link)
     *
     * @note If link is not null, then this floatingwidget is
     *       automatically parented to the corresponding linkwidget item
     *       and hence is made linkWidget's scene's member.
     */
    void setLink(LinkWidget * l);

    /// @return The Text_Role of this FloatingTextWidget.
    Uml::Text_Role textRole() const {
        return m_textRole;
    }
    void setTextRole(Uml::Text_Role role);

    void handleRename();
    void changeName(const QString& newText);

    static bool isTextValid(const QString &text);

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

    virtual bool loadFromXMI( QDomElement & qElement );
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

protected:
    virtual void updateGeometry();
    virtual void updateTextItemGroups();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

    /**
     * Sets the text for this label if it is acting as a sequence diagram
     * message or a collaboration diagram message.
     */
    void setMessageText();

private:
    /// The association or message widget we may be linked to.
    LinkWidget * m_linkWidget;

    //////////////////// Data loaded/saved:

    /// Prepended text (such as for scope of association Role or method)
    QString m_preText;
    /**
     * Ending text (such as bracket on changability notation for
     * association Role)
     */
    QString m_postText;

    /// The role the text widget will enact.
    Uml::Text_Role m_textRole;
};

#endif
