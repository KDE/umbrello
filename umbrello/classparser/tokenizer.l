/***************************************************************************
                          tokenizer.l  -  description
                             -------------------
    begin                : Mon Mar 15 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@syncom.se
   
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
%Start COMMENT PREPARSER PPCOMMENT

%{
  #include <cstdlib>
  #include <iostream>
  #include <qstring.h>
  #include "tokenizer.h"
  #include "FlexLexer.h"

  /* Line where a comment starts. */
  int comment_start = -1;

  /* Line where a comment ends. */
  int comment_end = -1;

  /* Last parsed comment. */
  QString comment;

  /* Function prototypes */
  bool beginComment(yyFlexLexer *lex);
  void inComment(yyFlexLexer *lex);
  bool endComment(yyFlexLexer *lex);
%}

WS           [[:blank:]\r]+
LETTER       [A-Za-z_~]
DIGIT        [0-9]
NUM          {DIGIT}+
ID           {LETTER}+({LETTER}|{DIGIT})*

SIMPLE_ESCAPE [abfnrtv'"?\\]
OCTAL_ESCAPE  [0-7]{1,3}
HEX_ESCAPE "x"[0-9a-fA-F]+

ESCAPE_SEQUENCE [\\]({SIMPLE_ESCAPE}|{OCTAL_ESCAPE}|{HEX_ESCAPE})
CCHAR [^'\\\n]|{ESCAPE_SEQUENCE}
SCHAR [^"\\\n]|{ESCAPE_SEQUENCE}

%option yylineno noyywrap
%option C++ yymore

%%
    /*==============> Strip whitespace <==============*/
<INITIAL,PREPARSER>{WS}         {}

    /*==============> Special keyword <==============*/
<INITIAL>"class"         {return CPCLASS;}
<INITIAL>"public"        {return CPPUBLIC;}
<INITIAL>"protected"     {return CPPROTECTED;}
<INITIAL>"private"       {return CPPRIVATE;}
<INITIAL>"const"         {return CPCONST;}
<INITIAL>"volatile"      {return CPVOLATILE;}
<INITIAL>"static"        {return CPSTATIC;}
<INITIAL>"virtual"       {return CPVIRTUAL;}
<INITIAL>"signals"       {return QTSIGNAL;}
<INITIAL>"slots"         {return QTSLOT;}
<INITIAL>"enum"          {return CPENUM;}
<INITIAL>"typedef"       {return CPTYPEDEF;}
<INITIAL>"friend"        {return CPFRIEND;}
<INITIAL>"struct"        {return CPSTRUCT;}
<INITIAL>"union"         {return CPUNION;}
<INITIAL>"operator"      {return CPOPERATOR;}
<INITIAL>"template"      {return CPTEMPLATE;}
<INITIAL>"throw"         {return CPTHROW;}
<INITIAL>"namespace"     {return CPNAMESPACE;}

    /*==============> Ignored stuff <==============*/
    /*========== added to avoid extern "C" trouble--rnolden 03232000====*/
<INITIAL>"Q_EXPORT"       {}
<INITIAL>"Q_EXPORTH"      {}
<INITIAL>"Q_OBJECT"       {}
<INITIAL>"__BEGIN_DECLS"  {}
<INITIAL>"__END_DECLS"    {}
<INITIAL>"inline"         {}
<INITIAL>"afx_msg"        {}
<INITIAL>"extern"         {}

    /*==============> String <==============*/
<INITIAL>"L"?["]{SCHAR}*["]     {return STRING;}
<INITIAL>"L"?[']{CCHAR}+[']    {return STRING;}

    /*==============> Normal operators <==============*/
<INITIAL>":"          {return ':';}
<INITIAL>"::"         {return CLCL;}
<INITIAL>"~"          {return '~';}
<INITIAL>"="          {return '=';}
<INITIAL>"<"          {return '<';}
<INITIAL>">"          {return '>';}
<INITIAL>"&"          {return '&';}
<INITIAL>"+"          {return '+';}
<INITIAL>"-"          {return '-';}
<INITIAL>"*"          {return '*';}
<INITIAL>"/"          {return '/';}
<INITIAL>"%"          {return '%';}
<INITIAL>"|"          {return '|';}
<INITIAL>"^"          {return '^';}
<INITIAL>"!"          {return '!';}

    /*==============> End of Phrase <==============*/
<INITIAL>";"          {return ';';}

    /*==============> Comma <==============*/
<INITIAL>","          {return ',';}

    /*==============> Paranthesis <==============*/
<INITIAL>"("          {return '(';}
<INITIAL>")"          {return ')';}

    /*==============> Brackets <==============*/
<INITIAL>"{"          {return '{';}
<INITIAL>"}"          {return '}';}
<INITIAL>"["          {return '[';}
<INITIAL>"]"          {return ']';}

    /*==============> Identifiers <==============*/
<INITIAL>{ID}         {return IDEN;}

    /*==============> Numbers <==============*/
<INITIAL>{NUM}        {return NUM;}

    /*==============> Skip preprocessor directives <==============*/
<INITIAL>"#"[acdeimpsuw ]	{BEGIN (PREPARSER);}
<PREPARSER>[^\\]"\n"	{ BEGIN (0);}
<PREPARSER>"/*"		{beginComment((yyFlexLexer *) this);
                                  BEGIN (PPCOMMENT);}
<PPCOMMENT>"*/"        { endComment((yyFlexLexer *) this);
                       BEGIN (PREPARSER); }

    /*==============> Strip // comments <==============*/
<INITIAL>"//"(.)*[\n<<EOF>>] {}

    /*==============> Strip comments <==============*/
<INITIAL>"/*"        {beginComment((yyFlexLexer *) this);
                                  BEGIN (COMMENT);}

<COMMENT>"*/"        { endComment((yyFlexLexer *) this);
                       BEGIN (0); }

<COMMENT,PPCOMMENT>[\n\r]  {inComment((yyFlexLexer *) this);}
<COMMENT,PPCOMMENT>.  {inComment((yyFlexLexer *) this);}

    /*==============> All other characters are ignored <==============*/
<INITIAL,PREPARSER>[\n]  	{}
<INITIAL,PREPARSER>.  		{}

%%

bool beginComment(yyFlexLexer *lex)
{
 comment_start = lex->lineno();
 comment="/*";
 return true;
}

void inComment(yyFlexLexer *lex)
{
  const char *text=lex->YYText();
  comment+=text;
};

bool endComment(yyFlexLexer *lex)
{
 comment_end = lex->lineno();
 comment+="*/";
 return true;
}

