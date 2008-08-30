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

/**
 * Displays a note box to allow multiple lines of text to be
 * displayed.  These widgets are diagram specific.  They will still
 * need a unique id from the @ref UMLDoc class for deletion and other
 * purposes.
 *
 * @short Displays a note box.
 * @author Paul Hensgen <phensgen@techie.com>
 * @author Gopala Krishna
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

    /**
     * Sets the note type by converting the string parameter \a noteType
     * to NoteWidget::NoteType.
     * Provided for convenience.
     */
    void setNoteType( NoteType noteType );

    /**
     * Sets the note type by converting the string parameter \a noteType
     * to NoteWidget::NoteType.
     * Provided for convenience.
     */
    void setNoteType( const QString& noteType );

    /**
     * @return  ID of an UMLScene, or Uml::id_None if no
     *          hyperlink is set.
     */
    Uml::IDType diagramLink() const {
        return m_diagramLink;
    }

    /**
     * Set the ID of the diagram hyperlinked to this note.
     * To switch off the hyperlink, set this to Uml::id_None.
     *
     * @param sceneID ID of an UMLScene.
     * @todo Fix the display of diagram link.
     */
    void setDiagramLink(Uml::IDType sceneID);

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    void askForNoteType(NewUMLRectWidget* &targetWidget);

	virtual bool loadFromXMI( QDomElement & qElement );
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

public Q_SLOTS:

    /**
     * Reimplemented from NewUMLRectWidget::slotMenuSelection to handle
     * some menu actions.
     */
    void slotMenuSelection(QAction* action);

protected:
    virtual void updateGeometry();
    virtual void updateTextItemGroups();
	virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);

private:
	enum {
		GroupIndex
	};
    enum {
        DiagramLinkItemIndex,
        NoteTypeItemIndex,
        NoteTextItemIndex,
        TextItemCount
    };

    // Data loaded/saved

    /// The diagram/scene this note links to.
    Uml::IDType m_diagramLink;

    /// The type of note. @see NoteWidget::NoteType
    NoteType m_noteType;
};

#endif
