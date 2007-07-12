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

class UMLView;

class FloatingTextWidgetController;

/**
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

class FloatingTextWidget : public UMLWidget {
    Q_OBJECT
public:
    friend class FloatingTextWidgetController;

    /** sometimes the x/y values get numbers of <0 and >10000 - which is
        probably due to a bug somewhere in calculating the position.
      -> workaround till problem is found: define min and max limits
      => if x or y is outside of interval, the position is reset
        ( e.g. by AssociationWidget::resetTextPositions() )
     */
    static const int restrictPositionMin = 0;
    static const int restrictPositionMax = 3000;


    /**
     * Constructs a FloatingTextWidget instance.
     *
     * @param view The parent of this FloatingTextWidget.
     * @param role The role this FloatingTextWidget will take up.
     * @param text The main text to display.
     * @param id The ID to assign (-1 will prompt a new ID.)
     */
    explicit FloatingTextWidget(UMLView * view, Uml::Text_Role role = Uml::tr_Floating,
                       const QString& text = "", Uml::IDType id = Uml::id_None);

    /**
     * destructor
     */
    virtual ~FloatingTextWidget();

    /**
     * Set the main body of text to display.
     *
     * @param t The text to display.
     */
    void setText(const QString &t);

    /**
     * Set some text to be prepended to the main body of text.
     * @param t The text to prepend to main body which is displayed.
     */
    void setPreText(const QString &t);

    /**
     * Set some text to be appended to the main body of text.
     * @param t The text to append to main body which is displayed.
     */
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

    /**
     * Use to get the _main body_ of text (e.g. prepended and appended
     * text is omitted) as currently displayed by the widget.
     *
     * @return The main text currently being displayed by the widget.
     */
    QString getText() const;

    /**
     * Use to get the pre-text which is prepended to the main body of
     * text to be displayed.
     *
     * @return The pre-text currently displayed by the widget.
     */
    QString getPreText() const;

    /**
     * Use to get the post-text which is appended to the main body of
     * text to be displayed.
     *
     * @return The post-text currently displayed by the widget.
     */
    QString getPostText() const;

    /**
     * Use to get the total text (prepended + main body + appended)
     * currently displayed by the widget.
     *
     * @return The text currently being displayed by the widget.
     */
    QString getDisplayText() const;

    /**
     * Displays a dialog box to change the text.
     */
    void changeTextDlg();

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
    LinkWidget * getLink();

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
    Uml::Text_Role getRole() const;

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
    void showOpDlg();

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
     * This method is public due to called by @ref MessageWidget
     * when this is text for a @ref MessageWidget.
     *
     * @param sel  The selection that has been made.
     */
    void slotMenuSelection(int sel);

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

    /**
     * The association or message widget we may be linked to.
     */
    LinkWidget * m_pLink;

    //////////////////// Data loaded/saved:

    /// Prepended text (such as for scope of association Role or method)
    QString m_PreText;
    /**
     * Ending text (such as bracket on changability notation for
     * association Role)
     */
    QString m_PostText;

    /**
     * The role the text widget will enact.
     */
    Uml::Text_Role m_Role;

};

#endif
