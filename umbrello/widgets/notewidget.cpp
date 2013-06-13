/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "notewidget.h"

// app includes
#include "debug_utils.h"
#include "listpopupmenu.h"
#include "notedialog.h"
#include "umldoc.h"
#include "umlview.h"
#include "uml.h"

// kde includes
#include <klocale.h>
#include <kcolordialog.h>
#include <kinputdialog.h>

// qt includes
#include <QPainter>

/**
 * Constructs a NoteWidget.
 *
 * @param scene      The parent to this widget.
 * @param noteType   The NoteWidget::NoteType of this NoteWidget
 * @param id         The unique id of the widget.
 *                   The default (-1) will prompt a new ID.
 */
NoteWidget::NoteWidget(UMLScene * scene, NoteType noteType , Uml::ID::Type id)
  : UMLWidget(scene, WidgetBase::wt_Note, id, NULL),
    m_diagramLink(Uml::ID::None),
    m_noteType(noteType)
{
    setZValue(20); //make sure always on top.
}

/**
 * Destructor.
 */
NoteWidget::~NoteWidget()
{
}

/**
 * Override default method.
 */
void NoteWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    const int margin = 10;
    int w = width()-1;
    int h = height()-1;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    QPolygon poly(6);
    poly.setPoint(0, 0, 0);
    poly.setPoint(1, 0, h);
    poly.setPoint(2, w, h);
    poly.setPoint(3, w, margin);
    poly.setPoint(4, w - margin, 0);
    poly.setPoint(5, 0, 0);

    setPenFromSettings(painter);
    if ( UMLWidget::useFillColor() ) {
        QBrush brush( UMLWidget::fillColor() );
        painter->setBrush(brush);
        painter->drawPolygon(poly);
    } else
        painter->drawPolyline(poly);
    painter->drawLine(w - margin, 0, w - margin, margin);
    painter->drawLine(w - margin, margin, w, margin);
    painter->setPen(textColor());
    switch(m_noteType) {
    case NoteWidget::PreCondition :
        painter->drawText(0, margin, w, fontHeight, Qt::AlignCenter, "<< precondition >>");
        break;
    case NoteWidget::PostCondition :
        painter->drawText(0, margin, w, fontHeight, Qt::AlignCenter, "<< postcondition >>");
        break;
    case NoteWidget::Transformation :
        painter->drawText(0, margin, w, fontHeight, Qt::AlignCenter, "<< transformation >>");
        break;
    case NoteWidget::Normal :
    default :
        break;
    }

    if (m_selected) {
        paintSelected(painter);
    }

//    paintText(&p, 0, 0);
    paintTextWordWrap(painter);
}

/**
 * Returns the type of note.
 */
NoteWidget::NoteType NoteWidget::noteType() const
{
    return m_noteType;
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
 * Sets the type of note.
 */
void NoteWidget::setNoteType(NoteType noteType)
{
    m_noteType = noteType;
}

/**
 * Sets the type of note.
 */
void NoteWidget::setNoteType(const QString& noteType)
{
    setNoteType(stringToNoteType(noteType));
}

/**
 * Return the ID of the diagram hyperlinked to this note.
 *
 * @return  ID of an UMLView, or Uml::ID::None if no
 *          hyperlink is set.
 */
Uml::ID::Type NoteWidget::diagramLink() const
{
    return m_diagramLink;
}

/**
 * Set the ID of the diagram hyperlinked to this note.
 * To switch off the hyperlink, set this to Uml::id_None.
 *
 * @param sceneID    ID of an UMLView.
 */
void NoteWidget::setDiagramLink(Uml::ID::Type viewID)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    UMLView *view = umldoc->findView(viewID);
    if (view == NULL) {
        uError() << "no view found for viewID " << Uml::ID::toString(viewID);
        return;
    }
    QString linkText("Diagram: " + view->umlScene()->name());
    setDocumentation(linkText);
    m_diagramLink = viewID;
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
    QString type = KInputDialog::getItem (i18n("Note Type"), i18n("Select the Note Type"), list,
                                          0, false, &pressedOK, UMLApp::app());

    if (pressedOK) {
        dynamic_cast<NoteWidget*>(targetWidget)->setNoteType(type);
    } else {
        targetWidget->cleanup();
        delete targetWidget;
        targetWidget = 0;
    }
}

