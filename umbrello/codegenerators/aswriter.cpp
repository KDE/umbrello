/***************************************************************************
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
#include "../classifier.h"
#include "../operation.h"
#include "../umldoc.h"
#include "../attribute.h"

#include <kdebug.h>
#include <qregexp.h>
#include <qtextstream.h>

ASWriter::ASWriter() {
}

ASWriter::~ASWriter() {}


void ASWriter::writeClass(UMLClassifier *c)
{
    if(!c)
    {
        kDebug()<<"Cannot write class of NULL concept!" << endl;
        return;
    }

    QString classname = cleanName(c->getName());
    QString fileName = c->getName().lower();

    //find an appropriate name for our file
    fileName = findFileName(c,".as");
    if (fileName.isEmpty())
    {
        emit codeGenerated(c, false);
        return;
    }

    QFile fileas;
    if(!openFile(fileas,fileName))
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
        as << str << m_endl;
    }


    //write includes
    UMLPackageList includes;
    findObjectsRelated(c,includes);
    for (UMLPackage *conc = includes.first(); conc; conc = includes.next())
    {
        QString headerName = findFileName(conc, ".as");
        if ( !headerName.isEmpty() )
        {
            as << "#include \"" << findFileName(conc,".as") << "\"" << m_endl;
        }
    }
    as << m_endl;

    //Write class Documentation if there is somthing or if force option
    if(forceDoc() || !c->getDoc().isEmpty())
    {
        as << m_endl << "/**" << m_endl;
        as << "  * class " << classname << m_endl;
        as << formatDoc(c->getDoc(),"  * ");
        as << "  */" << m_endl << m_endl;
    }

    UMLClassifierList superclasses = c->getSuperClasses();
    UMLAssociationList aggregations = c->getAggregations();
    UMLAssociationList compositions = c->getCompositions();

    //check if class is abstract and / or has abstract methods
    if(c->getAbstract() && !hasAbstractOps(c))
        as << "/******************************* Abstract Class ****************************" << m_endl << "  "
        << classname << " does not have any pure virtual methods, but its author" << m_endl
        << "  defined it as an abstract class, so you should not use it directly." << m_endl
        << "  Inherit from it instead and create only objects from the derived classes" << m_endl
        << "*****************************************************************************/" << m_endl << m_endl;

    as << classname << " = function ()" << m_endl;
    as << "{" << m_endl;
    as << m_indentation << "this._init ();" << m_endl;
    as << "}" << m_endl;
    as << m_endl;

    for(UMLClassifier *obj = superclasses.first();
            obj; obj = superclasses.next()) {
        as << classname << ".prototype = new " << cleanName(obj->getName()) << " ();" << m_endl;
    }

    as << m_endl;

    const bool isClass = !c->isInterface();
    if (isClass) {

        UMLAttributeList atl = c->getAttributeList();

        as << "/**" << m_endl;
        QString temp = "_init sets all " + classname +
                       " attributes to their default values. " +
                       "Make sure to call this method within your class constructor";
        as << formatDoc(temp, " * ");
        as << " */" << m_endl;
        as << classname << ".prototype._init = function ()" << m_endl;
        as << "{" << m_endl;
        for(UMLAttribute *at = atl.first(); at ; at = atl.next())
        {
            if (forceDoc() || !at->getDoc().isEmpty())
            {
                as << m_indentation << "/**" << m_endl
                << formatDoc(at->getDoc(), m_indentation + " * ")
                << m_indentation << " */" << m_endl;
            }
            if(!at->getInitialValue().isEmpty())
            {
                as << m_indentation << "this.m_" << cleanName(at->getName()) << " = " << at->getInitialValue() << ";" << m_endl;
            }
            else
            {
                as << m_indentation << "this.m_" << cleanName(at->getName()) << " = \"\";" << m_endl;
            }
        }
    }

    //associations
    if (forceSections() || !aggregations.isEmpty ())
    {
        as <<  m_endl << m_indentation << "/**Aggregations: */" << m_endl;
        writeAssociation(classname, aggregations , as );

    }

    if( forceSections() || !compositions.isEmpty())
    {
        as <<  m_endl << m_indentation << "/**Compositions: */" << m_endl;
        writeAssociation(classname, compositions , as );
    }

    as << m_endl;
    as << m_indentation << "/**Protected: */" << m_endl;

    if (isClass) {
        UMLAttributeList atl = c->getAttributeList();
        for (UMLAttribute *at = atl.first(); at ; at = atl.next())
        {
          if (at->getVisibility() == Uml::Visibility::Protected)
            {
                as << m_indentation << "ASSetPropFlags (this, \"" << cleanName(at->getName()) << "\", 1);" << m_endl;
            }
        }
    }

    UMLOperationList opList(c->getOpList());
    for (UMLOperation *op = opList.first(); op; op = opList.next())
    {
          if (op->getVisibility() == Uml::Visibility::Protected)
        {
            as << m_indentation << "ASSetPropFlags (this, \"" << cleanName(op->getName()) << "\", 1);" << m_endl;
        }
    }
    as << m_endl;
    as << m_indentation << "/**Private: */" << m_endl;
    if (isClass) {
        UMLAttributeList atl = c->getAttributeList();
        for (UMLAttribute *at = atl.first(); at; at = atl.next())
        {
              if (at->getVisibility() == Uml::Visibility::Private)
            {
                as << m_indentation << "ASSetPropFlags (this, \"" << cleanName(at->getName()) << "\", 7);" << m_endl;
            }
        }
    }

    for (UMLOperation *op = opList.first(); op; op = opList.next())
    {
          if (op->getVisibility() == Uml::Visibility::Protected)
        {
            as << m_indentation << "ASSetPropFlags (this, \"" << cleanName(op->getName()) << "\", 7);" << m_endl;
        }
    }
    as << "}" << m_endl;

    as << m_endl;

    //operations
    UMLOperationList ops(c->getOpList());
    writeOperations(classname, &ops, as);

    as << m_endl;

    //finish file

    //close files and notfiy we are done
    fileas.close();
    emit codeGenerated(c, true);
}

