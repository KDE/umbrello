/***************************************************************************
                          aswriter.cpp  -  description
                             -------------------
    begin                : Sat Feb 08 2003
    copyright            : (C) 2003 by Alexander Blum
    email                : blum@kewbee.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "aswriter.h"
#include "../association.h"
#include "../class.h"
#include "../operation.h"
#include "../umldoc.h"
#include "../attribute.h"

#include <kdebug.h>
#include <qregexp.h>
#include <qstring.h>

ASWriter::ASWriter( UMLDoc *parent, const char *name )
	:SimpleCodeGenerator( parent, name) {
	pListOfReservedKeywords = NULL;
}

ASWriter::~ASWriter() {}


void ASWriter::writeClass(UMLClassifier *c)
{
	if(!c)
	{
		kdDebug()<<"Cannot write class of NULL concept!" << endl;
		return;
	}

	QString classname = cleanName(c->getName());
	QString fileName = c->getName().lower();

	//find an appropriate name for our file
	fileName = findFileName(c,".as");
	if (!fileName)
	{
		emit codeGenerated(c, false);
		return;
	}

	QFile fileas;
	if(!openFile(fileas,fileName+".as"))
	{
		emit codeGenerated(c, false);
		return;
	}
	QTextStream as(&fileas);

	//////////////////////////////
	//Start generating the code!!
	/////////////////////////////


	//try to find a heading file (license, coments, etc)
	QString str;
	str = getHeadingFile(".as");
	if(!str.isEmpty())
	{
		str.replace(QRegExp("%filename%"),fileName+".as");
		str.replace(QRegExp("%filepath%"),fileas.name());
		as << str << endl;
	}


	//write includes
	UMLClassifierList includes;
	findObjectsRelated(c,includes);
	UMLClassifier *conc;
	for(conc = includes.first(); conc ;conc = includes.next())
	{
		QString headerName = findFileName(conc, ".as");
		if ( !headerName.isEmpty() )
		{
			as << "#include \"" << findFileName(conc,".as") << ".as\"" << endl;
		}
	}
	as << endl;

	//Write class Documentation if there is somthing or if force option
	if(forceDoc() || !c->getDoc().isEmpty())
	{
		as << endl << "/**" << endl;
		as << "  * class " << classname << endl;
		as << formatDoc(c->getDoc(),"  * ");
		as << "  */" << endl << endl;
	}

	UMLClassifierList superclasses = c->getSuperClasses();
	UMLAssociationList aggregations = c->getAggregations();
	UMLAssociationList compositions = c->getCompositions();

	//check if class is abstract and / or has abstract methods
	if(c->getAbstract() && !hasAbstractOps(c))
		as << "/******************************* Abstract Class ****************************" << endl << "  "
		<< classname << " does not have any pure virtual methods, but its author" << endl
		<< "  defined it as an abstract class, so you should not use it directly." << endl
		<< "  Inherit from it instead and create only objects from the derived classes" << endl
		<< "*****************************************************************************/" << endl << endl;

	as << classname << " = function ()" << endl;
	as << "{" << endl;
	as << m_indentation << "this._init ();" << endl;
	as << "}" << endl;
	as << endl;

	for(UMLClassifier *obj = superclasses.first();
	    obj; obj = superclasses.next()) {
		as << classname << ".prototype = new " << cleanName(obj->getName()) << " ();" << endl;
	}

	as << endl;

	UMLClass *myClass = dynamic_cast<UMLClass*>(c);
	if(myClass) {

		UMLAttributeList atl = myClass->getFilteredAttributeList();

	 	as << "/**" << endl;
		QString temp = "_init sets all " + classname + " attributes to its default\
	 value make sure to call this method within your class constructor";
		as << formatDoc(temp, " * ");
		as << " */" << endl;
		as << classname << ".prototype._init = function ()" << endl;
		as << "{" << endl;
		for(UMLAttribute *at = atl.first(); at ; at = atl.next())
		{
			if (forceDoc() || !at->getDoc().isEmpty())
			{
				as << m_indentation << "/**" << endl
				 << formatDoc(at->getDoc(), m_indentation + " * ")
				 << m_indentation << " */" << endl;
			}
			if(!at->getInitialValue().isEmpty())
			{
				as << m_indentation << "this.m_" << cleanName(at->getName()) << " = " << at->getInitialValue() << ";" << endl;
			}
			else
			{
	 			as << m_indentation << "this.m_" << cleanName(at->getName()) << " = \"\";" << endl;
			}
		}
	}

	//associations
	if (forceSections() || !aggregations.isEmpty ())
	{
		as <<  m_newLineEndingChars << m_indentation << "/**Aggregations: */" << m_newLineEndingChars;
		for (UMLAssociation *a = aggregations.first(); a; a = aggregations.next())
		{
			QString nm(cleanName(a->getObject(A)->getName()));
			if (a->getMulti(A).isEmpty())
				as << m_indentation << "this.m_" << nm << " = new " << nm << " ();" << m_newLineEndingChars;
			else
				as << m_indentation << "this.m_" << nm.lower() << " = new Array ();" << m_newLineEndingChars;
		}
	}
	if( forceSections() || !compositions.isEmpty())
	{
		as <<  m_newLineEndingChars << m_indentation << "/**Compositions: */" << m_newLineEndingChars;
		for(UMLAssociation *a = compositions.first(); a; a = compositions.next())
		{
			QString nm(cleanName(a->getObject(A)->getName()));
			if(a->getMulti(A).isEmpty())
				as << m_indentation << "this.m_" << nm << " = new " << nm << " ();" << m_newLineEndingChars;
			else
				as << m_indentation << "this.m_" << nm.lower() << " = new Array ();" << m_newLineEndingChars;
		}
	}
	as << endl;

	as << m_indentation << "/**Protected: */" << m_newLineEndingChars;
	if(myClass) {
		UMLAttributeList atl = myClass->getFilteredAttributeList();
		for (UMLAttribute *at = atl.first(); at ; at = atl.next())
		{
			if (at->getScope() == Uml::Protected)
			{
				as << m_indentation << "ASSetPropFlags (this, \"" << cleanName(at->getName()) << "\", 1);" << endl;
			}
		}
	}

 	UMLOperationList opList(c->getOpList());
	for (UMLOperation *op = opList.first(); op; op = opList.next())
	{
		if (op->getScope() == Uml::Protected)
		{
			as << m_indentation << "ASSetPropFlags (this, \"" << cleanName(op->getName()) << "\", 1);" << endl;
		}
	}
	as << endl;
	as << m_indentation << "/**Private: */" << m_newLineEndingChars;
	if(myClass) {
		UMLAttributeList atl = myClass->getFilteredAttributeList();
		for (UMLAttribute *at = atl.first(); at; at = atl.next())
		{
			if (at->getScope() == Uml::Private)
			{
				as << m_indentation << "ASSetPropFlags (this, \"" << cleanName(at->getName()) << "\", 7);" << endl;
			}
		}
	}

	for (UMLOperation *op = opList.first(); op; op = opList.next())
	{
		if (op->getScope() == Uml::Protected)
		{
			as << m_indentation << "ASSetPropFlags (this, \"" << cleanName(op->getName()) << "\", 7);" << endl;
		}
	}
	as << "}" << endl;

	as << endl;

	//operations
	UMLOperationList ops(c->getOpList());
	writeOperations(classname, &ops, as);

	as << endl;

	//finish file

	//close files and notfiy we are done
	fileas.close();
	emit codeGenerated(c, true);
}