/**
 * Loads a "notewidget" XMI element.
 */
bool NoteWidget::loadFromXMI(QDomElement & qElement)
{
    if (!UMLWidget::loadFromXMI(qElement))
        return false;
    setZValue(20); //make sure always on top.
    setDocumentation( qElement.attribute("text", "") );
    QString diagramlink = qElement.attribute("diagramlink", "");
    if (!diagramlink.isEmpty())
        m_diagramLink = Uml::ID::fromString(diagramlink);
    QString type = qElement.attribute("noteType", "");
    setNoteType( (NoteType)type.toInt() );
    return true;
}

/**
 * Saves to the "notewidget" XMI element.
 */
void NoteWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement noteElement = qDoc.createElement( "notewidget" );
    UMLWidget::saveToXMI( qDoc, noteElement );
    noteElement.setAttribute( "text", documentation() );
    if (m_diagramLink != Uml::ID::None)
        noteElement.setAttribute( "diagramlink", Uml::ID::toString(m_diagramLink) );
    noteElement.setAttribute( "noteType", m_noteType);
    qElement.appendChild( noteElement );
}

/**
 * Rename the text of the note widget.
 */
void NoteWidget::rename()
{
    NoteDialog * dlg = 0;
    UMLDoc *doc = UMLApp::app()->document();
    umlScene()->updateDocumentation(false);
    dlg = new NoteDialog(umlScene()->activeView(), this);
    if (dlg->exec()) {
        umlScene()->showDocumentation(this, true);
        doc->setModified(true);
        update();
    }
    delete dlg;
}

/**
 * Will be called when a menu selection has been made from the popup
 * menu.
 *
 * @param action   The action that has been selected.
 */
void NoteWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    switch(sel) {
    case ListPopupMenu::mt_Rename:
        rename();
        break;

    default:
        UMLWidget::slotMenuSelection(action);
        break;
    }
}

/**
 * Overrides method from UMLWidget.
 */
UMLSceneSize NoteWidget::minimumSize()
{
    int width = 60;
    int height = 30;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int textWidth = fm.width(documentation());
    if (m_noteType == PreCondition) {
        const int widthtemp = fm.width("<< precondition >>");
        width = textWidth > widthtemp ? textWidth : widthtemp;
        width += 10;
    }
    else if (m_noteType == PostCondition) {
        const int widthtemp = fm.width("<< postcondition >>");
        width = textWidth > widthtemp ? textWidth : widthtemp;
        width += 10;
    }
    else if (m_noteType == Transformation) {
        const int widthtemp = fm.width("<< transformation >>");
        width = textWidth > widthtemp ? textWidth : widthtemp;
        width += 10;
    }
    return UMLSceneSize(width, height);
}

/**
 * Overrides method from UMLWidget.
 */

/**
 * calculate content related size of widget.
 * Overrides method from UMLWidget.
 */
UMLSceneSize NoteWidget::calculateSize()
{
    int width = this->width();
    int height = this->height();
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int margin = fm.width("W");
    QSize size = fm.size (0, documentation());
    const int textWidth = size.width();
    if (m_noteType == PreCondition) {
        const int widthtemp = fm.width("<< precondition >>");
        width = textWidth > widthtemp ? textWidth : widthtemp;
        width += 2 * margin;
    }
    else if (m_noteType == PostCondition) {
        const int widthtemp = fm.width("<< postcondition >>");
        width = textWidth > widthtemp ? textWidth : widthtemp;
        width += 2 * margin;
    }
    else if (m_noteType == Transformation) {
        const int widthtemp = fm.width("<< transformation >>");
        width = textWidth > widthtemp ? textWidth : widthtemp;
        width += 2 * margin;
    }
    else {
    }
    return UMLSceneSize(width, height);
}

/**
 * Paints the text. Auxiliary to paint().
 * Implemented without word wrap.
 */
