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

#ifndef NOTEWIDGET_H
#define NOTEWIDGET_H

//app includes
#include "umlwidget.h"

// forward declarations
class NoteWidgetController;

// Qt forward declarations
class QPainter;
class QTextEdit;

/**
 * Displays a note box to allow multiple lines of text to be displayed.
 * These widgets are diagram specific.  They will still need a unique id
 * from the @ref UMLDoc class for deletion and other purposes.
 *
 * @short Displays a note box.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class NoteWidget : public UMLWidget {
    Q_OBJECT
public:
    friend class NoteWidgetController;

    /**
     * Constructs a NoteWidget.
     *
     * @param view              The parent to this widget.
     * @param noteType          The NoteWidget::NoteType of this NoteWidget
     * @param id                The unique id of the widget.
     *                  The default (-1) will prompt a new ID.
     */
    explicit NoteWidget(UMLView * view, Uml::IDType id = Uml::id_None );

    /**
     * destructor
     */
    virtual ~NoteWidget();

    /**
     * Overrides method from UMLWidget.
     */
    QSize calculateSize();

    /**
     * Returns the text in the box.
     *
     * @return  The text in the box.
     */
    QString getDoc() const;

    /**
     * Sets the note documentation.
     *
     * @param newText   The text to set the documentation to.
     */
    void setDoc(const QString &newText);

    /**
     * Set the ID of the diagram hyperlinked to this note.
     * To switch off the hyperlink, set this to Uml::id_None.
     *
     * @param viewID    ID of an UMLView.
     */
    void setDiagramLink(Uml::IDType viewID);

    /**
     * Return the ID of the diagram hyperlinked to this note.
     *
     * @return  ID of an UMLView, or Uml::id_None if no
     *          hyperlink is set.
     */
    Uml::IDType getDiagramLink() const;

    /**
     * Override default method.
     */
    void draw(QPainter & p, int offsetX, int offsetY);

    /**
     * Override method from UMLWidget.
     */
    void setFont(QFont font);

    /**
     * Override method from UMLWidget.
     */
    void setX(int x);

    /**
     * Override method from UMLWidget.
     */
    void setY(int y);

    /**
     * Saves to the "notewidget" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads a "notewidget" XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

public slots:
    void slotMenuSelection(int sel);
    void slotViewScrolled(int x, int y);

protected:
    // Data loaded/saved
    Uml::IDType m_DiagramLink;

    /**
     * Draws the text.  Auxiliary to draw().
     */
    void drawText(QPainter * p = NULL, int offsetX = 0, int offsetY = 0);
private:
    /**
     * Initializes key variables for the class.
     */
    void init();

    void setEditorGeometry(int dx = 0, int dy = 0);
#if defined (NOTEWIDGET_EMBED_EDITOR)
    QTextEdit *m_pEditor;
#else
    QString m_Text;
#endif
};

#endif
