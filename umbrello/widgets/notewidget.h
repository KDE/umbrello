/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
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
                        Uml::ID::Type id = Uml::ID::None);
    virtual ~NoteWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    static NoteType stringToNoteType(const QString& noteType);

    NoteType noteType() const;
    void setNoteType(NoteType noteType);
    void setNoteType(const QString& noteType);

    Uml::ID::Type diagramLink() const;
    void setDiagramLink(Uml::ID::Type viewID);

    void askForNoteType(UMLWidget* &targetWidget);

    virtual bool loadFromXMI(QDomElement & qElement);
    virtual void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);

public Q_SLOTS:
    void slotMenuSelection(QAction* action);

protected:
    virtual UMLSceneSize minimumSize();
    virtual UMLSceneSize calculateSize();
    void paintText(QPainter *painter);
    void paintTextWordWrap(QPainter *painter);

private:
    Uml::ID::Type m_diagramLink;  ///< The diagram/scene this note links to.
    NoteType      m_noteType;     ///< The type of note. @see NoteWidget::NoteType
    QString       m_Text;
};

#endif