void NoteWidget::paintText(QPainter *painter)
{
    if (painter == 0) {
        return;
    }

    QString text = documentation();
    if (text.length() == 0) {
        return;
    }

    painter->setPen(Qt::black);
    QFont font = UMLWidget::font();
    painter->setFont(font);

    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const int margin      = fm.width("W");
    const QSize textSize  = fm.size(Qt::TextExpandTabs, text);

    const int width = this->width() - margin * 2;
    const int height = this->height() - fontHeight;
    int textY = fontHeight / 2;
    int textX = margin;

    if ((textSize.width() < width) && (textSize.height() < height)) {
        // the entire text is small enough - draw it
        painter->drawText(textX, textY,
                    textSize.width(), textSize.height(),
                    Qt::AlignLeft, text);
    }
    else {
        // not all text can be drawn
        QStringList lines = text.split(QChar('\n'));
        foreach(const QString& line, lines) {
            int lineWidth = fm.width(line);
            if (lineWidth < width) {
                // line is small enough - draw it
                painter->drawText(textX, textY,
                            textSize.width(), fontHeight,
                            Qt::AlignLeft, line);
            }
            else {
                // draw a smaller line
                for(int len = line.length(); len > 0; --len) {
                    QString smallerLine = line.left(len);
                    int smallerLineWidth = fm.width(smallerLine);
                    if (smallerLineWidth < width) {
                        // line is small enough - draw it
                        painter->drawText(textX, textY,
                                    width, fontHeight,
                                    Qt::AlignLeft, smallerLine);
                    }
                }
            }
            textY += fontHeight;
            if (textY > height) {
                // skip the next lines - size is not enough
                break;
            }
        }
    }
}

/**
 * Paints the text. Auxiliary to paint().
 * Implemented with word wrap.
 */
void NoteWidget::paintTextWordWrap(QPainter *painter)
{
    if (painter == 0) {
        return;
    }
    QString text = documentation();
    if (text.length() == 0) {
        return;
    }
    // Implement word wrap for text as follows:
    // wrap at width on whole words.
    // if word is wider than width then clip word
    // if reach height exit and don't print anymore
    // start new line on \n character
    painter->setPen( Qt::black );
    QFont font = UMLWidget::font();
    painter->setFont( font );
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    QString word = "";
    QString fullLine = "";
    QString testCombineLine = "";
    const int margin = fm.width( "W" );
    int textY = fontHeight / 2;
    int textX = margin;
    const int width = this -> width() - margin * 2;
    const int height = this -> height() - fontHeight;
    QChar returnChar('\n');
    QChar c;

    for (int i = 0; i <= text.length(); ++i) {
        if (i < text.length()) {
            c = text[i];
        } else {
            // all chars of text have been handled already ->
            // perform this last run to spool current content of "word"
            c = returnChar;
        }
        if (c == returnChar || c.isSpace()) {
            // new word delimiter found -> it is time to decide on word wrap
            testCombineLine = fullLine + ' ' + word;
            int textWidth = fm.width( testCombineLine );
            if (textX + textWidth > width) {
                // combination of "fullLine" and "word" doesn't fit into one line ->
                // print "fullLine" in current line, update write position to next line
                // and decide then on following actions
                painter->drawText(textX, textY,
                            textWidth, fontHeight, Qt::AlignLeft, fullLine );
                fullLine = word;
                word = "";
                // update write position
                textX = margin;
                textY += fontHeight;
                if (textY > height)
                    return;
                // in case of c==newline ->
                // print "word" and set write position one additional line lower
                if (c == returnChar) {
                    // print "word" - which is now "fullLine" and set to next line
                    painter->drawText(textX, textY,
                                textWidth, fontHeight, Qt::AlignLeft, fullLine);
                    fullLine = "";
                    textX = margin;
                    textY += fontHeight;
                    if( textY > height ) return;
                }
            }
            else if (c == returnChar) {
                // newline found and combination of "fullLine" and "word" fits
                // in one line
                painter->drawText(textX, textY,
                            textWidth, fontHeight, Qt::AlignLeft, testCombineLine);
                fullLine = word = "";
                textX = margin;
                textY += fontHeight;
                if (textY > height)
                    return;
            } else {
                // word delimiter found, and combination of "fullLine", space and "word" fits into one line
                fullLine = testCombineLine;
                word = "";
            }
        } else {
            // no word delimiter found --> add current char to "word"
            if (c != '\0')
                word += c;
        }
    }//end for
}

/**
 * Event handler for moude double click events.
 *
 * @param event The QGraphicsSceneMouseEvent event.
 */
void NoteWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_diagramLink == Uml::ID::None) {
            rename();
        } else {
            UMLDoc *umldoc = UMLApp::app()->document();
            umldoc->changeCurrentView(m_diagramLink);
        }
        event->accept();
    }
}

#include "notewidget.moc"
