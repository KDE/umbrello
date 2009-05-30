/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef FLOATINGTEXTWIDGET_H
#define FLOATINGTEXTWIDGET_H

#include "umlwidget.h"
#include "linkwidget.h"

class UMLView;

class FloatingTextWidgetController;

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

    /** sometimes the x/y values get numbers of <0 and >10000 - which
        is probably due to a bug somewhere in calculating the
        position.  ->workaround till problem is found: define min and
        max limits => if x or y is outside of interval, the position
        is reset ( e.g. by AssociationWidget::resetTextPositions() )
     */
    static const int restrictPositionMin = 0;
    static const int restrictPositionMax = 3000;

    explicit FloatingTextWidget(UMLView * view, Uml::Text_Role role = Uml::tr_Floating,
                       const QString& text = "", Uml::IDType id = Uml::id_None);
    virtual ~FloatingTextWidget();

    QString text() const;
    void setText(const QString &t);

    void setTextcmd(const QString &t);

    QString preText() const;
    void setPreText(const QString &t);

    QString postText() const;
    void setPostText(const QString &t);

    /**
     * Set the sequence number to display.
     *
     * @param sn The sequence number to display.
     */
    void setSeqNum(const QString &sn);

    /**
     * Return the sequence number.
     *
     * @return The sequence number.
     */
    QString getSeqNum() const;

    /**
     * Set the operation to display.
     *
     * @param op The operation to display.
     */
    void setOperation(const QString &op);

    /**
     * Return the operation that is displayed.
     *
     * @return The operation that is displayed.
     *
    QString getOperation() const;
     */

    QString displayText() const;

    /**
     * Displays a dialog box to change the text.
     */
    void showChangeTextDialog();

    /**
     * Set the LinkWidget that this FloatingTextWidget is related to.
     *
     * @param l The related LinkWidget.
     */
    void setLink(LinkWidget * l);

    /**
     * Returns the LinkWidget this floating text is related to.
     *
     * @return The LinkWidget this floating text is related to.
     */
    LinkWidget * link();

    /**
     * Returns whether this is a line of text.
     * Used for transparency in printing.
     *
     * @return Returns whether this is a line of text.
     */
    bool isText() {
        return true;
    }

    /**
     * Activate the FloatingTextWidget after the saved data has been loaded
     *
     * @param ChangeLog Pointer to the IDChangeLog.
     * @return  true for success
     */
    bool activate( IDChangeLog* ChangeLog = 0 );

    /**
     * Sets the role type of this FloatingTextWidget.
     *
     * @param role  The Text_Role of this FloatingTextWidget.
     */
    void setRole(Uml::Text_Role role);

    /**
     * Return the role of the text widget
     *
     * @return The Text_Role of this FloatingTextWidget.
     */
    Uml::Text_Role textRole() const;

    /**
     * For a text to be valid it must be non-empty, i.e. have a length
     * larger that zero, and have at least one non whitespace character.
     *
     * @param text The string to analyze.
     * @return True if the given text is valid.
     */
    static bool isTextValid(const QString &text);

    /**
     * Overrides default method
     */
    void draw(QPainter & p, int offsetX, int offsetY);

    /**
     * Handle the ListPopupMenu::mt_Rename case of the slotMenuSelection.
     * Given an own method because it requires rather lengthy code.
     */
    void handleRename();

    /**
     * Change Name
     */
    void changeName(const QString& newText);

    /**
     * Shows an operation dialog box.
     */
    void showOperationDialog();

    /**
     * Show the properties for a FloatingTextWidget.
     * Depending on the role of the floating text wiget, the options dialog
     * for the floating text widget, the rename dialog for floating text or
     * the options dialog for the link widget are shown.
     */
    void showProperties();

    /**
     * Creates the "floatingtext" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads the "floatingtext" XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

public slots:
    /**
     * Called when a menu selection has been made.
     *
     * @param action  The action that has been selected.
     */
    void slotMenuSelection(QAction* action);

    /**
     * Sets the text for this label if it is acting as a sequence
     * diagram message or a collaboration diagram message.
     */
    void setMessageText();

protected:
    /**
     * Overrides method from UMLWidget.
     */
    QSize calculateSize();

private:
    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * Override default method
     */
    void resizeEvent(QResizeEvent* /*re*/);

    /// The association or message widget we may be linked to.
    LinkWidget * m_linkWidget;

    //////////////////// Data loaded/saved:

    /// Prepended text (such as for scope of association Role or method)
    QString m_preText;

    /// Ending text (such as bracket on changability notation for association Role)
    QString m_postText;

    /// The role the text widget will enact.
    Uml::Text_Role m_textRole;

};

#endif
