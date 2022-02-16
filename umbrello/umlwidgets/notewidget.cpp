/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "notewidget.h"

// app includes
#include "debug_utils.h"
#include "dialog_utils.h"
#include "docwindow.h"
#include "listpopupmenu.h"
#include "notedialog.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "widget_utils.h"

// kde includes
#include <KLocalizedString>
#if QT_VERSION < 0x050000
#include <kcolordialog.h>
#endif

// qt includes
#include <QPainter>
#if QT_VERSION >= 0x050000
#include <QColorDialog>
#endif
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(NoteWidget)

QPointer<NoteWidget> NoteWidget::s_pCurrentNote;

/**
 * Constructs a NoteWidget.
 *
 * @param scene      The parent to this widget.
 * @param noteType   The NoteWidget::NoteType of this NoteWidget
 * @param id         The unique id of the widget.
 *                   The default (-1) will prompt a new ID.
 */
NoteWidget::NoteWidget(UMLScene * scene, NoteType noteType , Uml::ID::Type id)
  : UMLWidget(scene, WidgetBase::wt_Note, id),
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

    int w = width();
    int h = height();
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    QPolygon poly(6);
    poly.setPoint(0, 0, 0);
    poly.setPoint(1, 0, h);
    poly.setPoint(2, w, h);
    poly.setPoint(3, w, defaultMargin );
    poly.setPoint(4, w - defaultMargin , 0);
    poly.setPoint(5, 0, 0);

    setPenFromSettings(painter);
    if (UMLWidget::useFillColor()) {
        QBrush brush(UMLWidget::fillColor());
        painter->setBrush(brush);
        painter->drawPolygon(poly);
    } else
        painter->drawPolyline(poly);
    painter->drawLine(w - defaultMargin , 0, w - defaultMargin , defaultMargin );
    painter->drawLine(w - defaultMargin , defaultMargin , w, defaultMargin );
    painter->setPen(textColor());
    switch(m_noteType) {
    case NoteWidget::PreCondition :
        painter->drawText(0, defaultMargin , w, fontHeight, Qt::AlignCenter,
                          Widget_Utils::adornStereo(QLatin1String("precondition")));
        break;
    case NoteWidget::PostCondition :
        painter->drawText(0, defaultMargin , w, fontHeight, Qt::AlignCenter,
                          Widget_Utils::adornStereo(QLatin1String("postcondition")));
        break;
    case NoteWidget::Transformation :
        painter->drawText(0, defaultMargin , w, fontHeight, Qt::AlignCenter,
                          Widget_Utils::adornStereo(QLatin1String("transformation")));
        break;
    case NoteWidget::Normal :
    default :
        break;
    }

    UMLWidget::paint(painter, option, widget);

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
    if (noteType == QLatin1String("Precondition"))
        return NoteWidget::PreCondition;
    else if (noteType == QLatin1String("Postcondition"))
        return NoteWidget::PostCondition;
    else if (noteType == QLatin1String("Transformation"))
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
 * @return  ID of a UMLView, or Uml::ID::None if no
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
 * @param viewID    ID of a UMLScene.
 */
void NoteWidget::setDiagramLink(Uml::ID::Type viewID)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    UMLView *view = umldoc->findView(viewID);
    if (view == 0) {
        logError1("NoteWidget::setDiagramLink: no view found for viewID %1",
                  Uml::ID::toString(viewID));
        return;
    }
    QString linkText(QLatin1String("Diagram: ") + view->umlScene()->name());
    setDocumentation(linkText);
    m_diagramLink = viewID;
    update();
}

/**
 * Set the given diagram as hyperlinked to this note.
 * If the diagram name is empty or the related view
 * could not be found, the link will be removed.
 *
 * @param diagramName name of diagram to link to
 * @return true  if link could be added
 * @return false if link could not be added
 */
bool NoteWidget::setDiagramLink(const QString &diagramName)
{
    if (diagramName.isEmpty()) {
        m_diagramLink = Uml::ID::None;
        return true;
    }

    UMLDoc *umldoc = UMLApp::app()->document();
    UMLView *view = 0;
    for (int i = 1; i < Uml::DiagramType::N_DIAGRAMTYPES; ++i) {
        Uml::DiagramType::Enum dt = Uml::DiagramType::fromInt(i);
        view = umldoc->findView(dt, diagramName, true);
        if (view)
            break;
    }
    if (view == 0 || view->umlScene() == 0) {
        m_diagramLink = Uml::ID::None;
        return false;
    }
    m_diagramLink = view->umlScene()->ID();
    update();
    return true;
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

#if QT_VERSION >= 0x050000
    QString type = QInputDialog::getItem (UMLApp::app(),
                                          i18n("Note Type"), i18n("Select the Note Type"), list,
                                          0, false, &pressedOK);
#else
    QString type = KInputDialog::getItem (i18n("Note Type"), i18n("Select the Note Type"), list,
                                          0, false, &pressedOK, UMLApp::app());
#endif

    if (pressedOK) {
        targetWidget->asNoteWidget()->setNoteType(type);
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
    setDocumentation(qElement.attribute(QLatin1String("text")));
    QString diagramlink = qElement.attribute(QLatin1String("diagramlink"));
    if (!diagramlink.isEmpty())
        m_diagramLink = Uml::ID::fromString(diagramlink);
    QString type = qElement.attribute(QLatin1String("noteType"));
    setNoteType((NoteType)type.toInt());
    return true;
}

/**
 * Saves to the "notewidget" XMI element.
 */
void NoteWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("notewidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeAttribute(QLatin1String("text"), documentation());
    if (m_diagramLink != Uml::ID::None)
        writer.writeAttribute(QLatin1String("diagramlink"), Uml::ID::toString(m_diagramLink));
    writer.writeAttribute(QLatin1String("noteType"), QString::number(m_noteType));
    writer.writeEndElement();
}

/**
 * Show a properties dialog for a NoteWidget.
 */
bool NoteWidget::showPropertiesDialog()
{
    bool result = false;
    NoteDialog * dlg = 0;
    UMLApp::app()->docWindow()->updateDocumentation(false);
    dlg = new NoteDialog(umlScene()->activeView(), this);
    if (dlg->exec()) {
        UMLApp::app()->docWindow()->showDocumentation(this, true);
        UMLApp::app()->document()->setModified(true);
        update();
        result = true;
    }
    delete dlg;
    return result;
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
    case ListPopupMenu::mt_Properties:
        showPropertiesDialog();
        break;

    default:
        UMLWidget::slotMenuSelection(action);
        break;
    }
}

