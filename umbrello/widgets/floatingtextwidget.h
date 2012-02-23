/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef FLOATINGTEXTWIDGET_H
#define FLOATINGTEXTWIDGET_H

#include "basictypes.h"
#include "umlwidget.h"

class FloatingTextWidgetController;
class LinkWidget;
class UMLScene;

/**
 * @short Displays a line of text or an operation.
 *
 * This is a multipurpose class. In its simplest form it will display a
 * line of text.
 * It can also be setup to be the text for an operation with regard to the
 * @ref MessageWidget on the sequence diagram.
 * It is also used for the text required for an association.
 *
 * The differences between all these different uses will be the popup menu
 * that is associated with it.
 *
 * @short Displays a line of text or an operation.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class FloatingTextWidget : public UMLWidget
{
    Q_OBJECT
public:
    friend class FloatingTextWidgetController;

    /** TODO:
        sometimes the x/y values get numbers of <0 and >10000 - which
        is probably due to a bug somewhere in calculating the
        position.  ->workaround till problem is found: define min and
        max limits => if x or y is outside of interval, the position
        is reset ( e.g. by AssociationWidget::resetTextPositions() )
     */
    static const int restrictPositionMin = 0;
    static const int restrictPositionMax = 3000;

    explicit FloatingTextWidget(UMLScene * scene, Uml::TextRole role = Uml::TextRole::Floating,
                                const QString& text = "", Uml::IDType id = Uml::id_None);
    virtual ~FloatingTextWidget();

    QString text() const;
    void setText(const QString &t);

    void setTextcmd(const QString &t);

    QString preText() const;
    void setPreText(const QString &t);

    QString postText() const;
    void setPostText(const QString &t);

    QString displayText() const;

    void showChangeTextDialog();
    void showOperationDialog();
    virtual void showPropertiesDialog();

    LinkWidget* link() const;
    void setLink(LinkWidget * l);

    /**
     * Returns whether this is a line of text.
     * Used for transparency in printing.
     *
     * @return Returns whether this is a line of text.
     */
    bool isText() {
        return true;
    }

    bool activate( IDChangeLog* ChangeLog = 0 );

    Uml::TextRole textRole() const;
    void setTextRole(Uml::TextRole role);

    void handleRename();
    void changeName(const QString& newText);

    static bool isTextValid(const QString &text);

    void paint(QPainter & p, int offsetX, int offsetY);

    virtual bool loadFromXMI(QDomElement & qElement);
    virtual void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);

public slots:
    virtual void slotMenuSelection(QAction* action);
    void setMessageText();

protected:
    UMLSceneSize minimumSize();

private:
    /// The association or message widget we may be linked to.
    LinkWidget * m_linkWidget;

    //////////////////// Data loaded/saved:

    /// Prepended text (such as for scope of association Role or method)
    QString m_preText;
    /// Ending text (such as bracket on changability notation for association Role)
    QString m_postText;
    /// The role the text widget will enact.
    Uml::TextRole m_textRole;

};

#endif
