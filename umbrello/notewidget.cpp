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

// own header
#include "notewidget.h"
//qt includes
#include <qpointarray.h>
#include <qpainter.h>
#include <qtextedit.h>
#include <qframe.h>
// kde includes
#include <kdebug.h>
#include <kcolordialog.h>
// app includes
#include "notewidgetcontroller.h"
#include "dialogs/notedialog.h"
#include "clipboard/umldrag.h"
#include "umldoc.h"
#include "umlview.h"
#include "uml.h"
#include "listpopupmenu.h"

#define NOTEMARGIN 10

NoteWidget::NoteWidget(UMLView * view, Uml::IDType id)
        : UMLWidget(view, id, new NoteWidgetController(this)) {
    init();
    setSize(100,80);
    setZ( 20 ); //make sure always on top.
#ifdef NOTEWIDGET_EMBED_EDITOR
    // NB: This code is currently deactivated because
    // Zoom does not yet work with the embedded text editor.
    m_pEditor = new QTextEdit(view);
    m_pEditor->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    m_pEditor->setHScrollBarMode(QScrollView::AlwaysOff);
    m_pEditor->setVScrollBarMode(QScrollView::AlwaysOff);
    m_pEditor->setTextFormat(Qt::RichText);
    m_pEditor->setShown(true);
    setEditorGeometry();
    connect(m_pView, SIGNAL(contentsMoving(int, int)),
            this, SLOT(slotViewScrolled(int, int)));
#endif
}

void NoteWidget::init() {
    UMLWidget::setBaseType(Uml::wt_Note);
    m_DiagramLink = Uml::id_None;
}

NoteWidget::~NoteWidget() {
#ifdef NOTEWIDGET_EMBED_EDITOR
    delete m_pEditor;
#endif
}

void NoteWidget::setDiagramLink(Uml::IDType viewID) {
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    UMLView *view = umldoc->findView(viewID);
    if (view == NULL) {
        kError() << "NoteWidget::setDiagramLink(" << ID2STR(viewID)
        << "): no view found for this ID." << endl;
        return;
    }
    QString linkText("Diagram: " + view->getName());
#if defined (NOTEWIDGET_EMBED_EDITOR)
    m_pEditor->setUnderline(true);
    m_pEditor->insert(linkText);
    m_pEditor->setUnderline(false);
#else
    setDoc(linkText);
    update();
#endif
    m_DiagramLink = viewID;
}

Uml::IDType NoteWidget::getDiagramLink() const {
    return m_DiagramLink;
}

void NoteWidget::slotViewScrolled(int x, int y) {
    setEditorGeometry(x, y);
}

void NoteWidget::setFont(QFont font) {
    UMLWidget::setFont(font);
#ifdef NOTEWIDGET_EMBED_EDITOR
    m_pEditor->setFont(font);
#endif
}

void NoteWidget::setEditorGeometry(int dx /*=0*/, int dy /*=0*/) {
#if defined (NOTEWIDGET_EMBED_EDITOR)
    const QRect editorGeometry( UMLWidget::getX() - dx + 6,
                                UMLWidget::getY() - dy + 10,
                                UMLWidget::getWidth() - 16,
                                UMLWidget::getHeight() - 16);
    m_pEditor->setGeometry( editorGeometry );
    drawText();
#else
    dx=0; dy=0;   // avoid "unused arg" warnings
#endif
}

void NoteWidget::setX( int x ) {
    UMLWidget::setX(x);
    setEditorGeometry();
}

void NoteWidget::setY( int y ) {
    UMLWidget::setY(y);
    setEditorGeometry();
}

QString NoteWidget::getDoc() const {
#if defined (NOTEWIDGET_EMBED_EDITOR)
    return m_pEditor->text();
#else
    return m_Text;
#endif
}

void NoteWidget::setDoc(const QString &newText) {
#if defined (NOTEWIDGET_EMBED_EDITOR)
    m_pEditor->setText(newText);
#else
    m_Text = newText;
#endif
}