/**
 * Overrides method from UMLWidget.
 */
QSizeF NoteWidget::minimumSize() const
{
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    return QSizeF(fm.averageCharWidth()*20, fm.height() * 2);
}

/**
 * Calculate content related size of widget.
 * Overrides method from UMLWidget.
 */
QSizeF NoteWidget::calculateSize(bool withExtensions /* = true */) const
{
    Q_UNUSED(withExtensions)
    int width = 0;
    int height = this->height();
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    QSize size = fm.size(Qt::TextExpandTabs, documentation());
    const int textWidth = size.width();
    const int textHeight = size.height();
    if (m_noteType == PreCondition) {
        const int widthtemp = fm.width(Widget_Utils::adornStereo(QLatin1String("precondition")));
        width = textWidth > widthtemp ? textWidth : widthtemp;
    }
    else if (m_noteType == PostCondition) {
        const int widthtemp = fm.width(Widget_Utils::adornStereo(QLatin1String("postcondition")));
        width = textWidth > widthtemp ? textWidth : widthtemp;
    }
    else if (m_noteType == Transformation) {
        const int widthtemp = fm.width(Widget_Utils::adornStereo(QLatin1String("transformation")));
        width = textWidth > widthtemp ? textWidth : widthtemp;
    }

    if (textWidth > width)
        width = textWidth;
    if (textHeight > height)
        height = textHeight;
    width += 2 * defaultMargin;

    return QSizeF(width, height);
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
    const QSize textSize  = fm.size(Qt::TextExpandTabs, text);

    const int width = this->width() - defaultMargin * 2;
    const int height = this->height() - fontHeight;
    int textY = fontHeight / 2;
    int textX = defaultMargin;

    if ((textSize.width() < width) && (textSize.height() < height)) {
        // the entire text is small enough - draw it
        painter->drawText(textX, textY,
                    textSize.width(), textSize.height(),
                    Qt::AlignLeft, text);
    }
    else {
        // not all text can be drawn
        QStringList lines = text.split(QLatin1Char('\n'));
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
    painter->setPen(Qt::black);
    QFont font = UMLWidget::font();
    painter->setFont(font);
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    QString word;
    QString fullLine;
    QString testCombineLine;
    int textY = fontHeight / 2;
    int textX = defaultMargin;
    const int width = this -> width() - defaultMargin * 2;
    const int height = this -> height() - fontHeight;
    QChar returnChar = QChar::fromLatin1('\n');
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
            testCombineLine = fullLine + QLatin1Char(' ') + word;
            int textWidth = fm.width(testCombineLine);
            if (textX + textWidth > width) {
                // combination of "fullLine" and "word" doesn't fit into one line ->
                // print "fullLine" in current line, update write position to next line
                // and decide then on following actions
                painter->drawText(textX, textY,
                            textWidth, fontHeight, Qt::AlignLeft, fullLine);
                fullLine = word;
                word = QString();
                // update write position
                textX = defaultMargin;
                textY += fontHeight;
                if (textY > height)
                    return;
                // in case of c==newline ->
                // print "word" and set write position one additional line lower
                if (c == returnChar) {
                    // print "word" - which is now "fullLine" and set to next line
                    painter->drawText(textX, textY,
                                textWidth, fontHeight, Qt::AlignLeft, fullLine);
                    fullLine = QString();
                    textX = defaultMargin;
                    textY += fontHeight;
                    if(textY > height) return;
                }
            }
            else if (c == returnChar) {
                // newline found and combination of "fullLine" and "word" fits
                // in one line
                painter->drawText(textX, textY,
                            textWidth, fontHeight, Qt::AlignLeft, testCombineLine);
                fullLine = word = QString();
                textX = defaultMargin;
                textY += fontHeight;
                if (textY > height)
                    return;
            } else {
                // word delimiter found, and combination of "fullLine", space and "word" fits into one line
                fullLine = testCombineLine;
                word = QString();
            }
        } else {
            // no word delimiter found --> add current char to "word"
            if (c != QLatin1Char('\0'))
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
    if (event->button() == Qt::LeftButton && m_diagramLink != Uml::ID::None) {
        UMLDoc *umldoc = UMLApp::app()->document();
        umldoc->changeCurrentView(m_diagramLink);
        event->accept();
    } else
        showPropertiesDialog();
}

