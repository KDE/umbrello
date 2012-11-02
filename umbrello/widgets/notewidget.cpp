/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "notewidget.h"

// app includes
#include "debug_utils.h"
#include "dialog_utils.h"
#include "listpopupmenu.h"
#include "notedialog.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "widget_utils.h"

// kde includes
#include <kinputdialog.h>
#include <klocale.h>

// qt includes
#include <QPainter>
#include <QPolygonF>
#include <QSizeF>

/**
 * Constructs a NoteWidget.
 *
 * @param noteType The NoteWidget::NoteType of this NoteWidget
 * @param id The unique id of the widget.
 *           The default (-1) will prompt a new ID.
 */
NoteWidget::NoteWidget(NoteType noteType, Uml::IDType id)
  : UMLWidget(WidgetBase::wt_Note, id),
    m_diagramLink(Uml::id_None),
    m_noteType(noteType)
{
    createTextItemGroup();
    setZValue(20); //make sure always on top.
}

/**
 * Destructor.
 */
NoteWidget::~NoteWidget()
{
}

/**
 * Converts a string to NoteWidget::NoteType.
 */
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
    updateTextItemGroups();
}

/**
 * Sets the note type by converting the string parameter \a noteType
 * to NoteWidget::NoteType.
 * Provided for convenience.
 */
void NoteWidget::setNoteType(const QString& noteType)
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
    UMLView *view = umlDoc()->findView(sceneID);
    if (view == 0) {
        uError() << "no view found for viewID " << ID2STR(sceneID);
        return;
    }

    QString linkText("Diagram: " + view->umlScene()->name());
    m_diagramLink = sceneID;
}

/**
 * Implementation of paint.
 */
void NoteWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(brush());

    QSizeF triSize(10, 10);
    Widget_Utils::drawTriangledRect(painter, rect(), triSize);
}

/**
 * Display a dialog box to allow the user to choose the note's type.
 */
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
 * Reimplemented from UMLWidget::loadFromXMI to load note
 * widget info from XMI.
 */
bool NoteWidget::loadFromXMI(QDomElement& qElement)
{
    if ( !UMLWidget::loadFromXMI(qElement))
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
 * Reimplemented from UMLWidget::saveToXMI to save note widget
 * into XMI.
 */
void NoteWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
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
 * Reimplemented from UMLWidget::updateGeometry to calculate
 * minimum size for this widget.
 */
void NoteWidget::updateGeometry()
{
    TextItemGroup *grp = textItemGroupAt(GroupIndex);
    qreal widthWithoutNote = 0;

    // Ignore if TextItems haven't been properly constructed
    // still. (happens during creation of object)
    if (grp->textItemCount() > NoteTextItemIndex) {
        TextItem *noteTextItem = grp->textItemAt(NoteTextItemIndex);
        noteTextItem->setExplicitVisibility(false);
        widthWithoutNote = grp->minimumSize().width();
        noteTextItem->setExplicitVisibility(true);
    }

    const qreal atleast6Chars = QFontMetricsF(grp->font()).width("w") * 6;
    if (widthWithoutNote > atleast6Chars) {
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
    diagramLinkItem->setExplicitVisibility(false);
    //FIXME: Fixe diagram link drawing

    TextItem *noteTypeItem = grp->textItemAt(NoteTypeItemIndex);
    if (m_noteType == NoteWidget::PreCondition) {
        noteTypeItem->setText(i18n("<< precondition >>"));
        noteTypeItem->setExplicitVisibility(true);
    }
    else if (m_noteType == NoteWidget::PostCondition) {
        noteTypeItem->setText(i18n("<< postcondition >>"));
        noteTypeItem->setExplicitVisibility(true);
    }
    else if (m_noteType == NoteWidget::Transformation) {
        noteTypeItem->setText(i18n("<< transformation >>"));
        noteTypeItem->setExplicitVisibility(true);
    }
    else { // = NoteWidget::Normal
        noteTypeItem->setExplicitVisibility(false);
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
    if (change == SizeHasChanged) {
        TextItemGroup *grp = textItemGroupAt(GroupIndex);
        grp->setGroupGeometry(rect());
    }
    else if (change == DocumentationHasChanged) {
        updateTextItemGroups();
        return QVariant(); // no need for base method.
    }
    return UMLWidget::attributeChange(change, oldValue);
}

/**
 * Reimplemented from UMLWidget::slotMenuSelection to handle
 * some menu actions.
 */
void NoteWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu *menu = ListPopupMenu::menuFromAction(action);
    if (!menu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }
    ListPopupMenu::MenuType sel = menu->getMenuType(action);
    switch(sel) {
    case ListPopupMenu::mt_Rename:
        {
            umlScene()->updateDocumentation(false);
            NoteDialog * dlg = new NoteDialog(umlScene()->activeView(), this);
            if (dlg->exec()) {
                umlScene()->showDocumentation(this, true);
                umlDoc()->setModified(true);
                update();
            }
            delete dlg;
        }
        break;

    case ListPopupMenu::mt_Clear:
        umlScene()->updateDocumentation(true);
        setDocumentation(QString());
        umlScene()->showDocumentation(this, true);
        umlDoc()->setModified(true);
        update();
        break;

    default:
        UMLWidget::slotMenuSelection(action);
        break;
    }
}

void NoteWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    umlScene()->updateDocumentation(false);
    NoteDialog * dlg = new NoteDialog(umlScene()->activeView(), this);
    if (dlg->exec()) {
        umlScene()->showDocumentation(this, true);
        umlDoc()->setModified(true);
        update();
    }
    delete dlg;
}

#include "notewidget.moc"
