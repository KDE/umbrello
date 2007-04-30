
/***************************************************************************
                          codeviewerdialog.cpp  -  description
                             -------------------
    begin                : Fri Aug 1 2003
    copyright            : (C) 2003 by Brian Thomas
    email                : brian.thomas@gsfc.nasa.gov
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codeeditor.h"

// qt/kde includes
#include <qkeysequence.h>
#include <qcursor.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qbrush.h>
#include <qlayout.h>
#include <qregexp.h>
#include <kdebug.h>
#include <klocale.h>

// local includes
#include "../attribute.h"
#include "../classifier.h"
#include "../umldoc.h"
#include "../umlrole.h"

#include "../codeaccessormethod.h"
#include "../codegenerator.h"
#include "../codeclassfield.h"
#include "../codeclassfielddeclarationblock.h"
#include "../codedocument.h"
#include "../codeoperation.h"
#include "../codemethodblock.h"
#include "../classifiercodedocument.h"
#include "../ownedhierarchicalcodeblock.h"
#include "../codegenerators/codegenfactory.h"

#include "codeviewerdialog.h"
#include "classpropdlg.h"
#include "umlattributedialog.h"
#include "umlroledialog.h"
#include "umloperationdialog.h"

CodeEditor::CodeEditor ( const QString & text, const QString & context, CodeViewerDialog * parent, const char * name , CodeDocument * doc)
        : QTextEdit ( text, context, parent, name)
{
    init(parent, doc);
}

CodeEditor::CodeEditor ( CodeViewerDialog * parent, const char* name, CodeDocument * doc )
        : QTextEdit ( parent, name )
{
    init(parent, doc);
}

/*
 *  Destroys the object and frees any allocated resources
 */
CodeEditor::~CodeEditor() { }

// clear the display of all text
void CodeEditor::clearText () {

    //        setCaption( tr2i18n("") );
    m_selectedTextBlock = 0;
    m_textBlockList.clear();
    m_tbInfoMap->clear();

    // now call super-class
    clear();

}

Settings::CodeViewerState CodeEditor::getState()
{
    return m_parentDlg->getState();
}

QLabel * CodeEditor::getComponentLabel() {
    return m_parentDlg->componentLabel;
}


// FIX: used only for debugging right now.. eliminate eventually -b.t.
void CodeEditor::clicked(int para, int pos)
{
    getComponentLabel()->setText("para:"+QString::number(para)+" pos:"+QString::number(pos));
}


bool CodeEditor::close ( bool alsoDelete )
{

    // capture last code block, if it exists
    if(m_lastTextBlockToBeEdited)
    {
        updateTextBlockFromText (m_lastTextBlockToBeEdited);
        m_lastTextBlockToBeEdited = 0;
    }

    return QTextEdit::close(alsoDelete);

}

void CodeEditor::doubleClicked(int para, int pos)
{

    m_lastPara = para;
    m_lastPos = pos;

    // ugh. more ugliness. We want to be able to call up the
    // correct editing dialog for the given attribute.
    TextBlock * tBlock = m_textBlockList.at(para);
    editTextBlock(tBlock, para);

}

// allow us to edit, as appropriate, the parent UMLObject of the
// given text block.
void CodeEditor::editTextBlock(TextBlock * tBlock, int para) {

    if(tBlock)
    {
        TextBlockInfo *info = (*m_tbInfoMap)[tBlock];
        if(info) {
            UMLObject *obj = info->getParent();
            if(obj)
            {

                UMLAttribute * at = 0;
                UMLRole * role = 0;
                UMLOperation * op = 0;

                if( (at = dynamic_cast<UMLAttribute*>(obj)) )
                {
                    UMLAttributeDialog dlg( this, at);
                    if( dlg.exec() ) { rebuildView(para); }
                }
                else if( (dynamic_cast<UMLClassifier*>(obj)) )
                {
                    if (obj->showProperties())
                        rebuildView(para);
                }
                else if( (role = dynamic_cast<UMLRole*>(obj)))
                {
                    UMLRoleDialog dlg(this,role);
                    if( dlg.exec() ) { rebuildView(para); }
                }
                else if( (op = dynamic_cast<UMLOperation*>(obj)) )
                    //else if( (cop = dynamic_cast<CodeOperation*>(tBlock)) )
                {
                    UMLOperationDialog dlg(this,op);
                    if( dlg.exec() ) { rebuildView(para); }
                }
                else
                {
                    kError()<<" CodeViewerDlg ERROR: UNKNOWN parent for textBlock"<<endl;
                }

            }
        }
    }
}

// return whether is empty or just whitespace
bool CodeEditor::StringIsBlank(const QString &str)
{
    if(str.isEmpty() || str.stripWhiteSpace().isEmpty())
        return true;
    return false;
}

void CodeEditor::keyPressEvent ( QKeyEvent * e ) {

    // kDebug() <<"KEY PRESS EVENT:["<<e->text().latin1()<<"] ascii CODE:"<<e->ascii();

    if((e->ascii() == 8) ) // || (e->ascii() == 127)) // what about delete?
        m_backspacePressed = true;

    // Q: can the MAC or WIN/DOS sequences occur?
    if((e->ascii() == 10) || (e->ascii() == 13) || (e->text() == "\r\n"))
        m_newLinePressed = true;

    QTextEdit::keyPressEvent(e);
}