////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods


void ASWriter::writeAssociation(QString& classname, UMLAssociationList& assocList , QTextStream &as )
{
    for(UMLAssociation *a = assocList.first(); a; a = assocList.next())
    {
        // association side
        Uml::Role_Type role = a->getObject(Uml::A)->getName() == classname ? Uml::B:Uml::A;

        QString roleName(cleanName(a->getRoleName(role)));

        if (!roleName.isEmpty()) {

            // association doc
            if (forceDoc() || !a->getDoc().isEmpty()) {
                as << m_indentation << "/**" << m_endl
                << formatDoc(a->getDoc(), m_indentation + " * ")
                << m_indentation << " */" << m_endl;
            }

            // role doc
            if (forceDoc() || !a->getRoleDoc(role).isEmpty()) {
                as << m_indentation << "/**" << m_endl
                << formatDoc(a->getRoleDoc(role), m_indentation + " * ")
                << m_indentation << " */" << m_endl;
            }

            bool okCvt;
            int nMulti = a->getMulti(role).toInt(&okCvt,10);
            bool isNotMulti = a->getMulti(role).isEmpty() || (okCvt && nMulti == 1);

            QString typeName(cleanName(a->getObject(role)->getName()));

            if (isNotMulti)
                as << m_indentation << "this.m_" << roleName << " = new " << typeName << "();" << m_endl;
            else
                as << m_indentation << "this.m_" << roleName << " = new Array();" << m_endl;

            // role visibility
            if (a->getVisibility(role) == Uml::Visibility::Private)
            {
               as << m_indentation << "ASSetPropFlags (this, \"m_" << roleName << "\", 7);" << m_endl;
            }
            else if (a->getVisibility(role)== Uml::Visibility::Protected)
            {
                as << m_indentation << "ASSetPropFlags (this, \"m_" << roleName << "\", 1);" << m_endl;
            }
        }
    }
}

