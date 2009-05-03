/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
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
#include "widget_utils.h"

// qt includes
#include <QtCore/QSizeF>
#include <QtGui/QPainter>
#include <QtGui/QPolygonF>

// kde includes
#include <kinputdialog.h>
#include <klocale.h>

/**
 * Constructs a NoteWidget.
 *
 * @param noteType The NoteWidget::NoteType of this NoteWidget
 * @param id The unique id of the widget.
 *           The default (-1) will prompt a new ID.
 */
NoteWidget::NoteWidget(NoteType noteType , Uml::IDType id)
    : UMLWidget(0, id),
      m_diagramLink(Uml::id_None),
      m_noteType(noteType)
{
    m_baseType = Uml::wt_Note;
    createTextItemGroup();
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

/// Sets the @ref NoteWidget::NoteType for this widget.
void NoteWidget::setNoteType(NoteType noteType)
{
    m_noteType = noteType;
    updateTextItemGroups();
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
 * @todo Fix the display of diagram link.
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

void NoteWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const QSizeF sz = size();

    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());

    QSizeF triSize(10, 10);
    Widget_Utils::drawTriangledRect(painter, rect(), triSize);
}

/// Display a dialogBox to allow the user to choose the note's type
void NoteWidget::askForNoteType(UMLWidget* &targetWidget)
{
    static const QStringList list = QStringList() << i18n("Precondition")
                                                  << i18n("Postcondition")
                                                  << i18n("Transformation");

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

/**
 * Reimplemented from UMLWidget::saveToXMI to save note widget
 * into XMI.
 */
void NoteWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement noteElement = qDoc.createElement( "notewidget" );
    UMLWidget::saveToXMI( qDoc, noteElement );
    noteElement.setAttribute("text", documentation());
    if (m_diagramLink != Uml::id_None) {
        noteElement.setAttribute( "diagramlink", ID2STR(m_diagramLink) );
    }
    noteElement.setAttribute( "noteType", m_noteType);
    qElement.appendChild(noteElement);
}

/**
 * Reimplemented from UMLWidget::loadFromXMI to load note
 * widget info from XMI.
 */
bool NoteWidget::loadFromXMI( QDomElement & qElement )
{
    if( !UMLWidget::loadFromXMI( qElement ) )
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

/**
 * Reimplemented from UMLWidget::updateGeometry to calculate
 * minimum size for this widget.
 */
void NoteWidget::updateGeometry()
{
    TextItemGroup *grp = textItemGroupAt(GroupIndex);
    qreal widthWithoutNote = 0;

    // Ignore if TextItems haven't been properly constructed
    // still. (happens during creation of object)
    if(grp->textItemCount() > NoteTextItemIndex) {
        TextItem *noteTextItem = grp->textItemAt(NoteTextItemIndex);
        noteTextItem->hide();
        widthWithoutNote = grp->minimumSize().width();
        noteTextItem->show();
    }

    const qreal atleast6Chars = QFontMetricsF(grp->font()).width("w") * 6;
    if(widthWithoutNote > atleast6Chars) {
        grp->setLineBreakWidth(widthWithoutNote);
    }

    setMinimumSize(grp->minimumSize());
    UMLWidget::updateGeometry();
}

/**
 * Reimplemented from UMLWidget::updateTextItemGroups to update
 * texts and their properties.
 */
void NoteWidget::updateTextItemGroups()
{
    TextItemGroup *grp = textItemGroupAt(GroupIndex);
    grp->setTextItemCount(TextItemCount);

    TextItem *diagramLinkItem = grp->textItemAt(DiagramLinkItemIndex);
    diagramLinkItem->hide();
    //FIXME: Fixe diagram link drawing

    TextItem *noteTypeItem = grp->textItemAt(NoteTypeItemIndex);
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

    TextItem *noteTextItem = grp->textItemAt(NoteTextItemIndex);
    noteTextItem->setText(documentation());

    UMLWidget::updateTextItemGroups();
}

/**
 * Reimplemented from UMLWidget::attributeChange to handle
 * SizeHasChanged in which we set the positions of texts and to handle
 * DocumentationHasChanged to update the geometry.
 */
QVariant NoteWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if(change == SizeHasChanged) {
        TextItemGroup *grp = textItemGroupAt(GroupIndex);
        grp->setGroupGeometry(rect());
    }
    else if(change == DocumentationHasChanged) {
        updateTextItemGroups();
        return QVariant(); // no need for base method.
    }
    return UMLWidget::attributeChange(change, oldValue);
}

/**
 * Reimplemented from UMLWidget::slotMenuSelection to handle
 * some menu actions.
 */
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
        UMLWidget::slotMenuSelection(action);
        break;
    }
}

#include "notewidget.moc"