void CodeEditor::loadFromDocument ()
{

    // clear the tool
    clearText();

    // set caption on tool
    QString caption = m_parentDoc->getFileName() + m_parentDoc->getFileExtension();
    setCaption( tr2i18n( caption.latin1() ) );

    // header for document
    QString header = m_parentDoc->getHeader()->toString();
    QString componentName = QString("header for file ") +caption;
    if(!StringIsBlank(header))
        insert(header,m_parentDoc->getHeader(),false,getState().fontColor,
               getState().nonEditBlockColor,0,componentName);

    // now all the text blocks in the document
    TextBlockList * items = m_parentDoc->getTextBlockList();
    appendText(items);

    setCursorPosition(0,0);

}

void CodeEditor::insert (const QString & text, TextBlock * parent, bool editable, const QColor & fgcolor, const QColor & bgcolor, UMLObject * umlobj, const QString & displayName, int startLine)
{

    // set some params
    bool isInsert = false;
    setColor(fgcolor);

    // its an append op if startLine is -1, otherwise its
    // an actual insert, which is more complicated
    if(startLine == -1)
    {
        startLine = paragraphs()-1;
        QTextEdit::append(text); // put actual text in. Use insert instead of append so history is preserved?
    }
    else
    {
        isInsert = true;
        QTextEdit::insertAt(text, startLine, 0);
    }

    // actual put in text

    // now do 'paragraph' background highlighting
    //        int endLine = paragraphs()-2;
    int endLine = text.contains(QRegExp("\n")) + startLine -1;
    if(m_isHighlighted)
        for(int para=startLine;para<=endLine;para++)
            setParagraphBackgroundColor(para,bgcolor);

    // record paragraph information
    // Did we already start recording info for this parent object?
    TextBlockInfo * tbinfo;
    if(m_tbInfoMap->contains(parent))
        tbinfo = (*m_tbInfoMap)[parent];
    else {
        tbinfo = new TextBlockInfo();
        tbinfo->displayName = displayName;
        tbinfo->isCodeAccessorMethod = dynamic_cast<CodeAccessorMethod*>(parent) ? true : false;
        m_tbInfoMap->insert(parent,tbinfo);
    }

    // set a parent, if its not already set
    if(umlobj && !tbinfo->getParent())
    {
        tbinfo->displayName = displayName;
        tbinfo->setParent(umlobj);
        tbinfo->isClickable = textBlockIsClickable(umlobj);
    }

    // now mark all lines that we just inserted as belonging to the parent
    for(int para=startLine;para<=endLine;para++)
        m_textBlockList.insert(para,parent);

    // lastly, update the para info
    // start position is relative to the FIRST parent position
    int start = startLine - m_textBlockList.findRef(parent);
    int size = endLine-startLine;

    // create the object that records this particular "paragraph"
    ParaInfo * item = new ParaInfo();
    item->start = start;
    item->size= size;
    item->fgcolor = fgcolor;
    item->bgcolor = bgcolor;
    item->isEditable = editable;

    if(isInsert)
    {
        // now we have to fix the 'start' value for all the para
        // info blocks that coorspond to textblocks that we inserted
        // inside of. This means parent tblock paragraph locations
        // that are greater than zero in that type of textblock

        int increase = size + 1;
        QMap<TextBlock*,TextBlockInfo*>::Iterator it;
        for ( it = m_tbInfoMap->begin(); it != m_tbInfoMap->end(); ++it )
        {
            TextBlock * tblock = it.key();
            TextBlockInfo * thisTbInfo = it.data();
            int firstLoc = m_textBlockList.findRef(tblock);
            ParaInfo * lastPi = thisTbInfo->m_paraList.last();

            for(ParaInfo * pi = thisTbInfo->m_paraList.first(); pi; pi = thisTbInfo->m_paraList.next())
            {
                int minPara = pi->start+firstLoc;

                // only worth doing if in range of the whole
                // representation
                if(!pi->start && (startLine > (lastPi->start+firstLoc+lastPi->size) || endLine < minPara) )
                    break;

                // now, only for those paraInfo blocks which
                // have exceeded our last line, we increase them
                if(pi->start && minPara >= endLine )
                    pi->start += increase;

            }
        }

    }

    tbinfo->m_paraList.append(item);

}

void CodeEditor::appendText(TextBlockList * items)
{

    for (TextBlock *tb = items->first(); tb; tb = items->next())
    {
        // types of things we may cast our text block into
        // This isnt efficient, and is a vote for recording
        // code block types in an enumerated list somewhere,
        // as well as a generic attribute "blockType" we could
        // quickly access, rather than casting. -b.t.
        HierarchicalCodeBlock * hb = dynamic_cast<HierarchicalCodeBlock *>(tb);
        CodeMethodBlock * mb = 0;
        CodeClassFieldDeclarationBlock * db = 0;
        CodeBlockWithComments * cb = 0;
        // CodeComment * cm = 0;
        if(hb)
            appendText(hb);
        else if ( (mb = dynamic_cast<CodeMethodBlock*>(tb)) )
            appendText(mb);
        else if ( (db = dynamic_cast<CodeClassFieldDeclarationBlock*>(tb)) )
            appendText(db);
        else if ( (cb = dynamic_cast<CodeBlockWithComments*>(tb)) )
            appendText(cb);
        /*
                        // no! shouldn't be any 'naked' comments floating about. Always
                        // are assocated with a parent code block
                        else if ( (cm = dynamic_cast<CodeComment*>(tb)) )
                                appendText(cm);
        */
        else
            appendText(tb); // no cast worked. Just do a text block
    }

}