void ASWriter::writeOperations(QString classname, UMLOperationList *opList, QTextStream &as)
{
    UMLOperation *op;
    UMLAttributeList atl;
    UMLAttribute *at;

    for(op = opList->first(); op; op = opList->next())
    {
        atl = op -> getParmList();
        //write method doc if we have doc || if at least one of the params has doc
        bool writeDoc = forceDoc() || !op->getDoc().isEmpty();
        for (at = atl.first(); at ; at = atl.next())
            writeDoc |= !at->getDoc().isEmpty();

        if( writeDoc )  //write method documentation
        {
            as << "/**" << m_endl << formatDoc(op->getDoc()," * ");

            for (at = atl.first(); at; at = atl.next())  //write parameter documentation
            {
                if(forceDoc() || !at->getDoc().isEmpty())
                {
                    as << " * @param " + cleanName(at->getName())<<m_endl;
                    as << formatDoc(at->getDoc(),"    *      ");
                }
            }//end for : write parameter documentation
            as << " */" << m_endl;
        }//end if : write method documentation

        as << classname << ".prototype." << cleanName(op->getName()) << " = function " << "(";

        int i= atl.count();
        int j=0;
        for (at = atl.first(); at; at = atl.next(),j++)
        {
            as << cleanName(at->getName())
            << (!(at->getInitialValue().isEmpty()) ? (QString(" = ")+at->getInitialValue()) : QString(""))
            << ((j < i-1)?", ":"");
        }
        as << ")" << m_endl << "{" << m_endl <<
        m_indentation << m_endl << "}" << m_endl;
        as <<  m_endl << m_endl;
    }//end for
}

/**
 * returns "ActionScript"
 */
Uml::Programming_Language ASWriter::getLanguage() {
    return Uml::pl_ActionScript;
}

