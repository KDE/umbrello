/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Alexander Blum <blum@kewbee.de>               *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "aswriter.h"

#include "association.h"
#include "attribute.h"
#include "classifier.h"
#include "debug_utils.h"
#include "operation.h"
#include "umldoc.h"

#include <QtCore/QRegExp>
#include <QtCore/QTextStream>

static const char *reserved_words[] = {
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
    0
};

/**
 * Constructor.
 */
ASWriter::ASWriter()
{
}

/**
 * Destructor.
 */
ASWriter::~ASWriter()
{
}

/**
 * Call this method to generate Actionscript code for a UMLClassifier.
 * @param c   the class you want to generate code for
 */
void ASWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        uDebug()<<"Cannot write class of NULL concept!";
        return;
    }

    QString classname = cleanName(c->name());
    QString fileName = c->name().toLower();

    //find an appropriate name for our file
    fileName = findFileName(c,".as");
    if (fileName.isEmpty())
    {
        emit codeGenerated(c, false);
        return;
    }

    QFile fileas;
    if (!openFile(fileas,fileName))
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
    if (!str.isEmpty())
    {
        str.replace(QRegExp("%filename%"),fileName+".as");
        str.replace(QRegExp("%filepath%"),fileas.fileName());
        as << str << m_endl;
    }

    //write includes
    UMLPackageList includes;
    findObjectsRelated(c,includes);
    foreach (UMLPackage* conc, includes ) {
        QString headerName = findFileName(conc, ".as");
        if ( !headerName.isEmpty() )
        {
            as << "#include \"" << findFileName(conc,".as") << "\"" << m_endl;
        }
    }
    as << m_endl;

    //Write class Documentation if there is somthing or if force option
    if (forceDoc() || !c->doc().isEmpty())
    {
        as << m_endl << "/**" << m_endl;
        as << "  * class " << classname << m_endl;
        as << formatDoc(c->doc(),"  * ");
        as << "  */" << m_endl << m_endl;
    }

    UMLClassifierList superclasses = c->getSuperClasses();
    UMLAssociationList aggregations = c->getAggregations();
    UMLAssociationList compositions = c->getCompositions();

    //check if class is abstract and / or has abstract methods
    if (c->isAbstract() && !hasAbstractOps(c))
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

    foreach(UMLClassifier* obj, superclasses ) {
        as << classname << ".prototype = new " << cleanName(obj->name()) << " ();" << m_endl;
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
        foreach (UMLAttribute* at, atl ) {
            if (forceDoc() || !at->doc().isEmpty())
            {
                as << m_indentation << "/**" << m_endl
                << formatDoc(at->doc(), m_indentation + " * ")
                << m_indentation << " */" << m_endl;
            }
            if(!at->getInitialValue().isEmpty())
            {
                as << m_indentation << "this.m_" << cleanName(at->name()) << " = " << at->getInitialValue() << ";" << m_endl;
            }
            else
            {
                as << m_indentation << "this.m_" << cleanName(at->name()) << " = \"\";" << m_endl;
            }
        }
    }

    //associations
    if (forceSections() || !aggregations.isEmpty())
    {
        as <<  m_endl << m_indentation << "/**Aggregations: */" << m_endl;
        writeAssociation(classname, aggregations , as );
    }

    if (forceSections() || !compositions.isEmpty())
    {
        as <<  m_endl << m_indentation << "/**Compositions: */" << m_endl;
        writeAssociation(classname, compositions , as );
    }

    as << m_endl;
    as << m_indentation << "/**Protected: */" << m_endl;

    if (isClass) {
        UMLAttributeList atl = c->getAttributeList();
        foreach (UMLAttribute* at, atl ) {
          if (at->visibility() == Uml::Visibility::Protected) {
                as << m_indentation << "ASSetPropFlags (this, \"" << cleanName(at->name()) << "\", 1);" << m_endl;
          }
        }
    }

    UMLOperationList opList(c->getOpList());
    foreach (UMLOperation* op, opList ) {
        if (op->visibility() == Uml::Visibility::Protected) {
            as << m_indentation << "ASSetPropFlags (this, \"" << cleanName(op->name()) << "\", 1);" << m_endl;
        }
    }
    as << m_endl;
    as << m_indentation << "/**Private: */" << m_endl;
    if (isClass) {
        UMLAttributeList atl = c->getAttributeList();
        foreach (UMLAttribute* at,  atl ) {
            if (at->visibility() == Uml::Visibility::Private) {
                as << m_indentation << "ASSetPropFlags (this, \"" << cleanName(at->name()) << "\", 7);" << m_endl;
            }
        }
    }

    foreach (UMLOperation* op, opList ) {
        if (op->visibility() == Uml::Visibility::Protected) {
            as << m_indentation << "ASSetPropFlags (this, \"" << cleanName(op->name()) << "\", 7);" << m_endl;
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

/**
 * Write a list of associations.
 *
 * @param classname   the name of the class
 * @param assocList   the list of associations
 * @param as          output stream for the AS file
 */
void ASWriter::writeAssociation(QString& classname, UMLAssociationList& assocList , QTextStream &as )
{
    foreach (UMLAssociation *a , assocList )
    {
        // association side
        Uml::Role_Type role = a->getObject(Uml::A)->name() == classname ? Uml::B:Uml::A;

        QString roleName(cleanName(a->getRoleName(role)));

        if (!roleName.isEmpty()) {

            // association doc
            if (forceDoc() || !a->doc().isEmpty()) {
                as << m_indentation << "/**" << m_endl
                << formatDoc(a->doc(), m_indentation + " * ")
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

            QString typeName(cleanName(a->getObject(role)->name()));

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

/**
 * Write a list of class operations.
 *
 * @param classname   the name of the class
 * @param opList      the list of operations
 * @param as          output stream for the AS file
 */
void ASWriter::writeOperations(QString classname, UMLOperationList *opList, QTextStream &as)
{
    UMLAttributeList atl;

    foreach (UMLOperation* op , *opList ) {
        atl = op -> getParmList();
        //write method doc if we have doc || if at least one of the params has doc
        bool writeDoc = forceDoc() || !op->doc().isEmpty();
        foreach (UMLAttribute* at,  atl  ) {
            writeDoc |= !at->doc().isEmpty();
        }

        if( writeDoc )  //write method documentation
        {
            as << "/**" << m_endl << formatDoc(op->doc()," * ");

            foreach (UMLAttribute* at,  atl ) {
                if(forceDoc() || !at->doc().isEmpty()) {
                    as << " * @param " + cleanName(at->name())<<m_endl;
                    as << formatDoc(at->doc(),"    *      ");
                }
            }//end for : write parameter documentation
            as << " */" << m_endl;
        }//end if : write method documentation

        as << classname << ".prototype." << cleanName(op->name()) << " = function " << "(";

        int i= atl.count();
        int j=0;
        for (UMLAttributeListIt atlIt( atl ); atlIt.hasNext(); ++j) {
            UMLAttribute* at = atlIt.next();
            as << cleanName(at->name())
            << (!(at->getInitialValue().isEmpty()) ? (QString(" = ")+at->getInitialValue()) : QString(""))
            << ((j < i-1)?", ":"");
        }
        as << ")" << m_endl << "{" << m_endl;
        QString sourceCode = op->getSourceCode();
        if (sourceCode.isEmpty()) {
            as << m_indentation << m_endl;
        }
        else {
            as << formatSourceCode(sourceCode, m_indentation);
        }
        as << "}" << m_endl;
        as <<  m_endl << m_endl;
    }//end for
}

/**
 * Returns "ActionScript".
 * @return   the programming language identifier
 */
Uml::ProgrammingLanguage ASWriter::language() const
{
    return Uml::ProgrammingLanguage::ActionScript;
}

/**
 * Get list of reserved keywords.
 * @return   the list of reserved keywords
 */
QStringList ASWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        for (int i = 0; reserved_words[i]; ++i) {
            keywords.append(reserved_words[i]);
        }
    }

    return keywords;
}

#include "aswriter.moc"
