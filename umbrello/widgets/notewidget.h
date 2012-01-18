/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
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

    /// This enum type is used to specify the type of note.
    enum NoteType
    {
        Normal,
        PreCondition,
        PostCondition,
        Transformation
    };

    explicit NoteWidget(UMLScene * scene, NoteWidget::NoteType noteType = Normal,
                        Uml::IDType id = Uml::id_None);
    virtual ~NoteWidget();

    static NoteType stringToNoteType(const QString& noteType);

    NoteType noteType() const;
    void setNoteType(NoteType noteType);
    void setNoteType(const QString& noteType);

    QString documentation() const;
    void setDocumentation(const QString &newText);

    Uml::IDType diagramLink() const;
    void setDiagramLink(Uml::IDType viewID);

    virtual void paint(QPainter & p, int offsetX, int offsetY);

    void askForNoteType(UMLWidget* &targetWidget);

    virtual bool loadFromXMI(QDomElement & qElement);
    virtual void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);

public Q_SLOTS:
    void slotMenuSelection(QAction* action);

protected:
    virtual UMLSceneSize minimumSize();
    void drawText(QPainter * p = 0, int offsetX = 0, int offsetY = 0);
    void drawTextWordWrap(QPainter * p = 0, int offsetX = 0, int offsetY = 0);

private:
    Uml::IDType m_diagramLink;  ///< The diagram/scene this note links to.
    NoteType    m_noteType;     ///< The type of note. @see NoteWidget::NoteType
    QString     m_Text;
};

#endif