void CodeEditor::appendText (CodeComment * comment, TextBlock * parent, UMLObject * umlObj , const QString & componentName)
{

    if(!comment->getWriteOutText() && !m_showHiddenBlocks)
        return;

    QColor bgcolor = getState().nonEditBlockColor;
    if(!comment->getWriteOutText() && m_showHiddenBlocks)
        bgcolor = getState().hiddenColor;

    QString indent = comment->getIndentationString();
    QString text = comment->toString(); // use comment formatting, NOT formatMultiLineText(comment->toString(), indent, "\n");
    if(!StringIsBlank(text))
        insert(text,parent,true,getState().fontColor, bgcolor, umlObj, componentName);

}

void CodeEditor::appendText (CodeBlockWithComments * cb ) {

    if(!cb->getWriteOutText() && !m_showHiddenBlocks)
        return;

    QString indent = cb->getIndentationString();
    QString body = cb->formatMultiLineText (cb->getText(), indent, "\n");

    QColor bgcolor = getState().editBlockColor;
    QString componentName = QString("CodeBlock");

    appendText(cb->getComment(), cb, 0, componentName);

    if(!cb->getWriteOutText() && m_showHiddenBlocks)
        bgcolor = getState().hiddenColor;

    if(!StringIsBlank(body))
        insert(body,cb,true,getState().fontColor,bgcolor,0);

}

void CodeEditor::appendText (CodeClassFieldDeclarationBlock * db ) {

    if(!db->getWriteOutText() && !m_showHiddenBlocks)
        return;

    QString indent = db->getIndentationString();
    QString body = db->formatMultiLineText (db->getText(), indent, "\n");

    UMLObject * parentObj = db->getParentClassField()->getParentObject();

    QColor bgcolor = getState().editBlockColor;
    QString componentName = QString("");
    if(parentObj)
    {
        if(db->getParentClassField()->parentIsAttribute()) {
            componentName = parentDocName + "::attribute_field(" + parentObj->getName() + ')';
        } else {
            UMLRole * role = dynamic_cast<UMLRole*>(parentObj);
            componentName = parentDocName + "::association_field(" + role->getName() + ')';
        }
        bgcolor = getState().umlObjectColor;
    }

    appendText(db->getComment(), db, parentObj,componentName);

    if(!db->getWriteOutText() && m_showHiddenBlocks)
        bgcolor = getState().hiddenColor;

    if(!StringIsBlank(body))
        insert(body,db,false,getState().fontColor,bgcolor,parentObj);


}

void CodeEditor::appendText (CodeMethodBlock * mb) {

    // Note: IF CodeAccessors are hidden, we DON'T show
    // it even when requested as the hiddeness of these methods
    // should be controled by the class fields, not the user in the editor.
    if(!mb->getWriteOutText() && (!m_showHiddenBlocks || dynamic_cast<CodeAccessorMethod*>(mb)))
        return;

    QColor bgcolor = getState().umlObjectColor;
    QString indent = mb->getIndentationString();
    QString bodyIndent = mb->getIndentationString(mb->getIndentationLevel()+1);

    QString startText = mb->formatMultiLineText ( mb->getStartMethodText(), indent, "\n");
    QString body = mb->formatMultiLineText (mb->getText(), bodyIndent, "\n");
    QString endText = mb->formatMultiLineText( mb->getEndMethodText(), indent, "\n");

    if(body.isEmpty())
        body = " \n";

    if(!mb->getWriteOutText() && m_showHiddenBlocks)
    {
        // it gets the 'hidden' color
        bgcolor = getState().hiddenColor;
    }

    QString componentName = QString("<b>parentless method\?</b>");

    // ugly, but we need to know if there is a parent object here.
    CodeOperation * op = dynamic_cast<CodeOperation*>(mb);
    CodeAccessorMethod * accessor = dynamic_cast<CodeAccessorMethod*>(mb);
    UMLObject * parentObj = 0;
    if(op)
    {
        parentObj = op->getParentOperation();
        if(((UMLOperation*)parentObj)->isConstructorOperation())
            componentName = parentDocName + "::operation("+ parentObj->getName()+") constructor method";
        else
            componentName = parentDocName + "::operation("+ parentObj->getName()+") method";
    }
    if(accessor)
    {
        parentObj = accessor->getParentObject();
        if(accessor->getParentClassField()->parentIsAttribute()) {
            componentName = parentDocName + "::attribute_field(" + parentObj->getName() + ") accessor method";
        } else {
            UMLRole * role = dynamic_cast<UMLRole*>(parentObj);
            componentName = parentDocName + "::association_field(" + role->getName() + ") accessor method";
        }

    }

    //appendText(mb->getComment(), mb, parentObj, componentName);
    appendText(mb->getComment(), mb->getComment(), parentObj, componentName);

    if(!StringIsBlank(startText))
        insert(startText,mb,false,getState().fontColor,bgcolor,parentObj);
    // always insert body for methods..IF we don't, we create a
    // situation where the user cant edit the body (!)
    insert(body,mb,true,getState().fontColor,bgcolor,parentObj);
    if(!StringIsBlank(endText))
        insert(endText,mb,false,getState().fontColor,bgcolor,parentObj);

}

