/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "notewidget.h"

// app includes
#include "dialog_utils.h"
#include "dialogs/notedialog.h"
#include "listpopupmenu.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

// qt includes
#include <QtCore/QSizeF>
#include <QtGui/QPainter>
#include <QtGui/QPolygonF>

// kde includes
#include <kinputdialog.h>
#include <klocale.h>

const qreal NoteWidget::Margin = 10;

/**
 * Constructs a NoteWidget.
 *
 * @param noteType The NoteWidget::NoteType of this NoteWidget
 * @param id The unique id of the widget.
 *           The default (-1) will prompt a new ID.
 */
NoteWidget::NoteWidget(NoteType noteType , Uml::IDType id)
    : NewUMLRectWidget(0, id),
      m_minimumSize(100, 80),
      m_diagramLink(Uml::id_None),
      m_noteType(noteType)
{
    m_baseType = Uml::wt_Note;
    m_textItemGroup = new TextItemGroup(this);
    setZValue(20); //make sure always on top.
}

/// destructor
NoteWidget::~NoteWidget()
{
}

/// Converts a string to NoteWidget::NoteType
NoteWidget::NoteType NoteWidget::stringToNoteType(const QString& noteType)
{
    if (noteType == "Precondition")
        return NoteWidget::PreCondition;
    else if (noteType == "Postcondition")
        return NoteWidget::PostCondition;
    else if (noteType == "Transformation")
        return NoteWidget::Transformation;
    else
        return NoteWidget::Normal;
}

/**
 * Sets the @ref NoteWidget::NoteType for this widget.
 */
void NoteWidget::setNoteType(NoteType noteType)
{
    m_noteType = noteType;
}

/**
 * Sets the note type by converting the string parameter \a noteType
 * to NoteWidget::NoteType.
 * Provided for convenience.
 */
void NoteWidget::setNoteType( const QString& noteType )
{
    NoteType type = NoteWidget::stringToNoteType(noteType);
    setNoteType(type);
}

/**
 * Set the ID of the diagram hyperlinked to this note.
 * To switch off the hyperlink, set this to Uml::id_None.
 *
 * @param sceneID ID of an UMLScene.
 */
void NoteWidget::setDiagramLink(Uml::IDType sceneID)
{
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    UMLView *view = umldoc->findView(sceneID);
    if (view == 0) {
        uError() << "no view found for viewID " << ID2STR(sceneID);
        return;
    }

    QString linkText("Diagram: " + view->umlScene()->getName());
// #if defined (NOTEWIDGET_EMBED_EDITOR)
//     m_pEditor->setUnderline(true);
//     m_pEditor->insert(linkText);
//     m_pEditor->setUnderline(false);
// #else
//     setDoc(linkText);
//     update();
// #endif
    m_diagramLink = sceneID;
}

QSizeF NoteWidget::sizeHint(Qt::SizeHint which)
{
    if(which == Qt::MinimumSize) {
        return m_minimumSize;
    }
    return NewUMLRectWidget::sizeHint(which);
}

void NoteWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
    const QSizeF sz = size();
    qreal w = sz.width() - 1;
    qreal h = sz.height() - 1;

    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());

    QPolygonF poly(5);
    poly[0] = QPointF(0, 0);
    poly[1] = QPointF(0, h);
    poly[2] = QPointF(w, h);
    poly[3] = QPointF(w, NoteWidget::Margin);
    poly[4] = QPointF(w - NoteWidget::Margin, 0);
    painter->drawPolygon(poly);

    painter->drawLine(w - NoteWidget::Margin, 0, w - NoteWidget::Margin, NoteWidget::Margin);
    painter->drawLine(w - NoteWidget::Margin, NoteWidget::Margin, w, NoteWidget::Margin);

    // The rest of text drawing is taken care by TextItemGroup and TextItem
}

void NoteWidget::askForNoteType(NewUMLRectWidget* &targetWidget)
{
    static const QStringList list = QStringList() << "Precondition" << "Postcondition" << "Transformation";

    bool pressedOK = false;
    QString type = KInputDialog::getItem(i18n("Note Type"), i18n("Select the Note Type"), list,
                                         0, false, &pressedOK, UMLApp::app());

    if (pressedOK) {
        dynamic_cast<NoteWidget*>(targetWidget)->setNoteType(type);
    } else {
        delete targetWidget;
        targetWidget = 0;
    }
}

void NoteWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement noteElement = qDoc.createElement( "notewidget" );
    NewUMLRectWidget::saveToXMI( qDoc, noteElement );
    noteElement.setAttribute("text", documentation());
    if (m_diagramLink != Uml::id_None) {
        noteElement.setAttribute( "diagramlink", ID2STR(m_diagramLink) );
    }
    noteElement.setAttribute( "noteType", m_noteType);
    qElement.appendChild(noteElement);
}