////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

void ASWriter::writeOperations(QString classname, UMLOperationList *opList, QTextStream &as)
{
	UMLOperation *op;
	UMLAttributeList *atl;
	UMLAttribute *at;

	for(op = opList->first(); op; op = opList->next())
	{
		atl = op -> getParmList();
		//write method doc if we have doc || if at least one of the params has doc
		bool writeDoc = forceDoc() || !op->getDoc().isEmpty();
		for(at = atl->first(); at ; at = atl -> next())
			writeDoc |= !at->getDoc().isEmpty();

		if( writeDoc )  //write method documentation
		{
			as << "/**" << m_newLineEndingChars << formatDoc(op->getDoc()," * ");

			for(at = atl->first(); at ; at = atl -> next())  //write parameter documentation
			{
				if(forceDoc() || !at->getDoc().isEmpty())
				{
					as << " * @param " + cleanName(at->getName())<<endl;
					as << formatDoc(at->getDoc(),"    *      ");
				}
			}//end for : write parameter documentation
			as << " */" << endl;
		}//end if : write method documentation

		as << classname << ".prototype." << cleanName(op->getName()) << " function " << "(";

		int i= atl->count();
		int j=0;
		for (at = atl->first(); at ;at = atl->next(),j++)
		{
			as << cleanName(at->getName())
				 << (!(at->getInitialValue().isEmpty()) ? (QString(" = ")+at->getInitialValue()) : QString(""))
				 << ((j < i-1)?", ":"");
		}
		as << ")" << m_newLineEndingChars << "{" << m_newLineEndingChars <<
		m_indentation << m_newLineEndingChars << "}" << m_newLineEndingChars;
		as <<  m_newLineEndingChars << endl;
	}//end for
}

/**
 * returns "ActionScript"
 */
QString ASWriter::getLanguage() {
        return "ActionScript";
}