const QStringList ASWriter::reservedKeywords() const {

    static QStringList keywords;

    if ( keywords.isEmpty() ) {
        keywords << "abs"
        << "acos"
        << "add"
        << "addListener"
        << "addProperty"
        << "align"
        << "_alpha"
        << "and"
        << "appendChild"
        << "apply"
        << "Array"
        << "asin"
        << "atan"
        << "atan2"
        << "attachMovie"
        << "attachSound"
        << "attributes"
        << "autoSize"
        << "background"
        << "backgroundColor"
        << "BACKSPACE"
        << "beginFill"
        << "beginGradientFill"
        << "blockIndent"
        << "bold"
        << "Boolean"
        << "border"
        << "borderColor"
        << "bottomScroll"
        << "break"
        << "bullet"
        << "call"
        << "callee"
        << "caller"
        << "capabilities"
        << "CAPSLOCK"
        << "case"
        << "ceil"
        << "charAt"
        << "charCodeAt"
        << "childNodes"
        << "chr"
        << "clear"
        << "clearInterval"
        << "cloneNode"
        << "close"
        << "color"
        << "Color"
        << "comment"
        << "concat"
        << "connect"
        << "contentType"
        << "continue"
        << "CONTROL"
        << "cos"
        << "createElement"
        << "createEmptyMovieClip"
        << "createTextField"
        << "createTextNode"
        << "_currentframe"
        << "curveTo"
        << "Date"
        << "default"
        << "delete"
        << "DELETEKEY"
        << "do"
        << "docTypeDecl"
        << "DOWN"
        << "_droptarget"
        << "duplicateMovieClip"
        << "duration"
        << "E"
        << "else"
        << "embedFonts"
        << "enabled"
        << "END"
        << "endFill"
        << "endinitclip"
        << "ENTER"
        << "eq"
        << "escape"
        << "ESCAPE"
        << "eval"
        << "evaluate"
        << "exp"
        << "false"
        << "firstChild"
        << "floor"
        << "focusEnabled"
        << "_focusrect"
        << "font"
        << "for"
        << "_framesloaded"
        << "fromCharCode"
        << "fscommand"
        << "function"
        << "ge"
        << "get"
        << "getAscii"
        << "getBeginIndex"
        << "getBounds"
        << "getBytesLoaded"
        << "getBytesTotal"
        << "getCaretIndex"
        << "getCode"
        << "getDate"
        << "getDay"
        << "getDepth"
        << "getEndIndex"
        << "getFocus"
        << "getFontList"
        << "getFullYear"
        << "getHours"
        << "getMilliseconds"
        << "getMinutes"
        << "getMonth"
        << "getNewTextFormat"
        << "getPan"
        << "getProperty"
        << "getRGB"
        << "getSeconds"
        << "getTextExtent"
        << "getTextFormat"
        << "getTime"
        << "getTimer"
        << "getTimezoneOffset"
        << "getTransform"
        << "getURL"
        << "getUTCDate"
        << "getUTCDay"
        << "getUTCFullYear"
        << "getUTCHours"
        << "getUTCMilliseconds"
        << "getUTCMinutes"
        << "getUTCMonth"
        << "getUTCSeconds"
        << "getVersion"
        << "getVolume"
        << "getYear"
        << "_global"
        << "globalToLocal"
        << "goto"
        << "gotoAndPlay"
        << "gotoAndStop"
        << "gt"
        << "hasAccessibility"
        << "hasAudio"
        << "hasAudioEncoder"
        << "hasChildNodes"
        << "hasMP3"
        << "hasVideoEncoder"
        << "height"
        << "_height"
        << "hide"
        << "_highquality"
        << "hitArea"
        << "hitTest"
        << "HOME"
        << "hscroll"
        << "html"
        << "htmlText"
        << "if"
        << "ifFrameLoaded"
        << "ignoreWhite"
        << "in"
        << "include"
        << "indent"
        << "indexOf"
        << "initclip"
        << "INSERT"
        << "insertBefore"
        << "install"
        << "instanceof"
        << "int"
        << "isActive"
        << "isDown"
        << "isFinite"
        << "isNaN"
        << "isToggled"
        << "italic"
        << "join"
        << "lastChild"
        << "lastIndexOf"
        << "le"
        << "leading"
        << "LEFT"
        << "leftMargin"
        << "length"
        << "_level"
        << "lineStyle"
        << "lineTo"
        << "list"
        << "LN10"
        << "LN2"
        << "load"
        << "loaded"
        << "loadMovie"
        << "loadMovieNum"
        << "loadSound"
        << "loadVariables"
        << "loadVariablesNum"
        << "LoadVars"
        << "localToGlobal"
        << "log"
        << "LOG10E"
        << "LOG2E"
        << "max"
        << "maxChars"
        << "maxhscroll"
        << "maxscroll"
        << "MAX_VALUE"
        << "mbchr"
        << "mblength"
        << "mbord"
        << "mbsubstring"
        << "method"
        << "min"
        << "MIN_VALUE"
        << "moveTo"
        << "multiline"
        << "_name"
        << "NaN"
        << "ne"
        << "NEGATIVE_INFINITY"
        << "new"
        << "newline"
        << "nextFrame"
        << "nextScene"
        << "nextSibling"
        << "nodeName"
        << "nodeType"
        << "nodeValue"
        << "not"
        << "null"
        << "Number"
        << "Object"
        << "on"
        << "onChanged"
        << "onClipEvent"
        << "onClose"
        << "onConnect"
        << "onData"
        << "onDragOut"
        << "onDragOver"
        << "onEnterFrame"
        << "onKeyDown"
        << "onKeyUp"
        << "onKillFocus"
        << "onLoad"
        << "onMouseDown"
        << "onMouseMove"
        << "onMouseUp"
        << "onPress"
        << "onRelease"
        << "onReleaseOutside"
        << "onResize"
        << "onRollOut"
        << "onRollOver"
        << "onScroller"
        << "onSetFocus"
        << "onSoundComplete"
        << "onUnload"
        << "onUpdate"
        << "onXML"
        << "or"
        << "ord"
        << "_parent"
        << "parentNode"
        << "parseFloat"
        << "parseInt"
        << "parseXML"
        << "password"
        << "PGDN"
        << "PGUP"
        << "PI"
        << "pixelAspectRatio"
        << "play"
        << "pop"
        << "position"
        << "POSITIVE_INFINITY"
        << "pow"
        << "prevFrame"
        << "previousSibling"
        << "prevScene"
        << "print"
        << "printAsBitmap"
        << "printAsBitmapNum"
        << "printNum"
        << "__proto__"
        << "prototype"
        << "push"
        << "_quality"
        << "random"
        << "registerClass"
        << "removeListener"
        << "removeMovieClip"
        << "removeNode"
        << "removeTextField"
        << "replaceSel"
        << "restrict"
        << "return"
        << "reverse"
        << "RIGHT"
        << "rightMargin"
        << "_root"
        << "_rotation"
        << "round"
        << "scaleMode"
        << "screenColor"
        << "screenDPI"
        << "screenResolutionX"
        << "screenResolutionY"
        << "scroll"
        << "selectable"
        << "send"
        << "sendAndLoad"
        << "set"
        << "setDate"
        << "setFocus"
        << "setFullYear"
        << "setHours"
        << "setInterval"
        << "setMask"
        << "setMilliseconds"
        << "setMinutes"
        << "setMonth"
        << "setNewTextFormat"
        << "setPan"
        << "setProperty"
        << "setRGB"
        << "setSeconds"
        << "setSelection"
        << "setTextFormat"
        << "setTime"
        << "setTransform"
        << "setUTCDate"
        << "setUTCFullYear"
        << "setUTCHours"
        << "setUTCMilliseconds"
        << "setUTCMinutes"
        << "setUTCMonth"
        << "setUTCSeconds"
        << "setVolume"
        << "setYear"
        << "shift"
        << "SHIFT"
        << "show"
        << "showMenu"
        << "sin"
        << "size"
        << "slice"
        << "sort"
        << "sortOn"
        << "Sound"
        << "_soundbuftime"
        << "SPACE"
        << "splice"
        << "split"
        << "sqrt"
        << "SQRT1_2"
        << "SQRT2"
        << "start"
        << "startDrag"
        << "status"
        << "stop"
        << "stopAllSounds"
        << "stopDrag"
        << "String"
        << "substr"
        << "substring"
        << "super"
        << "swapDepths"
        << "switch"
        << "TAB"
        << "tabChildren"
        << "tabEnabled"
        << "tabIndex"
        << "tabStops"
        << "tan"
        << "target"
        << "_target"
        << "targetPath"
        << "tellTarget"
        << "text"
        << "textColor"
        << "TextFormat"
        << "textHeight"
        << "textWidth"
        << "this"
        << "toggleHighQuality"
        << "toLowerCase"
        << "toString"
        << "_totalframes"
        << "toUpperCase"
        << "trace"
        << "trackAsMenu"
        << "true"
        << "type"
        << "typeof"
        << "undefined"
        << "underline"
        << "unescape"
        << "uninstall"
        << "unloadMovie"
        << "unloadMovieNum"
        << "unshift"
        << "unwatch"
        << "UP"
        << "updateAfterEvent"
        << "url"
        << "_url"
        << "useHandCursor"
        << "UTC"
        << "valueOf"
        << "var"
        << "variable"
        << "_visible"
        << "void"
        << "watch"
        << "while"
        << "width"
        << "_width"
        << "with"
        << "wordWrap"
        << "_x"
        << "XML"
        << "xmlDecl"
        << "XMLSocket"
        << "_xmouse"
        << "_xscale"
        << "_y"
        << "_ymouse";
    }

    return keywords;
}

#include "aswriter.moc"