void CodeEditor::appendText (TextBlock * tb) {

    if(!tb->getWriteOutText() && !m_showHiddenBlocks)
        return;

    QColor bgcolor = getState().nonEditBlockColor;
    if(!tb->getWriteOutText() && m_showHiddenBlocks)
        bgcolor = getState().hiddenColor;

    QString str = tb->toString();
    insert(str,tb,false,getState().fontColor,bgcolor);

}

void CodeEditor::appendText(HierarchicalCodeBlock * hblock)
{

    if(!hblock->getWriteOutText() && !m_showHiddenBlocks)
        return;

    OwnedHierarchicalCodeBlock * test = dynamic_cast<OwnedHierarchicalCodeBlock *>(hblock);
    UMLObject * parentObj = 0;
    QString componentName = QString("");
    QColor paperColor = getState().nonEditBlockColor;
    if(test)
    {
        parentObj = test->getParentObject();
        UMLClassifier *c = dynamic_cast<UMLClassifier*>(parentObj);
        if (c) {
            QString typeStr;
            if (c->isInterface())
                typeStr = "Interface";
            else
                typeStr = "Class";
            componentName = parentDocName + "::" + typeStr + '(' + parentObj->getName() + ')';
        } else {
            componentName = parentDocName + "::UNKNOWN(" + parentObj->getName() + ')';
        }

        paperColor = getState().umlObjectColor;
    }

    if(!hblock->getWriteOutText() && m_showHiddenBlocks)
        paperColor = getState().hiddenColor;

    TextBlockList * items = hblock->getTextBlockList();
    QString indent = hblock->getIndentationString();
    QString startText = hblock->formatMultiLineText ( hblock->getStartText(), indent, "\n");
    QString endText = hblock->formatMultiLineText( hblock->getEndText(), indent, "\n");

    appendText(hblock->getComment(), hblock, parentObj, componentName);

    if(!StringIsBlank(startText))
        insert(startText,hblock,false,getState().fontColor,paperColor, parentObj);
    appendText(items);
    if(!StringIsBlank(endText))
        insert(endText,hblock,false,getState().fontColor,paperColor);

}

void CodeEditor::insertParagraph ( const QString & text, int para )
{
    QTextEdit::insertParagraph(text,para);
}

void CodeEditor::removeParagraph ( int para )
{
    QTextEdit::removeParagraph(para);
}

// All umlobjects which may have pop-up boxes should return true here
// Yes, a CRAPPY way of doing this. Im not proud. =b.t.
bool CodeEditor::textBlockIsClickable(UMLObject * obj)
{

    if( dynamic_cast<UMLAttribute*>(obj) )
        return true;
    else if( dynamic_cast<UMLClassifier*>(obj) )
        return true;
    else if( dynamic_cast<UMLRole*>(obj) )
        return true;
    else if( dynamic_cast<UMLOperation*>(obj) )
        return true;

    return false;
}

void CodeEditor::slotChangeSelectedBlockView()
{
    TextBlock * tb = m_selectedTextBlock;
    if(tb) {
        tb->setWriteOutText(tb->getWriteOutText() ? false : true );
        rebuildView(m_lastPara);
    }

}

// change the status of the comment writeOutText value to
// opposite of current value
void CodeEditor::slotChangeSelectedBlockCommentView()
{

    TextBlock * tb = m_selectedTextBlock;
    CodeBlockWithComments * cb = 0;
    if(tb && (cb = dynamic_cast<CodeBlockWithComments*>(tb)))
    {
        cb->getComment()->setWriteOutText(cb->getComment()->getWriteOutText() ? false : true );
        rebuildView( m_lastPara );
    }

}

void CodeEditor::slotInsertCodeBlockBeforeSelected()
{

    TextBlock * tb = m_selectedTextBlock;
    CodeBlockWithComments * newBlock = m_parentDoc->newCodeBlockWithComments();
    newBlock->setText("<<INSERT>>");
    newBlock->getComment()->setWriteOutText(false);

    m_parentDoc->insertTextBlock(newBlock, tb, false);

    int location = m_textBlockList.findRef(m_selectedTextBlock); // find first para of selected block

    QString body = newBlock->formatMultiLineText (newBlock->getText(), newBlock->getIndentationString(), "\n");

    insert(body,newBlock,true,getState().fontColor,
           getState().editBlockColor,0,QString("CodeBlock"),location);

}