/**
 * checks whether type is "ASWriter"
 *
 * @param type
 */
bool ASWriter::isType (QString & type)
{
   if(type == "ASWriter")
        return true;
   return false;
}

/**
 * List of reserved keywords for this code generator.
 *
 * Just add new keywords, then mark all lines and
 * pipe it through the external 'sort' program.
 */
static const char *ReservedKeywords[] = {
  "abs",
  "acos",
  "add",
  "addListener",
  "addProperty",
  "align",
  "_alpha",
  "and",
  "appendChild",
  "apply",
  "Array",
  "asin",
  "atan",
  "atan2",
  "attachMovie",
  "attachSound",
  "attributes",
  "autoSize",
  "background",
  "backgroundColor",
  "BACKSPACE",
  "beginFill",
  "beginGradientFill",
  "blockIndent",
  "bold",
  "Boolean",
  "border",
  "borderColor",
  "bottomScroll",
  "break",
  "bullet",
  "call",
  "callee",
  "caller",
  "capabilities",
  "CAPSLOCK",
  "case",
  "ceil",
  "charAt",
  "charCodeAt",
  "childNodes",
  "chr",
  "clear",
  "clearInterval",
  "cloneNode",
  "close",
  "color",
  "Color",
  "comment",
  "concat",
  "connect",
  "contentType",
  "continue",
  "CONTROL",
  "cos",
  "createElement",
  "createEmptyMovieClip",
  "createTextField",
  "createTextNode",
  "_currentframe",
  "curveTo",
  "Date",
  "default",
  "delete",
  "DELETEKEY",
  "do",
  "docTypeDecl",
  "DOWN",
  "_droptarget",
  "duplicateMovieClip",
  "duration",
  "E",
  "else",
  "embedFonts",
  "enabled",
  "END",
  "endFill",
  "endinitclip",
  "ENTER",
  "eq",
  "escape",
  "ESCAPE",
  "eval",
  "evaluate",
  "exp",
  "false",
  "firstChild",
  "floor",
  "focusEnabled",
  "_focusrect",
  "font",
  "for",
  "_framesloaded",
  "fromCharCode",
  "fscommand",
  "function",
  "ge",
  "get",
  "getAscii",
  "getBeginIndex",
  "getBounds",
  "getBytesLoaded",
  "getBytesTotal",
  "getCaretIndex",
  "getCode",
  "getDate",
  "getDay",
  "getDepth",
  "getEndIndex",
  "getFocus",
  "getFontList",
  "getFullYear",
  "getHours",
  "getMilliseconds",
  "getMinutes",
  "getMonth",
  "getNewTextFormat",
  "getPan",
  "getProperty",
  "getRGB",
  "getSeconds",
  "getTextExtent",
  "getTextFormat",
  "getTime",
  "getTimer",
  "getTimezoneOffset",
  "getTransform",
  "getURL",
  "getUTCDate",
  "getUTCDay",
  "getUTCFullYear",
  "getUTCHours",
  "getUTCMilliseconds",
  "getUTCMinutes",
  "getUTCMonth",
  "getUTCSeconds",
  "getVersion",
  "getVolume",
  "getYear",
  "_global",
  "globalToLocal",
  "goto",
  "gotoAndPlay",
  "gotoAndStop",
  "gt",
  "hasAccessibility",
  "hasAudio",
  "hasAudioEncoder",
  "hasChildNodes",
  "hasMP3",
  "hasVideoEncoder",
  "height",
  "_height",
  "hide",
  "_highquality",
  "hitArea",
  "hitTest",
  "HOME",
  "hscroll",
  "html",
  "htmlText",
  "if",
  "ifFrameLoaded",
  "ignoreWhite",
  "in",
  "include",
  "indent",
  "indexOf",
  "initclip",
  "INSERT",
  "insertBefore",
  "install",
  "instanceof",
  "int",
  "isActive",
  "isDown",
  "isFinite",
  "isNaN",
  "isToggled",
  "italic",
  "join",
  "lastChild",
  "lastIndexOf",
  "le",
  "leading",
  "LEFT",
  "leftMargin",
  "length",
  "_level",
  "lineStyle",
  "lineTo",
  "list",
  "LN10",
  "LN2",
  "load",
  "loaded",
  "loadMovie",
  "loadMovieNum",
  "loadSound",
  "loadVariables",
  "loadVariablesNum",
  "LoadVars",
  "localToGlobal",
  "log",
  "LOG10E",
  "LOG2E",
  "max",
  "maxChars",
  "maxhscroll",
  "maxscroll",
  "MAX_VALUE",
  "mbchr",
  "mblength",
  "mbord",
  "mbsubstring",
  "method",
  "min",
  "MIN_VALUE",
  "moveTo",
  "multiline",
  "_name",
  "NaN",
  "ne",
  "NEGATIVE_INFINITY",
  "new",
  "newline",
  "nextFrame",
  "nextScene",
  "nextSibling",
  "nodeName",
  "nodeType",
  "nodeValue",
  "not",
  "null",
  "Number",
  "Object",
  "on",
  "onChanged",
  "onClipEvent",
  "onClose",
  "onConnect",
  "onData",
  "onDragOut",
  "onDragOver",
  "onEnterFrame",
  "onKeyDown",
  "onKeyUp",
  "onKillFocus",
  "onLoad",
  "onMouseDown",
  "onMouseMove",
  "onMouseUp",
  "onPress",
  "onRelease",
  "onReleaseOutside",
  "onResize",
  "onRollOut",
  "onRollOver",
  "onScroller",
  "onSetFocus",
  "onSoundComplete",
  "onUnload",
  "onUpdate",
  "onXML",
  "or",
  "ord",
  "_parent",
  "parentNode",
  "parseFloat",
  "parseInt",
  "parseXML",
  "password",
  "PGDN",
  "PGUP",
  "PI",
  "pixelAspectRatio",
  "play",
  "pop",
  "position",
  "POSITIVE_INFINITY",
  "pow",
  "prevFrame",
  "previousSibling",
  "prevScene",
  "print",
  "printAsBitmap",
  "printAsBitmapNum",
  "printNum",
  "__proto__",
  "prototype",
  "push",
  "_quality",
  "random",
  "registerClass",
  "removeListener",
  "removeMovieClip",
  "removeNode",
  "removeTextField",
  "replaceSel",
  "restrict",
  "return",
  "reverse",
  "RIGHT",
  "rightMargin",
  "_root",
  "_rotation",
  "round",
  "scaleMode",
  "screenColor",
  "screenDPI",
  "screenResolutionX",
  "screenResolutionY",
  "scroll",
  "selectable",
  "send",
  "sendAndLoad",
  "set",
  "setDate",
  "setFocus",
  "setFullYear",
  "setHours",
  "setInterval",
  "setMask",
  "setMilliseconds",
  "setMinutes",
  "setMonth",
  "setNewTextFormat",
  "setPan",
  "setProperty",
  "setRGB",
  "setSeconds",
  "setSelection",
  "setTextFormat",
  "setTime",
  "setTransform",
  "setUTCDate",
  "setUTCFullYear",
  "setUTCHours",
  "setUTCMilliseconds",
  "setUTCMinutes",
  "setUTCMonth",
  "setUTCSeconds",
  "setVolume",
  "setYear",
  "shift",
  "SHIFT",
  "show",
  "showMenu",
  "sin",
  "size",
  "slice",
  "sort",
  "sortOn",
  "Sound",
  "_soundbuftime",
  "SPACE",
  "splice",
  "split",
  "sqrt",
  "SQRT1_2",
  "SQRT2",
  "start",
  "startDrag",
  "status",
  "stop",
  "stopAllSounds",
  "stopDrag",
  "String",
  "substr",
  "substring",
  "super",
  "swapDepths",
  "switch",
  "TAB",
  "tabChildren",
  "tabEnabled",
  "tabIndex",
  "tabStops",
  "tan",
  "target",
  "_target",
  "targetPath",
  "tellTarget",
  "text",
  "textColor",
  "TextFormat",
  "textHeight",
  "textWidth",
  "this",
  "toggleHighQuality",
  "toLowerCase",
  "toString",
  "_totalframes",
  "toUpperCase",
  "trace",
  "trackAsMenu",
  "true",
  "type",
  "typeof",
  "undefined",
  "underline",
  "unescape",
  "uninstall",
  "unloadMovie",
  "unloadMovieNum",
  "unshift",
  "unwatch",
  "UP",
  "updateAfterEvent",
  "url",
  "_url",
  "useHandCursor",
  "UTC",
  "valueOf",
  "var",
  "variable",
  "_visible",
  "void",
  "watch",
  "while",
  "width",
  "_width",
  "with",
  "wordWrap",
  "_x",
  "XML",
  "xmlDecl",
  "XMLSocket",
  "_xmouse",
  "_xscale",
  "_y",
  "_ymouse",
  "_yscale",
  NULL
};

/**
 * get list of reserved keywords
 */
const QPtrList<const char *> * ASWriter::getReservedKeywords() {
  if (pListOfReservedKeywords == NULL)
  {
    pListOfReservedKeywords = convertListOfReservedKeywords(ReservedKeywords);
  }

  return pListOfReservedKeywords;
}

#include "aswriter.moc"
