
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
 ***************************************************************************/
#include <kdebug.h>
#include <klocale.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qbrush.h>
#include <qlayout.h>
#include <qregexp.h>

#include "codeviewerdialog.h"
#include "../attribute.h"
#include "../classifier.h"
#include "../class.h"
#include "../interface.h"
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

#include "codeeditor.h"
#include "classpropdlg.h"
#include "umlattributedialog.h"
#include "umlroledialog.h"
#include "umloperationdialog.h"

CodeViewerDialog::CodeViewerDialog ( QWidget* parent, CodeDocument * doc, SettingsDlg::CodeViewerState state, 
                                     const char* name, bool modal, WFlags fl )
    : CodeViewerDialogBase ( parent, name, modal, fl )

{
	if ( !name )
		setName( "CodeViewerDialog" );

	m_state = state;
	m_isHighlighted = false;
	m_newLinePressed = false;
	m_backspacePressed = false;
	m_parentDocument = doc;
	m_selectedTextBlock = 0;
	m_lastTextBlockToBeEdited = 0;
        m_paraInfoMap = new QMap<int, TextBlock *>;
        m_tbInfoMap = new QMap<TextBlock *, TextBlockInfo*>;

	setFont( m_state.font );

	// set some viewability parameters
	int margin = fontMetrics().height();
	int width = fontMetrics().maxWidth() * m_state.width;
	int height = fontMetrics().lineSpacing() * m_state.height;

	QBrush pbrush = QBrush ( m_state.paperColor);
	codeTextEdit->setPaper(pbrush);

	CodeViewerDialogBaseLayout->setMargin(margin);

	resize( QSize(width, height).expandedTo(minimumSizeHint()) );

	// set name of parent doc
	ClassifierCodeDocument * cdoc = dynamic_cast<ClassifierCodeDocument*>(m_parentDocument);
	if(cdoc)
		parentDocName = cdoc->getParentClassifier()->getName();
	else
		parentDocName = "";

	loadFromDocument(doc);

	connect(codeTextEdit,SIGNAL(mouseMoved(QMouseEvent*)),this,SLOT(mouseMoved(QMouseEvent*)));
	connect(codeTextEdit,SIGNAL(clicked(int,int)),this,SLOT(clicked(int,int)));
	connect(codeTextEdit,SIGNAL(doubleClicked(int,int)),this,SLOT(doubleClicked(int,int)));
	connect(codeTextEdit,SIGNAL(newLinePressed()),this,SLOT(newLinePressed()));
	connect(codeTextEdit,SIGNAL(backspacePressed()),this,SLOT(backspacePressed()));
	connect(codeTextEdit,SIGNAL(cursorPositionChanged(int,int)),this,SLOT(cursorPositionChanged(int,int)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
CodeViewerDialog::~CodeViewerDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

SettingsDlg::CodeViewerState CodeViewerDialog::getState() {
	return m_state;
}

void CodeViewerDialog::loadFromDocument(CodeDocument * doc)
{

	// set caption on tool
        QString caption = doc->getFileName() + doc->getFileExtension();
        setCaption( tr2i18n( caption.latin1() ) );

	// header for document
	QString header = doc->getHeader()->toString();
	QString componentName = QString("header for file ") +caption;
	if(!header.isEmpty())
		insert(header,doc->getHeader(),false,QColor("black"),QColor("white"),0,componentName);

	// now all the text blocks in the document
	QPtrList<TextBlock> * items = doc->getTextBlockList();
	insertText(items);

	codeTextEdit->setCursorPosition(0,0);
}

// All umlobjects which may have pop-up boxes should return true here
// Yes, a CRAPPY way of doing this. Im not proud. =b.t.
bool CodeViewerDialog::textBlockIsClickable(UMLObject * obj)
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

void CodeViewerDialog::insertText(QPtrList<TextBlock> * items)
{
        for (TextBlock *tb = items->first(); tb; tb = items->next())
        {
		HierarchicalCodeBlock * hb = dynamic_cast<HierarchicalCodeBlock *>(tb); 
		if(hb)
			insertText(hb);
		else { 
			CodeMethodBlock * mb = dynamic_cast<CodeMethodBlock*>(tb); 
			if(mb)
				insertText(mb);
			else { 
				CodeClassFieldDeclarationBlock * db = dynamic_cast<CodeClassFieldDeclarationBlock*>(tb); 
				if(db)
					insertText(db);
				else
					insertText(tb);
			}
		}
        }
}

void CodeViewerDialog::insertText (CodeComment * comment, TextBlock * parent, UMLObject * umlObj , const QString & componentName) 
{

        if(!comment->getWriteOutText())
		return;

        QString indent = comment->getIndentation();
	QString text = parent->formatMultiLineText(comment->toString(), indent, "\n");
	if(!text.isEmpty())
		insert(text,parent,false,QColor("black"),QColor("pink"), umlObj, componentName);
}

void CodeViewerDialog::insertText (CodeClassFieldDeclarationBlock * db ) {

        if(!db->getWriteOutText())
		return;

        QString indent = db->getIndentation();
        QString body = db->formatMultiLineText (db->getText(), indent, "\n");

        UMLObject * parentObj = db->getParentClassField()->getParentObject();

	QString componentName = QString("");
	if(parentObj)
	{
		if(db->getParentClassField()->parentIsAttribute()) {
			componentName = parentDocName + "::attribute_field(" + parentObj->getName() + ")";
		} else {
			UMLRole * role = dynamic_cast<UMLRole*>(parentObj);
			componentName = parentDocName + "::association_field(" + role->getName() + ")";
		}
	}

        insertText(db->getComment(), db, parentObj,componentName);

        if(!body.isEmpty())
                insert(body,db,false,m_state.fontColor,m_state.paperColor,parentObj);

	
}

void CodeViewerDialog::insertText (CodeMethodBlock * mb) {

        if(!mb->getWriteOutText())
		return;

	QColor bgcolor;
	QString indent = mb->getIndentation();
	QString bodyIndent = mb->getIndentationString(mb->getIndentationLevel()+1);
	QString startText = mb->formatMultiLineText ( mb->getStartMethodText(), indent, "\n");
	QString body = mb->formatMultiLineText (mb->getText(), bodyIndent, "\n");
	QString endText = mb->formatMultiLineText( mb->getEndMethodText(), indent, "\n");

	if(dynamic_cast<CodeAccessorMethod*>(mb)) {
		bgcolor = QColor("pink");
	} else {
		bgcolor = QColor("white");
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

        insertText(mb->getComment(), mb, parentObj, componentName);

        if(!startText.isEmpty())
                insert(startText,mb,false,QColor("black"),bgcolor,parentObj);
        if(!body.isEmpty())
                insert(body,mb,true,QColor("black"),bgcolor,parentObj);
	if(!endText.isEmpty())
		insert(endText,mb,false,QColor("black"),bgcolor,parentObj);

}

void CodeViewerDialog::insertText (TextBlock * tb) {

        if(!tb->getWriteOutText())
		return;

	QString str = tb->toString();
	insert(str,tb,false,QColor("black"),QColor("white"));

}

void CodeViewerDialog::insertText(HierarchicalCodeBlock * hblock)
{

        if(!hblock->getWriteOutText())
		return;

	OwnedHierarchicalCodeBlock * test = dynamic_cast<OwnedHierarchicalCodeBlock *>(hblock);
	UMLObject * parentObj = 0;
	QString componentName = QString("");
	if(test)
	{
		parentObj = test->getParentObject();
		if((dynamic_cast<UMLClass*>(parentObj)))
			componentName = parentDocName + "::class(" + parentObj->getName() + ")"; 
		else if((dynamic_cast<UMLInterface*>(parentObj)))
			componentName = parentDocName + "::interface(" + parentObj->getName() + ")"; 
		else 
			componentName = parentDocName + "::UNKNOWN(" + parentObj->getName() + ")"; 
	}

        QPtrList<TextBlock> * items = hblock->getTextBlockList();
	QString indent = hblock->getIndentation();
	QString startText = hblock->formatMultiLineText ( hblock->getStartText(), indent, "\n");
	QString endText = hblock->formatMultiLineText( hblock->getEndText(), indent, "\n");

        insertText(hblock->getComment(), hblock, parentObj,componentName);

	if(!startText.isEmpty())
		insert(startText,hblock,false,QColor("black"),QColor("white"));
	insertText(items);
	if(!endText.isEmpty())
		insert(endText,hblock,false,QColor("black"),QColor("white"));

}

void CodeViewerDialog::insert (const QString & text, TextBlock * parent, bool editable, const QColor & fgcolor, const QColor & bgcolor, UMLObject * umlobj, const QString & displayName)
{

	if(text.isEmpty())
		return;

	// we will need this later for background coloring
	int startLine = codeTextEdit->paragraphs()-1;

	// actual put in text
	codeTextEdit->setColor(m_state.fontColor);
	codeTextEdit->insert(text);

	// now do 'paragraph' background highlighting
	int endLine = codeTextEdit->paragraphs()-2;
	if(m_isHighlighted)
		for(int para=startLine;para<=endLine;para++)
			codeTextEdit->setParagraphBackgroundColor(para,bgcolor);

	// record paragraph information
	// Did wa already start recording info for this parent object?
	TextBlockInfo * tbinfo; 
	if(m_tbInfoMap->contains(parent))
		tbinfo = (*m_tbInfoMap)[parent];
	else { 
		tbinfo = new TextBlockInfo();
		tbinfo->displayName = displayName;
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
        {
                m_textBlockList.insert(para,parent);
                m_paraInfoMap->insert(para,parent);
        }

	// lastly, update the para info
	// start position is relative to the FIRST parent position
	int start = startLine - m_textBlockList.findRef(parent);

	// create the object that records this particular "paragraph"
	ParaInfo * item = new ParaInfo();
	item->start = start;
	item->size= endLine-startLine;
	item->fgcolor = fgcolor;
	item->bgcolor = bgcolor;
	item->isEditable = editable;

	tbinfo->m_paraList.append(item);

}

void CodeViewerDialog::doubleClicked(int para, int pos)
{

	m_lastPara = para;
	m_lastPos = pos;

	// ugh. more ugliness. We want to be able to call up the
	// correct editing dialog for the given attribute.
    	//TextBlock * tBlock = (*m_paraInfoMap)[para];
	TextBlock * tBlock = m_textBlockList.at(para);
	editTextBlock(tBlock, para);

}

// allow us to edit, as appropriate, the parent UMLObject of the
// given text block. 
void CodeViewerDialog::editTextBlock(TextBlock * tBlock, int para) {

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
				//CodeOperation * cop = 0;
				if( (at = dynamic_cast<UMLAttribute*>(obj)) )
				{
  					UMLAttributeDialog dlg( this, at);
					if( dlg.exec() ) { rebuildView(para); }
				} 
				else if( (dynamic_cast<UMLClassifier*>(obj)) )
				{
					if(m_parentDocument->getParentGenerator()->getDocument()->showProperties(obj)) 
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
					// CodeOperationDialog dlg(cop,this);
					UMLOperationDialog dlg(this,op);
					if( dlg.exec() ) { rebuildView(para); }
				} 
				else 
				{
					kdDebug()<<" CodeViewerDlg ERROR: UNKNOWN parent for textBlock"<<endl;
				}

			}
		}
	}
}

bool CodeViewerDialog::close ( bool alsoDelete )
{

	// remember widget size for next time

	m_state.height = height() / fontMetrics().lineSpacing();
	m_state.width = width() / fontMetrics().maxWidth();

	// capture last code block, if it exists 
	if(m_lastTextBlockToBeEdited)
		updateMethodBlockBody (m_lastTextBlockToBeEdited);

	return CodeViewerDialogBase::close(alsoDelete);

}

void CodeViewerDialog::clear () {

        setCaption( tr2i18n("") );
	m_selectedTextBlock = 0;
        m_paraInfoMap->clear();
        m_textBlockList.clear();
        m_tbInfoMap->clear();
	codeTextEdit->clear();

}

void CodeViewerDialog::updateMethodBlockBody (TextBlock * block) {

	CodeMethodBlock * cb;
	if(block && (cb = dynamic_cast<CodeMethodBlock*>(block))) {

		QString baseIndent = block->getIndentationString(block->getIndentationLevel()+1);

		TextBlockInfo *info = (*m_tbInfoMap)[block];
		int pstart = m_textBlockList.findRef(block);
		QString content = "";

		// Assemble content from editiable paras 
        	QPtrList<ParaInfo> list = info->m_paraList;
        	for(ParaInfo * item = list.first(); item; item=list.next())
        	{
			if(item->isEditable)
			{
				int lastpara = item->start+pstart+item->size;
				for(int para=(item->start+pstart);para<=lastpara;para++)
				{
					QString line = codeTextEdit->text(para);
			//		if(m_autoIndentCheckBox->isChecked())
			//		{
						// we DONT store indentation in method bodies
						line = line.remove(QRegExp("^"+baseIndent));
		//				line = baseIndent + line;
			//		}
					content += line;
					if(para != lastpara)
						content += "\n";
				}
			}
        	}

		cb->setText(content);
		cb->setContentType(CodeBlock::UserGenerated);

	}
}

void CodeViewerDialog::clicked(int para, int pos)
{
	componentLabel->setText("para:"+QString::number(para)+" pos:"+QString::number(pos));
}

void CodeViewerDialog::cursorPositionChanged(int para, int pos)
{

	// safety.. this is endemic of a 'bad' pointer event and can crash us otherwise
	if(pos < 0)
		return;

//	bool lastParaIsEditable = codeTextEdit->isReadOnly() ? false : true;
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

		// auto-indent new lines
		QString currentParaText = codeTextEdit->text(para);
		QString baseIndent = tBlock->getIndentationString(tBlock->getIndentationLevel()+1);
		int minPos = baseIndent.length();

		// add indent chars to the current line, if missing
		if(!m_backspacePressed && !currentParaText.contains(QRegExp("^"+baseIndent)))
		{
			codeTextEdit->insertAt(baseIndent,para,0);
			codeTextEdit->setCursorPosition(para,pos+minPos);
			return;
		}

		if(pos<minPos)
		{

			bool priorParaIsEditable = isParaEditable(para-1);
			if(m_backspacePressed && para && priorParaIsEditable)
			{
				int endOfPriorLine = codeTextEdit->paragraphLength(para-1);
				// IN this case, we remove old (para) line, and tack its
				// contents on the line we are going to.
				QString contents = codeTextEdit->text(para);
				contents = contents.right(contents.length()-m_lastPos+1);

				// this next thing happens when we arent deleting last line
				// of editable text, so we want to append whats left of this line
				// onto the one we are backspacing into
				if(paraIsNotSingleLine(para))
				{
					codeTextEdit->removeParagraph(para);
					codeTextEdit->insertAt(contents,(para-1),endOfPriorLine);
					codeTextEdit->setCursorPosition((para-1),endOfPriorLine);
				}

			} else {
				// well, if the following is true, then they
				// are trying to hack away at the last line, which
				// we cant allow to entirely disappear. Lets preserve
				// the indentation
				if(m_backspacePressed && !priorParaIsEditable)
				{
					QString contents = codeTextEdit->text(para);
					contents = contents.right(contents.length()-m_lastPos+1);
					contents = baseIndent + contents.left(contents.length()-1); // left is to remove trailing space
					codeTextEdit->insertParagraph(contents,para+1);
					codeTextEdit->removeParagraph(para);

					// furthermore, IF its nothing but indentation + whitespace
					// we switch this back to Auto-Generated.
					CodeMethodBlock * cb = dynamic_cast<CodeMethodBlock*>(tBlock);
					if(cb && contents.contains(QRegExp("^"+baseIndent+"\\s$")))
					{
						cb->setContentType(CodeBlock::AutoGenerated);
						cb->syncToParent();
					}
				
				}

				// send them to the first spot in the line which is editable
				codeTextEdit->setCursorPosition(para,minPos);

			}
			return;
		}
	}

	// look for changes in editability, if they occur, we need to record
	// the edits which have been made
	if((editPara && !lastParaIsEditable) || (!editPara && lastParaIsEditable)) {

		codeTextEdit->setReadOnly(editPara ? false : true);

		// IF this is a different text block, update the body of the method
		// it belongs to
		if(m_lastTextBlockToBeEdited && (m_lastTextBlockToBeEdited != m_textBlockList.at(para) || !editPara))
			updateMethodBlockBody (m_lastTextBlockToBeEdited);

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

bool CodeViewerDialog::paraIsNotSingleLine (int para) 
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

bool CodeViewerDialog::isParaEditable (int para) {

    if (para <0)
	return false;

    TextBlock * tBlock = m_textBlockList.at(para);
    if(tBlock)
    {
	TextBlockInfo *info = (*m_tbInfoMap)[tBlock];
	int pstart = m_textBlockList.findRef(tBlock);
	QPtrList<ParaInfo> list = info->m_paraList;
	for(ParaInfo * item = list.first(); item; item=list.next())
	{
		if((item->start+pstart) <= para && (item->start+pstart+item->size) >= para)
			return item->isEditable; 
	}
    }
    return false;
}

void CodeViewerDialog::changeTextBlockHighlighting(TextBlock * tBlock, bool selected) {

    if(tBlock)
    {
	TextBlockInfo *info = (*m_tbInfoMap)[tBlock];
	QPtrList<ParaInfo> list = info->m_paraList;
	int pstart = m_textBlockList.findRef(tBlock);
	for(ParaInfo * item = list.first(); item; item=list.next())
		for(int p=(item->start+pstart);p<=(item->start+pstart+item->size);p++)
			if(selected)
				if(info->isClickable)
					codeTextEdit->setParagraphBackgroundColor(p,m_state.highlightColor); 
				else
					codeTextEdit->setParagraphBackgroundColor(p,QColor(200,200,200)); 
			else if(m_isHighlighted)
				codeTextEdit->setParagraphBackgroundColor(p,item->bgcolor);
			else
				codeTextEdit->setParagraphBackgroundColor(p,m_state.paperColor);
    }

}

// colorizes/uncolorizes type for ALL paragraphs
void CodeViewerDialog::changeHighlighting(int signal) {

	int total_para = codeTextEdit->paragraphs()-1;
	if(signal) { 
		// we want to highlight
		m_isHighlighted = true;
		for(int para=0;para<total_para;para++)
		{
    			//TextBlock * tblock = (*m_paraInfoMap)[para];
			TextBlock * tblock = m_textBlockList.at(para);
	   		changeTextBlockHighlighting(tblock,false);
		}


	} else {
		// we DONT want to highlight
		m_isHighlighted = false;
		for(int para=0;para<total_para;para++)
			codeTextEdit->setParagraphBackgroundColor(para,m_state.paperColor);
	}

	// now redo the "selected" para, should it exist
	if(m_selectedTextBlock)
		changeTextBlockHighlighting(m_selectedTextBlock,true);

}

void CodeViewerDialog::backspacePressed( ) {
	m_backspacePressed = true;
}

void CodeViewerDialog::newLinePressed( ) {
	m_newLinePressed = true;
}

void CodeViewerDialog::contractSelectedParagraph( int paraToRemove ) {
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
				// a little cheat.. we dont want to remove last line as we need 
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

void CodeViewerDialog::expandSelectedParagraph( int priorPara ) {


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

void CodeViewerDialog::mouseMoved ( QMouseEvent * e ) 
{

	int para = codeTextEdit->paragraphAt(e->pos());

	if (para < 0)
		return; // shouldnt happen.. 

    	//TextBlock * tblock = (*m_paraInfoMap)[para];
	TextBlock * tblock = m_textBlockList.at(para);
        if (tblock && m_selectedTextBlock != tblock ) {
		TextBlockInfo * info = (*m_tbInfoMap)[tblock];
	
		// unhighlight old selected textblock regardless of whether
		// it was selected or not. 
                changeTextBlockHighlighting(m_selectedTextBlock,false);

		// highlight new block
                changeTextBlockHighlighting(tblock,true);

		// FIX: update the label that shows what type of component this is
		componentLabel->setText("<b>"+info->displayName+"</b>");

        	m_selectedTextBlock = tblock;

		if(m_lastTextBlockToBeEdited)
		{
			updateMethodBlockBody (m_lastTextBlockToBeEdited);
			m_lastTextBlockToBeEdited = 0;
		}
        }

        // record this as the last paragraph

}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CodeViewerDialog::languageChange()
{
    setCaption( tr2i18n( "Code Viewer" ) );
}

// Rebuild our view of the document. Happens whenever we change
// some field/aspect of an underlying UML object used to create
// the view.
// If connections are right, then the UMLObject will send out the modified()
// signal which will trigger a call to re-generate the appropriate code within
// the code document. Our burden is to appropriately prepare the tool: we clear
// out ALL the textblocks in the QTextEdit widget and then re-show them
// after the dialog disappears
void CodeViewerDialog::rebuildView( int startCursorPos ) { 

	clear();
	loadFromDocument(m_parentDocument);

	// make a minima attempt to leave the cursor (view of the code) where
        // we started
	int new_nrof_para = codeTextEdit->paragraphs() -1;
	codeTextEdit->setCursorPosition((startCursorPos < new_nrof_para ? startCursorPos : 0), 0);

}

#include "codeviewerdialog.moc"