void CodeEditor::slotInsertCodeBlockAfterSelected()
{

    TextBlock * tb = m_selectedTextBlock;
    CodeBlockWithComments * newBlock = m_parentDoc->newCodeBlockWithComments();
    newBlock->setText("<<INSERT>>");
    newBlock->getComment()->setWriteOutText(false);

    m_parentDoc->insertTextBlock(newBlock, tb, true);

    // find last para of selected block
    TextBlockInfo *tbinfo = (*m_tbInfoMap)[m_selectedTextBlock];
    ParaInfo * lastpi = tbinfo->m_paraList.last();
    int location = m_textBlockList.findRef(m_selectedTextBlock) + lastpi->start + lastpi->size + 1;

    QString body = newBlock->formatMultiLineText (newBlock->getText(), newBlock->getIndentationString(), "\n");

    insert(body,newBlock,true,getState().fontColor,
           getState().editBlockColor,0,QString("CodeBlock"),location);

}

QPopupMenu * CodeEditor::createPopupMenu ( const QPoint & pos )
{

    TextBlock * tb = m_selectedTextBlock;
    m_lastPara = paragraphAt(pos);

    QPopupMenu * menu = new QPopupMenu(this);
    // ugh. A bug in the Qt interaction between QTextEdit and Menu
    // can sometimes trigger a clear() call of the text area after
    // the popup menu is destroyed. The workaround is to disable
    // the behavior by blocking the destroy signal from the menu.
    menu->blockSignals(true);

    if (m_selectedTextBlock)
    {
        if(tb->getWriteOutText())
            menu->insertItem("Hide",this,SLOT(slotChangeSelectedBlockView()), Key_H, 0);
        else
            menu->insertItem("Show",this,SLOT(slotChangeSelectedBlockView()), Key_S, 0);

        CodeBlockWithComments * cb = dynamic_cast<CodeBlockWithComments*>(tb);
        if(cb)
            if(cb->getComment()->getWriteOutText())
                menu->insertItem("Hide Comment",this,SLOT(slotChangeSelectedBlockCommentView()), CTRL+Key_H, 1);
            else
                menu->insertItem("Show Comment",this,SLOT(slotChangeSelectedBlockCommentView()), CTRL+Key_S, 1);
        menu->insertSeparator();

        menu->insertItem("Insert Code Block Before",this,SLOT(slotInsertCodeBlockBeforeSelected()), CTRL+Key_B, 2);
        menu->insertItem("Insert Code Block After",this,SLOT(slotInsertCodeBlockAfterSelected()), CTRL+Key_A, 3);

        menu->insertSeparator();

        menu->insertItem("Copy",this,SLOT(slotCopyTextBlock()), CTRL+Key_C, 4);
        menu->insertItem("Paste",this,SLOT(slotPasteTextBlock()), CTRL+Key_V, 5);
        menu->insertItem("Cut",this,SLOT(slotCutTextBlock()), CTRL+Key_X, 6);

        // enable/disable based on conditions
        if(m_selectedTextBlock == m_parentDoc->getHeader())
            menu->setItemEnabled (2, false);

        if(!m_textBlockToPaste)
            menu->setItemEnabled (5, false);

        if(!tb->canDelete())
            menu->setItemEnabled (6, false);

        // manythings cant be copied. RIght now, lets just limit ourselves to
        // owned things and hierarchicalcodeblocks
        if(dynamic_cast<OwnedCodeBlock*>(m_selectedTextBlock) ||
                dynamic_cast<HierarchicalCodeBlock*>(m_selectedTextBlock))
            menu->setItemEnabled (4, false);

        // TBD
        // m_selectedTextBlock->insertCodeEditMenuItems(menu, this);
    }

    return menu;
}

void CodeEditor::slotCopyTextBlock ( )
{
    // make a copy
    if(dynamic_cast<HierarchicalCodeBlock*>(m_selectedTextBlock))
        m_textBlockToPaste = m_parentDoc->newHierarchicalCodeBlock();
    else if(dynamic_cast<CodeBlockWithComments*>(m_selectedTextBlock))
        m_textBlockToPaste = m_parentDoc->newCodeBlockWithComments();
    else if(dynamic_cast<CodeBlock*>(m_selectedTextBlock))
        m_textBlockToPaste = m_parentDoc->newCodeBlock();
    else if(dynamic_cast<CodeComment*>(m_selectedTextBlock))
        m_textBlockToPaste = CodeGenFactory::newCodeComment(m_parentDoc);
    else
    {
        kError()<<" ERROR: CodeEditor can't copy selected block:"<<m_selectedTextBlock<<" of unknown type"<<endl;
        m_textBlockToPaste = 0;
        return; // error!
    }

    m_textBlockToPaste->setAttributesFromObject(m_selectedTextBlock);

}

void CodeEditor::slotCutTextBlock ( ) {

    // make a copy first
    slotCopyTextBlock();

    // This could cause problems, but we are OK as
    // long as we only try to delete 'canDelete' textblocks
    if(m_selectedTextBlock->canDelete())
    {
        // just in case there are pending edits
        // we don't want to lose them
        if (m_lastTextBlockToBeEdited && m_lastTextBlockToBeEdited == (CodeBlock*) m_selectedTextBlock)
        {
            updateTextBlockFromText (m_lastTextBlockToBeEdited);
            m_lastTextBlockToBeEdited = 0;
        }

        m_parentDoc->removeTextBlock(m_selectedTextBlock);
        rebuildView(m_lastPara);
        // removeTextBlock(m_selectedTextBlock);
        m_selectedTextBlock = 0;
    }

}