void NoteWidget::draw(QPainter & p, int offsetX, int offsetY) {
    int margin = 10;
    int w = width()-1;

    int h= height()-1;
    QPointArray poly(6);
    poly.setPoint(0, offsetX, offsetY);
    poly.setPoint(1, offsetX, offsetY + h);
    poly.setPoint(2, offsetX + w, offsetY + h);
    poly.setPoint(3, offsetX + w, offsetY + margin);
    poly.setPoint(4, offsetX + w - margin, offsetY);
    poly.setPoint(5, offsetX, offsetY);
    UMLWidget::setPen(p);
    if ( UMLWidget::getUseFillColour() ) {
        QBrush brush( UMLWidget::getFillColour() );
        p.setBrush(brush);
        p.drawPolygon(poly);
#if defined (NOTEWIDGET_EMBED_EDITOR)
        m_pEditor->setPaper(brush);
#endif
    } else
        p.drawPolyline(poly);
    p.drawLine(offsetX + w - margin, offsetY, offsetX + w - margin, offsetY + margin);
    p.drawLine(offsetX + w - margin, offsetY + margin, offsetX + w, offsetY + margin);
    if(m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }

    drawText(&p, offsetX, offsetY);
}

QSize NoteWidget::calculateSize() {
    return QSize(50, 50);
}

void NoteWidget::slotMenuSelection(int sel) {
    NoteDialog * dlg = 0;
    UMLDoc *doc = UMLApp::app()->getDocument();
    switch(sel) {
        ///OBSOLETE - remove ListPopupMenu::mt_Link_Docs
        // case ListPopupMenu::mt_Link_Docs:
        //      m_pView->updateNoteWidgets();
        //      doc -> setModified(true);
        //      break;

    case ListPopupMenu::mt_Rename:
        m_pView -> updateDocumentation( false );
        dlg = new NoteDialog( m_pView, this );
        if( dlg -> exec() ) {
            m_pView -> showDocumentation( this, true );
            doc -> setModified(true);
            update();
        }
        delete dlg;
        break;

    default:
        UMLWidget::slotMenuSelection(sel);
        break;
    }
}

void NoteWidget::drawText(QPainter * p /*=NULL*/, int offsetX /*=0*/, int offsetY /*=0*/) {
#if defined (NOTEWIDGET_EMBED_EDITOR)
    m_pEditor->setText( getDoc() );
    m_pEditor->setShown(true);
    m_pEditor->repaint();
#else
    if (p == NULL)
        return;
    /*
    Implement word wrap for text as follows:
    wrap at width on whole words.
    if word is wider than width then clip word
    if reach height exit and don't print anymore
    start new line on \n character
    */
    p->setPen( Qt::black );
    QFont font = UMLWidget::getFont();
    p->setFont( font );
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    QString text = getDoc();
    if( text.length() == 0 )
        return;
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
    for (uint i = 0; i <= text.length(); i++) {
        if (i < text.length()) {
            c = text[i];
        } else {
            // all chars of text have been handled already ->
            // perform this last run to spool current content of "word"
            c = returnChar;
        }
        if (c == returnChar || c.isSpace()) {
            // new word delimiter found -> its time to decide on word wrap
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
            else if ( c == returnChar ) {
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
#endif
}

void NoteWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement noteElement = qDoc.createElement( "notewidget" );
    UMLWidget::saveToXMI( qDoc, noteElement );
    noteElement.setAttribute( "text", getDoc() );
    if (m_DiagramLink != Uml::id_None)
        noteElement.setAttribute( "diagramlink", ID2STR(m_DiagramLink) );
    qElement.appendChild( noteElement );
}

bool NoteWidget::loadFromXMI( QDomElement & qElement ) {
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    setZ( 20 ); //make sure always on top.
    setDoc( qElement.attribute("text", "") );
    QString diagramlink = qElement.attribute("diagramlink", "");
    if (!diagramlink.isEmpty())
        m_DiagramLink = STR2ID(diagramlink);
    return true;
}


#include "notewidget.moc"

