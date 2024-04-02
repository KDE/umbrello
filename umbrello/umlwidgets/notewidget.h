/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef NOTEWIDGET_H
#define NOTEWIDGET_H

//app includes
#include "umlwidget.h"

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
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class NoteWidget : public UMLWidget
{
    Q_OBJECT
public:

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
    bool setDiagramLink(const QString &diagramName);

    void askForNoteType(UMLWidget* &targetWidget);

    virtual bool showPropertiesDialog();

    virtual bool loadFromXMI(QDomElement & qElement);
    virtual void saveToXMI(QXmlStreamWriter& writer);
    virtual QSizeF minimumSize() const;

    static QPointer<NoteWidget> s_pCurrentNote;

public Q_SLOTS:
    void slotMenuSelection(QAction* action);

protected:
    virtual QSizeF calculateSize(bool withExtensions = true) const;
    void paintText(QPainter *painter);
    void paintTextWordWrap(QPainter *painter);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
    Uml::ID::Type m_diagramLink;  ///< The diagram/scene this note links to.
    NoteType      m_noteType;     ///< The type of note. @see NoteWidget::NoteType
};

#endif