void CodeEditor::slotPasteTextBlock ( ) {

    if(m_textBlockToPaste)
    {
        m_parentDoc->insertTextBlock(m_textBlockToPaste, m_selectedTextBlock);
        m_textBlockToPaste = 0;
        rebuildView(m_lastPara);
    }

}

void CodeEditor::slotRedrawText() {
    rebuildView(m_lastPara);
}

void CodeEditor::init ( CodeViewerDialog * parentDlg, CodeDocument * parentDoc ) {

    // safety to insure that we are up to date
    parentDoc->synchronize();

    m_parentDlg = parentDlg;
    m_parentDoc = parentDoc;

    setUndoRedoEnabled( false );
    setCursor( QCursor( 0 ) );
    setMouseTracking( true );
    setReadOnly (true);
    m_isHighlighted = getState().blocksAreHighlighted;
    m_showHiddenBlocks = getState().showHiddenBlocks;

    m_newLinePressed = false;
    m_backspacePressed = false;
    m_textBlockToPaste = 0;
    m_selectedTextBlock = 0;
    m_lastTextBlockToBeEdited = 0;
    m_tbInfoMap = new QMap<TextBlock *, TextBlockInfo*>;

    setFont( getState().font );

    // set name of parent doc
    ClassifierCodeDocument * cdoc = dynamic_cast<ClassifierCodeDocument*>(m_parentDoc);
    if(cdoc)
        parentDocName = cdoc->getParentClassifier()->getName();
    else
        parentDocName = "";

    // set some viewability parameters
    //int margin = fontMetrics().height();

    QBrush pbrush = QBrush ( getState().paperColor);
    setPaper(pbrush);

    // setMargin(margin);

    //       connect(this,SIGNAL(newLinePressed()),this,SLOT(newLinePressed()));
    //       connect(this,SIGNAL(backspacePressed()),this,SLOT(backspacePressed()));
    connect(this,SIGNAL(doubleClicked(int,int)),this,SLOT(doubleClicked(int,int)));
    connect(this,SIGNAL(cursorPositionChanged(int,int)),this,SLOT(cursorPositionChanged(int,int)));

    // do this last
    loadFromDocument();

}

void CodeEditor::updateTextBlockFromText (TextBlock * block) {

    if (block) {

        CodeMethodBlock * cmb = dynamic_cast<CodeMethodBlock*>(block);
        //QString baseIndent = block->getNewEditorLine(block->getIndentationLevel()+(cmb ? 1 : 0));
        QString baseIndent = block->getIndentationString(block->getIndentationLevel()+(cmb ? 1 : 0));

        TextBlockInfo *info = (*m_tbInfoMap)[block];
        UMLObject * parentObj = info->getParent();
        int pstart = m_textBlockList.findRef(block);
        QString content = "";

        // Assemble content from editiable paras
        QPtrList<ParaInfo> list = info->m_paraList;
        for(ParaInfo * item = list.first(); item; item=list.next())
        {
            if(item->isEditable)
            {
                int lastpara = item->start+pstart+item->size;
                int endEdit = block->lastEditableLine();
                int lastLineToAddNewLine = lastpara + endEdit;
                for(int para=(item->start+pstart);para<=lastpara;para++)
                {
                    QString line = block->unformatText(text(para), baseIndent);
                    content += line;
                    // \n are implicit in the editor (!) so we should put them
                    // back in, if there is any content from the line
                    if(!line.isEmpty() && para != lastLineToAddNewLine)
                        content += "\n";
                }
            }
        }

        //cerr<<"UPDATE GOT CONTENT:["<<content.latin1()<<"] to block:"<<block<<endl;
        block->setText(content);

        // if a parent for the block, try to set its documentation
        // as long as its NOT an accessor codeblock.
        if(parentObj && !info->isCodeAccessorMethod)
            parentObj->setDoc(content);

        // make note that its now user generated
        if(cmb)
            cmb->setContentType(CodeBlock::UserGenerated);

    }
}

