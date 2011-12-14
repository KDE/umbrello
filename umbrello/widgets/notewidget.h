/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2010                                               *
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
class NoteWidget : public UMLWidget
{
    Q_OBJECT
public:
    friend class NoteWidgetController;

    enum NoteType
    {
        Normal,
        PreCondition,
        PostCondition,
        Transformation
    };
    explicit NoteWidget(UMLView * view, NoteWidget::NoteType noteType = Normal, Uml::IDType id = Uml::id_None );
    virtual ~NoteWidget();

    void draw(QPainter & p, int offsetX, int offsetY);

    NoteType getNoteType() const;
    NoteType getNoteType(const QString& noteType) const;
    void setNoteType( NoteType noteType );
    void setNoteType( const QString& noteType );

    QString documentation() const;
    void setDocumentation(const QString &newText);

    Uml::IDType getDiagramLink() const;
    void setDiagramLink(Uml::IDType viewID);

    // this method is obsolate 
    void setFont(QFont font);

    // this method is obsolate 
    void setX(int x);
    // this method is obsolate 
    void setY(int y);

    void askForNoteType(UMLWidget* &targetWidget);

    bool loadFromXMI( QDomElement & qElement );
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );


public Q_SLOTS:
    void slotMenuSelection(QAction* action);

protected:
    QSize calculateSize();
    void drawText(QPainter * p = NULL, int offsetX = 0, int offsetY = 0);
    void drawTextWordWrap(QPainter * p = NULL, int offsetX = 0, int offsetY = 0);

    Uml::IDType m_diagramLink;  ///< Data loaded/saved.
    NoteType    m_noteType;     ///< Type of note.
    QString     m_type;         ///< Label to see the note's type.


private:
    QString m_Text;
};

#endif
