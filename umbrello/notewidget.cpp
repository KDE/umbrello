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
//qt includes
#include <q3pointarray.h>
#include <qpainter.h>
#include <q3textedit.h>
#include <qframe.h>
//Added by qt3to4:
#include <QGraphicsSceneMouseEvent>
// kde includes
#include <kdebug.h>
#include <klocale.h>
#include <kcolordialog.h>
#include <kinputdialog.h>
// app includes
#include "notewidgetcontroller.h"
#include "dialogs/notedialog.h"
#include "umldoc.h"
#include "umlview.h"
#include "uml.h"
#include "listpopupmenu.h"
#include "dialog_utils.h"
#include "umlscene.h"

#define NOTEMARGIN 10

NoteWidget::NoteWidget(UMLScene *scene, NoteType noteType , Uml::IDType id)
        : UMLWidget(scene, id, new NoteWidgetController(this)) {
    init();
    m_NoteType = noteType;
    setSize(100,80);
    setZ( 20 ); //make sure always on top.

    // [PORT] We can easily use QGraphicsTextItem here. :-)
#ifdef NOTEWIDGET_EMBED_EDITOR
    // NB: This code is currently deactivated because
    // Zoom does not yet work with the embedded text editor.
    m_pEditor = new Q3TextEdit(view);
    m_pEditor->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    m_pEditor->setHScrollBarMode(Q3ScrollView::AlwaysOff);
    m_pEditor->setVScrollBarMode(Q3ScrollView::AlwaysOff);
    m_pEditor->setTextFormat(Qt::RichText);
    m_pEditor->setShown(true);
    setEditorGeometry();
    setNoteType(noteType);

    connect(m_pScene, SIGNAL(contentsMoving(int, int)),
            this, SLOT(slotViewScrolled(int, int)));
#endif

}

void NoteWidget::init() {
    UMLWidget::setBaseType(Uml::wt_Note);
    m_DiagramLink = Uml::id_None;
}

NoteWidget::NoteType NoteWidget::getNoteType() const {
    return m_NoteType;
}

NoteWidget::NoteType NoteWidget::getNoteType(const QString& noteType) const {
        if (noteType == "Precondition")
        return NoteWidget::PreCondition;
    else if (noteType == "Postcondition")
        return NoteWidget::PostCondition;
    else if (noteType == "Transformation")
        return NoteWidget::Transformation;
        else
                return NoteWidget::Normal;
}

void NoteWidget::setNoteType( NoteType noteType ) {
    m_NoteType = noteType;
}

void NoteWidget::setNoteType( const QString& noteType ) {
    setNoteType(getNoteType(noteType));
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
        uError() << "no view found for viewID " << ID2STR(viewID) << endl;
        return;
    }
    QString linkText("Diagram: " + view->umlScene()->getName());
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

void NoteWidget::setX( qreal x ) {
    UMLWidget::setX(x);
    setEditorGeometry();
}

void NoteWidget::setY( qreal y ) {
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
    qreal margin = 10;
    qreal w = getWidth()-1;

    qreal h= getHeight()-1;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const qreal fontHeight  = fm.lineSpacing();
    QPolygon poly(6);
    poly.setPoint(0, offsetX, offsetY);
    poly.setPoint(1, offsetX, offsetY + h);
    poly.setPoint(2, offsetX + w, offsetY + h);
    poly.setPoint(3, offsetX + w, offsetY + margin);
    poly.setPoint(4, offsetX + w - margin, offsetY);
    poly.setPoint(5, offsetX, offsetY);

    setPenFromSettings(p);
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
    p.setPen(Qt::black);
    switch(m_NoteType) {
    case NoteWidget::PreCondition :
        p.drawText(offsetX, offsetY + margin ,w, fontHeight, Qt::AlignCenter, "<< precondition >>");
        break;

    case NoteWidget::PostCondition :
        p.drawText(offsetX, offsetY + margin ,w, fontHeight, Qt::AlignCenter, "<< postcondition >>");
        break;

    case NoteWidget::Transformation :
        p.drawText(offsetX, offsetY + margin ,w, fontHeight, Qt::AlignCenter, "<< transformation >>");
        break;
        case NoteWidget::Normal :
    default :  break;
    }

if(isSelected()) {
        drawSelected(&p, offsetX, offsetY);
    }

    drawText(&p, offsetX, offsetY);
}

QSizeF NoteWidget::calculateSize() {
    qreal width = 50, height = 50;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    //const qreal fontHeight  = fm.lineSpacing();
    const qreal textWidth = fm.width(m_Text);
    if (m_NoteType == PreCondition)
    {
        const qreal widthtemp = fm.width("<< precondition >>");
                width = textWidth > widthtemp ? textWidth : widthtemp;
        width += 10;
    }
    else if (m_NoteType == PostCondition)
    {
        const qreal widthtemp = fm.width("<< postcondition >>");
                width = textWidth > widthtemp ? textWidth : widthtemp;
        width += 10;
    }
    else if (m_NoteType == Transformation)
    {
        const qreal widthtemp = fm.width("<< transformation >>");
                width = textWidth > widthtemp ? textWidth : widthtemp;
        width += 10;
    }
    return QSizeF(width, height);
}

void NoteWidget::slotMenuSelection(QAction* action) {
    NoteDialog * dlg = 0;
    UMLDoc *doc = UMLApp::app()->getDocument();
    ListPopupMenu::Menu_Type sel = m_pMenu->getMenuType(action);
    switch(sel) {
        ///OBSOLETE - remove ListPopupMenu::mt_Link_Docs
        // case ListPopupMenu::mt_Link_Docs:
        //      m_pScene->updateNoteWidgets();
        //      doc -> setModified(true);
        //      break;

    case ListPopupMenu::mt_Rename:
        m_pScene -> updateDocumentation( false );
        dlg = new NoteDialog( m_pScene->activeView(), this );
        if( dlg -> exec() ) {
            m_pScene -> showDocumentation( this, true );
            doc -> setModified(true);
            update();
        }
        delete dlg;
        break;

    default:
        UMLWidget::slotMenuSelection(action);
        break;
    }
}

void NoteWidget::drawText(QPainter * p /*=NULL*/, qreal offsetX /*=0*/, qreal offsetY /*=0*/) {
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
    const qreal fontHeight  = fm.lineSpacing();
    QString text = getDoc();
    if( text.length() == 0 )
        return;
    QString word = "";
    QString fullLine = "";
    QString testCombineLine = "";
    const qreal margin = fm.width( "W" );
    qreal textY = fontHeight / 2;
    qreal textX = margin;
    const qreal width = this -> getWidth() - margin * 2;
    const qreal height = this -> getHeight() - fontHeight;
    QChar returnChar('\n');
    QChar c;


 //   QString text = getDoc();
    //QString text = l_Type + "\n" + m_Text;
    if( text.length() == 0 )
        return;

    for (int i = 0; i <= text.length(); i++) {
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
            qreal textWidth = fm.width( testCombineLine );
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

void NoteWidget::askForNoteType(UMLWidget* &targetWidget) {

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

void NoteWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement noteElement = qDoc.createElement( "notewidget" );
    UMLWidget::saveToXMI( qDoc, noteElement );
    noteElement.setAttribute( "text", getDoc() );
    if (m_DiagramLink != Uml::id_None)
        noteElement.setAttribute( "diagramlink", ID2STR(m_DiagramLink) );
    noteElement.setAttribute( "noteType", m_NoteType);
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
    QString type = qElement.attribute("noteType", "");
    setNoteType( (NoteType)type.toInt() );
    return true;
}


#include "notewidget.moc"