void CodeEditor::cursorPositionChanged(int para, int pos)
{

    // safety.. this is endemic of a 'bad' pointer event and can crash us otherwise
    if(pos < 0)
        return;

    //      bool lastParaIsEditable = isReadOnly() ? false : true;
    bool lastParaIsEditable = isParaEditable(m_lastPara);

    // IF last para where cursor is coming from was editable
    // we have a variety of things to look out for.
    if(lastParaIsEditable)
    {
        // If we got here as the result of a newline, then expansion
        // of a para editablity occurs.
        if((para-1) == m_lastPara && m_newLinePressed )
            expandSelectedParagraph ( m_lastPara );

        // conversely, we contract the zone of editablity IF we
        // got to current position as result of backspace
        if((para+1) == m_lastPara && m_backspacePressed )
            contractSelectedParagraph( para );

    }

    // now check if the current paragraph is really editiable, and if so,
    // so some things
    bool editPara = isParaEditable(para);
    if(editPara) {

        TextBlock * tBlock = m_textBlockList.at(para);
        CodeMethodBlock * cmb = dynamic_cast<CodeMethodBlock*>(tBlock);

        // auto-indent new lines
        QString currentParaText = text(para);
        QString baseIndent = tBlock->getNewEditorLine(tBlock->getIndentationLevel()+(cmb ? 1 : 0));
        // cerr<<"AUTO INDENT:["<<baseIndent.latin1()<<"] isMethod?"<<(cmb?"true":"false")<<endl;
        int minPos = baseIndent.length();

        // add indent chars to the current line, if missing
        if(!m_backspacePressed && !currentParaText.contains(QRegExp('^'+baseIndent)))
        {
            insertAt(baseIndent,para,0);
            setCursorPosition(para,pos+minPos);
            return;
        }

        if(pos<minPos)
        {

            bool priorParaIsEditable = isParaEditable(para-1);
            if(m_backspacePressed && para && priorParaIsEditable)
            {
                int endOfPriorLine = paragraphLength(para-1);
                // IN this case, we remove old (para) line, and tack its
                // contents on the line we are going to.
                QString contents = text(para);
                contents = contents.right(contents.length()-m_lastPos+1);

                // this next thing happens when we arent deleting last line
                // of editable text, so we want to append whats left of this line
                // onto the one we are backspacing into
                if(paraIsNotSingleLine(para))
                {
                    removeParagraph(para);
                    insertAt(contents,(para-1),endOfPriorLine);
                    setCursorPosition((para-1),endOfPriorLine);
                }

            } else {
                // well, if the following is true, then they
                // are trying to hack away at the last line, which
                // we cant allow to entirely disappear. Lets preserve
                // the indentation
                if(m_backspacePressed && !priorParaIsEditable)
                {
                    QString contents = text(para);
                    contents = contents.right(contents.length()-m_lastPos+1);
                    contents = baseIndent + contents.left(contents.length()-1); // left is to remove trailing space
                    insertParagraph(contents,para+1);
                    removeParagraph(para);

                    // furthermore, IF its nothing but indentation + whitespace
                    // we switch this back to Auto-Generated.
                    if(cmb && contents.contains(QRegExp('^'+baseIndent+"\\s$")))
                    {
                        cmb->setContentType(CodeBlock::AutoGenerated);
                        cmb->syncToParent();
                    }

                }

                // send them to the first spot in the line which is editable
                setCursorPosition(para,minPos);

            }
            return;
        }
    }

    // look for changes in editability, if they occur, we need to record
    // the edits which have been made
    if((editPara && !m_lastTextBlockToBeEdited) || (!editPara && m_lastTextBlockToBeEdited)) {

        setReadOnly(editPara ? false : true);

        // IF this is a different text block, update the body of the method
        // it belongs to
        if(m_lastTextBlockToBeEdited && (m_lastTextBlockToBeEdited != m_textBlockList.at(para) || !editPara))
        {
            updateTextBlockFromText (m_lastTextBlockToBeEdited);
            m_lastTextBlockToBeEdited = 0;
        }

        if(editPara)
            m_lastTextBlockToBeEdited = m_textBlockList.at(para);
        else
            m_lastTextBlockToBeEdited = 0;

    }

    m_lastPara = para;
    m_lastPos = pos;
    m_newLinePressed = false;
    m_backspacePressed = false;

}

bool CodeEditor::paraIsNotSingleLine (int para)
{
    TextBlock * tBlock = m_textBlockList.at(para);
    if(tBlock)
    {
        int pstart = m_textBlockList.findRef(tBlock);
        TextBlockInfo *info = (*m_tbInfoMap)[tBlock];
        QPtrList<ParaInfo> list = info->m_paraList;

        for(ParaInfo * item = list.first(); item; item=list.next())
            if((pstart+item->start) <= para && (item->start+pstart+item->size) >= para )
                if(item->size > 0)
                    return true;
    }
    return false;
}

bool CodeEditor::isParaEditable (int para) {

    if (para <0)
        return false;

    TextBlock * tBlock = m_textBlockList.at(para);
    if(tBlock)
    {
        int editStart = tBlock->firstEditableLine();
        int editEnd = tBlock->lastEditableLine();
        bool hasEditableRange = (editStart > 0 || editEnd < 0) ? true : false;
        TextBlockInfo *info = (*m_tbInfoMap)[tBlock];
        int pstart = m_textBlockList.findRef(tBlock);
        int relativeLine = para - pstart;
        QPtrList<ParaInfo> list = info->m_paraList;
        for(ParaInfo * item = list.first(); item; item=list.next())
        {
            if((item->start+pstart) <= para && (item->start+pstart+item->size) >= para)
                if(item->isEditable && hasEditableRange)
                {
                    if ( relativeLine >= editStart && relativeLine <= (item->size + editEnd) )
                        return true;
                    else
                        return false;
                } else
                    return item->isEditable;
        }
    }
    return false;
}

