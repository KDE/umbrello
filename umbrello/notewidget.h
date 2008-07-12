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
#include "newumlrectwidget.h"

// Forward declarations
class TextItemGroup;

/**
 * Displays a note box to allow multiple lines of text to be
 * displayed.  These widgets are diagram specific.  They will still
 * need a unique id from the @ref UMLDoc class for deletion and other
 * purposes.
 *
 * @short Displays a note box.
 * @author Paul Hensgen <phensgen@techie.com>
 * @author Gopala Krishna (port to TextItem)
 * @see NewUMLRectWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class NoteWidget : public NewUMLRectWidget
{
Q_OBJECT
public:

    /// This enum type is used to specifity the type of note.
    enum NoteType
    {
        Normal,
        PreCondition,
        PostCondition,
        Transformation
    };

    explicit NoteWidget(NoteWidget::NoteType noteType = Normal,
                        Uml::IDType id = Uml::id_None);
    virtual ~NoteWidget();

    static NoteType stringToNoteType(const QString& noteType);

    /// @return The type of note.
    NoteType noteType() const {
        return m_noteType;
    }
    void setNoteType( NoteType noteType );
    void setNoteType( const QString& noteType );

    /**
     * @return  ID of an UMLScene, or Uml::id_None if no
     *          hyperlink is set.
     */
    Uml::IDType diagramLink() const {
        return m_diagramLink;
    }
    void setDiagramLink(Uml::IDType sceneID);

    QSizeF sizeHint(Qt::SizeHint which);

    /**
     * Override default method.
     */
    void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    /**
     * Display a dialogBox to allow the user to choose the note's type
     */
    void askForNoteType(NewUMLRectWidget* &targetWidget);

    /**
     * Saves to the "notewidget" XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Loads a "notewidget" XMI element.
     */
    bool loadFromXMI( QDomElement & qElement );

public slots:
    void slotMenuSelection(QAction* action);

protected:
    void updateGeometry();
    void sizeHasChanged(const QSizeF& oldSize);

private:
    enum {
        DiagramLinkItemIndex,
        NoteTypeItemIndex,
        NoteTextItemIndex,
        TextItemCount
    };

    /// The margin for the note widget diagram.
    static const qreal Margin;

    /// Cache minimum size
    QSizeF m_minimumSize;

    // Data loaded/saved

    /// The diagram/scene this note links to.
    Uml::IDType m_diagramLink;

    /// The type of note. @see NoteWidget::NoteType
    NoteType m_noteType;

    /// The TextItemGroup to keep display note items and headings.
    TextItemGroup *m_textItemGroup;
};

#endif
