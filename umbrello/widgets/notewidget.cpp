/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "notewidget.h"
//qt includes
#include <QtGui/QPainter>
#include <QtGui/QFrame>
// kde includes
#include <klocale.h>
#include <kcolordialog.h>
#include <kinputdialog.h>
// app includes
#include "debug_utils.h"
#include "dialog_utils.h"
#include "listpopupmenu.h"
#include "notewidgetcontroller.h"
#include "notedialog.h"
#include "umldoc.h"
#include "umlview.h"
#include "uml.h"

NoteWidget::NoteWidget(UMLView * view, NoteType noteType , Uml::IDType id)
  : UMLWidget(view, id, new NoteWidgetController(this))
{
    UMLWidget::setBaseType(WidgetBase::wt_Note);
    m_DiagramLink = Uml::id_None;
    m_NoteType = noteType;
    setZ(20); //make sure always on top.
}

NoteWidget::NoteType NoteWidget::getNoteType() const
{
    return m_NoteType;
}

NoteWidget::NoteType NoteWidget::getNoteType(const QString& noteType) const
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

void NoteWidget::setNoteType( NoteType noteType )
{
    m_NoteType = noteType;
}

void NoteWidget::setNoteType( const QString& noteType )
{
    setNoteType(getNoteType(noteType));
}

NoteWidget::~NoteWidget()
{
}

void NoteWidget::setDiagramLink(Uml::IDType viewID)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    UMLView *view = umldoc->findView(viewID);
    if (view == NULL) {
        uError() << "no view found for viewID " << ID2STR(viewID);
        return;
    }
    QString linkText("Diagram: " + view->name());
    setDocumentation(linkText);
    m_DiagramLink = viewID;
}

Uml::IDType NoteWidget::getDiagramLink() const
{
    return m_DiagramLink;
}

void NoteWidget::setFont(QFont font)
{
    UMLWidget::setFont(font);
}

void NoteWidget::setX( int x )
{
    UMLWidget::setX(x);
}

void NoteWidget::setY( int y )
{
    UMLWidget::setY(y);
}

QString NoteWidget::documentation() const
{
    return m_Text;
}

void NoteWidget::setDocumentation(const QString &newText)
{
    m_Text = newText;
    update();
}

void NoteWidget::draw(QPainter & p, int offsetX, int offsetY)
{
    const int margin = 10;
    int w = width()-1;
    int h = height()-1;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    QPolygon poly(6);
    poly.setPoint(0, offsetX, offsetY);
    poly.setPoint(1, offsetX, offsetY + h);
    poly.setPoint(2, offsetX + w, offsetY + h);
    poly.setPoint(3, offsetX + w, offsetY + margin);
    poly.setPoint(4, offsetX + w - margin, offsetY);
    poly.setPoint(5, offsetX, offsetY);

    setPenFromSettings(p);
    if ( UMLWidget::getUseFillColour() ) {
        QBrush brush( UMLWidget::getFillColor() );
        p.setBrush(brush);
        p.drawPolygon(poly);
    } else
        p.drawPolyline(poly);
    p.drawLine(offsetX + w - margin, offsetY, offsetX + w - margin, offsetY + margin);
    p.drawLine(offsetX + w - margin, offsetY + margin, offsetX + w, offsetY + margin);
    p.setPen(Qt::black);
    switch(m_NoteType) {
    case NoteWidget::PreCondition :
        p.drawText(offsetX, offsetY + margin, w, fontHeight, Qt::AlignCenter, "<< precondition >>");
        break;
    case NoteWidget::PostCondition :
        p.drawText(offsetX, offsetY + margin, w, fontHeight, Qt::AlignCenter, "<< postcondition >>");
        break;
    case NoteWidget::Transformation :
        p.drawText(offsetX, offsetY + margin, w, fontHeight, Qt::AlignCenter, "<< transformation >>");
        break;
    case NoteWidget::Normal :
    default :
        break;
    }

    if (m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }

//    drawText(&p, offsetX, offsetY);
    drawTextWordWrap(&p, offsetX, offsetY);
}