bool NoteWidget::loadFromXMI( QDomElement & qElement )
{
    if( !NewUMLRectWidget::loadFromXMI( qElement ) )
        return false;
    setDocumentation(qElement.attribute("text", ""));
    QString diagramlink = qElement.attribute("diagramlink", "");
    if (!diagramlink.isEmpty()) {
        m_diagramLink = STR2ID(diagramlink);
    }
    QString type = qElement.attribute("noteType", "");
    setNoteType( (NoteType)type.toInt() );
    return true;
}

void NoteWidget::updateGeometry()
{
    m_textItemGroup->ensureTextItemNumbers(TextItemCount);

    TextItem dummy("");
    dummy.setDefaultTextColor(fontColor());
    dummy.setFont(font());
    // dummy.setAcceptHoverEvents(true);
    // dummy.setHoverBrush(hoverBrush);
    dummy.setAlignment(Qt::AlignCenter);
    dummy.setBackgroundBrush(Qt::NoBrush);

    TextItem *diagramLinkItem = m_textItemGroup->textItemAt(DiagramLinkItemIndex);
    diagramLinkItem->hide();
    //FIXME: Fixe diagram link drawing

    TextItem *noteTypeItem = m_textItemGroup->textItemAt(NoteTypeItemIndex);
    dummy.copyAttributesTo(noteTypeItem);
    if(m_noteType == NoteWidget::PreCondition) {
        noteTypeItem->setText(i18n("<< precondition >>"));
        noteTypeItem->show();
    }
    else if(m_noteType == NoteWidget::PostCondition) {
        noteTypeItem->setText(i18n("<< postcondition >>"));
        noteTypeItem->show();
    }
    else if(m_noteType == NoteWidget::Transformation) {
        noteTypeItem->setText(i18n("<< transformation >>"));
        noteTypeItem->show();
    }
    else { // = NoteWidget::Normal
        noteTypeItem->hide();
    }

    TextItem *noteTextItem = m_textItemGroup->textItemAt(NoteTextItemIndex);
    dummy.copyAttributesTo(noteTextItem);

    noteTextItem->hide();

    // Get width without note, so that we can display notes broken into multiple lines.
    int widthWithoutNote = int(m_textItemGroup->calculateMinimumSize().width());

    // Set the string now, since it also prevents update. Hitting two birds in a stone.
    noteTextItem->setText(documentation());
    noteTextItem->show();

    m_textItemGroup->setLineBreakWidth(widthWithoutNote == 0 ? TextItemGroup::NoLineBreak : widthWithoutNote);
    m_minimumSize = m_textItemGroup->calculateMinimumSize();

    m_minimumSize.rwidth() += 2 * NoteWidget::Margin;
    m_minimumSize.rheight() += 2 * NoteWidget::Margin;

    // Make sure the minimum size is atleast 100,30
    m_minimumSize = m_minimumSize.expandedTo(QSizeF(100, 30));

    NewUMLRectWidget::updateGeometry();
}

/// Align the text items on size change.
void NoteWidget::sizeHasChanged(const QSizeF& oldSize)
{
    QSizeF groupSize = size();
    groupSize.rwidth() -= NoteWidget::Margin * 2;
    groupSize.rheight() -= NoteWidget::Margin * 2;
    m_textItemGroup->alignVertically(groupSize);

    QPointF offset(NoteWidget::Margin, NoteWidget::Margin);
    m_textItemGroup->setPos(offset);

    NewUMLRectWidget::sizeHasChanged(oldSize);
}

void NoteWidget::slotMenuSelection(QAction* action) {
    NoteDialog * dlg = 0;
    UMLDoc *doc = UMLApp::app()->getDocument();

    // Get the menu which is always action's parent.
    ListPopupMenu *menu = qobject_cast<ListPopupMenu*>(action->parent());
    ListPopupMenu::Menu_Type sel = menu->getMenuType(action);
    switch(sel) {
        ///OBSOLETE - remove ListPopupMenu::mt_Link_Docs
        // case ListPopupMenu::mt_Link_Docs:
        //      umlScene()->updateNoteWidgets();
        //      doc->setModified(true);
        //      break;

    case ListPopupMenu::mt_Rename:
        umlScene()->updateDocumentation( false );
        dlg = new NoteDialog( umlScene()->activeView(), this );
        if( dlg->exec() ) {
            umlScene()->showDocumentation( this, true );
            doc->setModified(true);
            update();
        }
        delete dlg;
        break;

    default:
        NewUMLRectWidget::slotMenuSelection(action);
        break;
    }
}

#include "notewidget.moc"