void CodeEditor::changeTextBlockHighlighting(TextBlock * tBlock, bool selected) {

    if(tBlock)
    {
        TextBlockInfo *info = (*m_tbInfoMap)[tBlock];
        QPtrList<ParaInfo> list = info->m_paraList;
        int pstart = m_textBlockList.findRef(tBlock);
        for(ParaInfo * item = list.first(); item; item=list.next())
            for(int p=(item->start+pstart);p<=(item->start+pstart+item->size);p++)
                if(selected)
                    if(info->isClickable)
                        setParagraphBackgroundColor(p,getState().selectedColor);
                    else
                        setParagraphBackgroundColor(p,getState().nonEditBlockColor);
                else if(m_isHighlighted)
                    setParagraphBackgroundColor(p,item->bgcolor);
                else
                    setParagraphBackgroundColor(p,getState().paperColor);
    }

}

void CodeEditor::changeShowHidden (int signal) {

    if(signal)
        m_showHiddenBlocks = true;
    else
        m_showHiddenBlocks = false;

    rebuildView(m_lastPara);

}

// colorizes/uncolorizes type for ALL paragraphs
void CodeEditor::changeHighlighting(int signal) {

    int total_para = paragraphs()-1;
    if(signal) {
        // we want to highlight
        m_isHighlighted = true;
        for(int para=0;para<total_para;para++)
        {
            TextBlock * tblock = m_textBlockList.at(para);
            changeTextBlockHighlighting(tblock,false);
        }


    } else {
        // we DON'T want to highlight
        m_isHighlighted = false;
        for(int para=0;para<total_para;para++)
            setParagraphBackgroundColor(para,getState().paperColor);
    }

    // now redo the "selected" para, should it exist
    if(m_selectedTextBlock)
        changeTextBlockHighlighting(m_selectedTextBlock,true);

}

void CodeEditor::contractSelectedParagraph( int paraToRemove ) {
    TextBlock * tBlock = m_textBlockList.at(paraToRemove);
    if(tBlock)
    {
        int pstart = m_textBlockList.findRef(tBlock);
        TextBlockInfo *info = (*m_tbInfoMap)[tBlock];
        QPtrList<ParaInfo> list = info->m_paraList;

        bool lowerStartPosition = false;
        for(ParaInfo * item = list.first(); item; item=list.next())
        {
            if(lowerStartPosition)
                item->start -= 1;

            if((pstart+item->start) <= paraToRemove && (item->start+pstart+item->size) >= paraToRemove)
            {
                item->size -= 1;
                // a little cheat.. we don't want to remove last line as we need
                // to leave a place that can be 'edited' by the tool IF the user
                // changes their mind about method body content
                if(item->size < 0)
                    item->size = 0;
                lowerStartPosition = true;
            }
        }

        m_textBlockList.remove(paraToRemove);
    }
}

void CodeEditor::expandSelectedParagraph( int priorPara ) {


    TextBlock * tBlock = m_textBlockList.at(priorPara);
    if(tBlock)
    {
        // add this tBlock in
        m_textBlockList.insert(priorPara,tBlock);
        TextBlockInfo *info = (*m_tbInfoMap)[tBlock];
        QPtrList<ParaInfo> list = info->m_paraList;
        int pstart = m_textBlockList.findRef(tBlock);

        // now update the paragraph information
        bool upStartPosition = false;
        for(ParaInfo * item = list.first(); item; item=list.next())
        {
            // AFTER we get a match, then following para's need to have start position upped too
            if(upStartPosition)
                item->start += 1;

            if((pstart+item->start) <= priorPara && (item->start+pstart+item->size) >= priorPara)
            {
                item->size += 1;
                cursorPositionChanged(m_lastPara, m_lastPos);
                upStartPosition = true;
            }
        }
    }

}

void CodeEditor::contentsMouseMoveEvent ( QMouseEvent * e )
{

    int para = paragraphAt(e->pos());

    if (para < 0)
        return; // shouldn't happen..

    TextBlock * tblock = m_textBlockList.at(para);
    if (tblock && m_selectedTextBlock != tblock ) {
        TextBlockInfo * info = (*m_tbInfoMap)[tblock];

        // unhighlight old selected textblock regardless of whether
        // it was selected or not.
        changeTextBlockHighlighting(m_selectedTextBlock,false);

        // highlight new block
        changeTextBlockHighlighting(tblock,true);

        // FIX: update the label that shows what type of component this is
        getComponentLabel()->setText("<b>"+info->displayName+"</b>");

        m_selectedTextBlock = tblock;

        if(m_lastTextBlockToBeEdited)
        {
            updateTextBlockFromText (m_lastTextBlockToBeEdited);
            m_lastTextBlockToBeEdited = 0;
        }
    }

    // record this as the last paragraph

}


// Rebuild our view of the document. Happens whenever we change
// some field/aspect of an underlying UML object used to create
// the view.
// If connections are right, then the UMLObject will send out the modified()
// signal which will trigger a call to re-generate the appropriate code within
// the code document. Our burden is to appropriately prepare the tool: we clear
// out ALL the textblocks in the QTextEdit widget and then re-show them
// after the dialog disappears
void CodeEditor::rebuildView( int startCursorPos ) {

    loadFromDocument();

    // make a minima attempt to leave the cursor (view of the code) where
    // we started
    int new_nrof_para = paragraphs() -1;
    setCursorPosition((startCursorPos < new_nrof_para ? startCursorPos : 0), 0);

}




#include "codeeditor.moc"