QSize NoteWidget::calculateSize()
{
    int width = 60;
    int height = 30;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int textWidth = fm.width(m_Text);
    if (m_NoteType == PreCondition) {
        const int widthtemp = fm.width("<< precondition >>");
        width = textWidth > widthtemp ? textWidth : widthtemp;
        width += 10;
    }
    else if (m_NoteType == PostCondition) {
        const int widthtemp = fm.width("<< postcondition >>");
        width = textWidth > widthtemp ? textWidth : widthtemp;
        width += 10;
    }
    else if (m_NoteType == Transformation) {
        const int widthtemp = fm.width("<< transformation >>");
        width = textWidth > widthtemp ? textWidth : widthtemp;
        width += 10;
    }
    else {
        // keep width and height unchanged
    }
    return QSize(width, height);
}

void NoteWidget::slotMenuSelection(QAction* action)
{
    NoteDialog * dlg = 0;
    UMLDoc *doc = UMLApp::app()->document();
    ListPopupMenu::Menu_Type sel = m_pMenu->getMenuType(action);
    switch(sel) {
    case ListPopupMenu::mt_Rename:
        m_pView->updateDocumentation( false );
        dlg = new NoteDialog( m_pView, this );
        if( dlg->exec() ) {
            m_pView->showDocumentation( this, true );
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

/**
 * Draws the text. Auxiliary to draw().
 * Implemented without word wrap.
 */
void NoteWidget::drawText(QPainter * p, int offsetX, int offsetY)
{
    if (p == 0) {
        return;
    }

    QString text = documentation();
    if (text.length() == 0) {
        return;
    }

    p->setPen(Qt::black);
    QFont font = UMLWidget::font();
    p->setFont(font);

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
        p->drawText(offsetX + textX, offsetY + textY,
                    textSize.width(), textSize.height(),
                    Qt::AlignLeft, text);
    }
    else {
        // not all text can be drawn
        QStringList lines = text.split(QChar('\n'));
        foreach(QString line, lines) {
            int lineWidth = fm.width(line);
            if (lineWidth < width) {
                // line is small enough - draw it
                p->drawText(offsetX + textX, offsetY + textY,
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
                        p->drawText(offsetX + textX, offsetY + textY,
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
 * Draws the text. Auxiliary to draw().
 * Implemented with word wrap.
 */
void NoteWidget::drawTextWordWrap(QPainter * p, int offsetX, int offsetY)
{
    if (p == 0) {
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
    p->setPen( Qt::black );
    QFont font = UMLWidget::font();
    p->setFont( font );
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
                p->drawText(offsetX + textX, offsetY + textY,
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
                    p->drawText(offsetX + textX, offsetY + textY,
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
                p->drawText(offsetX + textX, offsetY + textY,
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

void NoteWidget::askForNoteType(UMLWidget* &targetWidget)
{
    bool pressedOK = false;
    const QStringList list = QStringList() << "Precondition" << "Postcondition" << "Transformation";
    QString type = KInputDialog::getItem (i18n("Note Type"), i18n("Select the Note Type"), list, 0, false, &pressedOK, UMLApp::app());

    if (pressedOK) {
        dynamic_cast<NoteWidget*>(targetWidget)->setNoteType(type);
    } else {
        targetWidget->cleanup();
        delete targetWidget;
        targetWidget = NULL;
    }
}

void NoteWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement noteElement = qDoc.createElement( "notewidget" );
    UMLWidget::saveToXMI( qDoc, noteElement );
    noteElement.setAttribute( "text", documentation() );
    if (m_DiagramLink != Uml::id_None)
        noteElement.setAttribute( "diagramlink", ID2STR(m_DiagramLink) );
    noteElement.setAttribute( "noteType", m_NoteType);
    qElement.appendChild( noteElement );
}

bool NoteWidget::loadFromXMI( QDomElement & qElement )
{
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    setZ( 20 ); //make sure always on top.
    setDocumentation( qElement.attribute("text", "") );
    QString diagramlink = qElement.attribute("diagramlink", "");
    if (!diagramlink.isEmpty())
        m_DiagramLink = STR2ID(diagramlink);
    QString type = qElement.attribute("noteType", "");
    setNoteType( (NoteType)type.toInt() );
    return true;
}

#include "notewidget.moc"

