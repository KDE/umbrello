-------------------------------------------------------------------------------
-- Copyright (c) 2008 Niko Sams <niko.sams@gmail.com>
--
-- This grammar is free software; you can redistribute it and/or
-- modify it under the terms of the GNU Library General Public
-- License as published by the Free Software Foundation; either
-- version 2 of the License, or (at your option) any later version.
--
-- This grammar is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Library General Public License
-- along with this library; see the file COPYING.LIB.  If not, write to
-- the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
-- Boston, MA 02110-1301, USA.
-----------------------------------------------------------


-----------------------------------------------------------
-- Grammar for PHP 5.2
-- Modelled after the Zend Grammar shipped with PHP5.2
-- source, the PHP Language Reference documentation,
-- and parts taken from KDevelop Java Grammar
-----------------------------------------------------------

-- 4 first/first conflicts:
--  - var_expression: variable vs. varExpressionNormal
--    no problem because of ifs that allow always just one rule
--  - classNameReference: STRING vs. staticMember (foo vs. foo::$bar)
--    resolved by LA()
--  - encapsVar: STRING_VARNAME LBRACKET vs. expr (expr allows STRING_VARNAME too - but not LBRACKET)
--    resolved by LA()
--  - constantOrClassConst: constant vs. class constant (FOO v.s Cls::FOO)
--    resolved by LA() (could be avoided, but the Ast is much cleaner that way)
-- 1 first/follow conflicts:
--  - elseifList: dangling-else conflict - should be ok

-- TODO: (post 1.0.0 release)
--  1) decrease memory consumption
--    1.1) use quint32 instead of qint64 for end/start tokens
--    1.2) investigate whether using a map/hash for the ducontext member of all
--         ast nodes gives a significant memory decrease while not hampering performance
--    1.3) investigate how unions could be used for exclusive AST node members
--    1.4) see whether we can always use the expression lists instead of both
--         single member pointer and list of members, esp. in expressions
--  2) better cope with invalid code, have at least a partial AST
--  3) investigate whether expanding the visitor lookup to a
--     (albeit huge) switch() in KDev-PG-Qt gives a significant performance gain
--     I have the gut feeling that the current lookup takes unnecessary much time
--

------------------------------------------------------------
-- Forward declaration in phpast.h
------------------------------------------------------------

[:

#include <QtCore/QRegularExpression>

namespace KDevelop
{
    class DUContext;
}

:]

------------------------------------------------------------
-- Additional includes for the parser
------------------------------------------------------------

%parser_declaration_header "tokenstream.h"
%parser_declaration_header "QtCore/QString"
%parser_declaration_header "language/duchain/problem.h"
%parser_declaration_header "phplexer.h"

%parser_bits_header "parserdebug.h"

------------------------------------------------------------
-- Export macro to use the parser in a shared lib
------------------------------------------------------------
%export_macro "KDEVPHPPARSER_EXPORT"
%export_macro_header "parserexport.h"


------------------------------------------------------------
-- Enumeration types for additional AST members,
-- in the global "Php" namespace
------------------------------------------------------------
%namespace
[:
    class Lexer;

    enum ModifierFlags {
        ModifierPrivate      = 1,
        ModifierPublic       = 1 << 1,
        ModifierProtected    = 1 << 2,
        ModifierStatic       = 1 << 3,
        ModifierFinal        = 1 << 4,
        ModifierAbstract     = 1 << 5
    };

    enum ClassModifier {
        NormalClass,
        AbstractClass,
        FinalClass
    };

    enum ScalarTypes {
        ScalarTypeInt,
        ScalarTypeFloat,
        ScalarTypeString
    };

    enum CastType {
        CastInt,
        CastDouble,
        CastString,
        CastArray,
        CastObject,
        CastBool,
        CastUnset
    };

    enum OperationType {
        OperationPlus = 1,
        OperationMinus,
        OperationConcat,
        OperationMul,
        OperationDiv,
        OperationExp,
        OperationMod,
        OperationAnd,
        OperationOr,
        OperationXor,
        OperationSl,
        OperationSr
    };
:]

------------------------------------------------------------
-- Ast Node class members
------------------------------------------------------------
%ast_extra_members
[:
  KDevelop::DUContext* ducontext;
:]

------------------------------------------------------------
-- Parser class members
------------------------------------------------------------

%parserclass (public declaration)
[:
  /**
   * Transform the raw input into tokens.
   * When this method returns, the parser's token stream has been filled
   * and any parse*() method can be called.
   */
  void tokenize(const QString& contents, int initialState = Lexer::HtmlState);

  enum ProblemType {
      Error,
      Warning,
      Info,
      Todo
  };
  KDevelop::ProblemPointer reportProblem( Parser::ProblemType type, const QString& message, int tokenOffset = -1 );
  QList<KDevelop::ProblemPointer> problems() {
      return m_problems;
  }
  QString tokenText(qint64 begin, qint64 end);
  void setDebug(bool debug);
  void setCurrentDocument(KDevelop::IndexedString url);
  void setTodoMarkers(const QStringList& markers);
  void extractTodosFromComment(const QString& comment, qint64 offset);

    enum InitialLexerState {
        HtmlState = 0,
        DefaultState = 1
    };

:]

%parserclass (private declaration)
[:
    enum VarExpressionState {
        Normal,
        OnlyVariable,
        OnlyNewObject
    };
    QString m_contents;
    bool m_debug;
    KDevelop::IndexedString m_currentDocument;
    QList<KDevelop::ProblemPointer> m_problems;

    struct ParserState {
        VarExpressionState varExpressionState;
        bool varExpressionIsVariable;
    };
    ParserState m_state;

    QRegularExpression m_todoMarkers;
:]

%parserclass (constructor)
[:
    m_state.varExpressionState = Normal;
    m_state.varExpressionIsVariable = false;
:]


%token_stream TokenStream ;;

-----------------------------------------------------------
-- List of defined tokens
-----------------------------------------------------------

-- keywords:
%token ABSTRACT ("abstract"), BREAK ("break"), CASE ("case"), CATCH ("catch"),
       CLASS ("class"), CONST ("const"), CONTINUE ("continue"),
       DEFAULT ("default"), DO ("do"), ELSE ("else"), EXTENDS ("extends"),
       FINAL ("final"), FOR ("for"), IF ("if"),
       IMPLEMENTS ("implements"), INSTANCEOF ("instanceof"), INTERFACE ("interface"),
       NEW ("new"), PRIVATE ("private"), PROTECTED ("protected"), PUBLIC ("public"),
       RETURN ("return"), STATIC ("static"), SWITCH ("switch"), THROW ("throw"),
       TRY ("try"), WHILE ("while"), ECHO ("echo"), PRINT ("print"), FINALLY ("finally"),
       CLONE ("clone"), EXIT ("exit"), ELSEIF ("elseif"), ENDIF ("endif"),
       ENDWHILE ("endwhile"), ENDFOR ("endfor"), FOREACH ("foreach"), ENDFOREACH ("endforeach"),
       DECLARE ("declare"), ENDDECLARE ("enddeclare"), AS ("as"), ENDSWITCH ("endswitch"),
       FUNCTION ("function"), USE ("use"), GLOBAL ("global"), VAR ("var "), UNSET ("unset"),
       ISSET ("isset"), EMPTY ("empty"), HALT_COMPILER ("halt compiler"),
       DOUBLE_ARROW ("=>"), LIST ("list"), ARRAY ("array"), CLASS_C ("__CLASS__"),
       METHOD_C ("__METHOD__"), FUNC_C ("__FUNCTION__"), LINE ("__LINE__"),
       FILE ("__FILE__"), COMMENT ("comment"), DOC_COMMENT ("doc comment"),  PAAMAYIM_NEKUDOTAYIM ("::"),
       INCLUDE ("include"), INCLUDE_ONCE ("include_once"), EVAL ("eval"), REQUIRE ("require"),
       REQUIRE_ONCE ("require_once"), NAMESPACE ("namespace"), NAMESPACE_C("__NAMESPACE__"), USE("use"),
       GOTO ("goto"), TRAIT ("trait"), INSTEADOF ("insteadof"), CALLABLE ("callable") ;;

-- casts:
%token INT_CAST ("int cast"), DOUBLE_CAST ("double cast"), STRING_CAST ("string cast"),
       ARRAY_CAST ("array cast"), OBJECT_CAST ("object cast"), BOOL_CAST ("bool cast"),
       UNSET_CAST ("unset cast") ;;

-- seperators:
%token SEMICOLON (";"), DOUBLE_QUOTE ("\""), LBRACKET ("["),
       RBRACKET ("]"),
       LPAREN ("("), RPAREN (")"), LBRACE ("{"), RBRACE ("}"),
       COMMA (","), AT ("@"),
       CURLY_OPEN ("curly open"), -- { in "{$foo}"; not the same as LBRACE
       DOLLAR_OPEN_CURLY_BRACES ("${"),
       START_HEREDOC ("start heredoc"), END_HEREDOC ("end heredoc"),
       BACKTICK ("`"), BACKSLASH ("\\"),
       START_NOWDOC("start nowdoc"), END_NOWDOC("end nowdoc") ;;

-- operators:
%token IS_EQUAL ("=="), IS_NOT_EQUAL ("!="), IS_IDENTICAL ("==="),
       IS_NOT_IDENTICAL ("!=="), IS_SMALLER ("<"), IS_GREATER (">"),
       IS_SMALLER_OR_EQUAL ("<="), IS_GREATER_OR_EQUAL (">="),
       BOOLEAN_OR ("||"), BOOLEAN_AND ("&&"), ASSIGN ("="), EXP_ASSIGN("**="),
       PLUS_ASSIGN ("+="), MINUS_ASSIGN ("-="), MUL_ASSIGN ("*="), DIV_ASSIGN ("/="),
       CONCAT_ASSIGN (".="), MOD_ASSIGN ("%="), AND_ASSIGN ("&="), OR_ASSIGN ("|="),
       XOR_ASSIGN ("^="), SL_ASSIGN ("<<="), SR_ASSIGN (">>="), OBJECT_OPERATOR ("->"),
       PLUS ("+"), MINUS("-"), CONCAT("."),
       INC ("++"), DEC ("--"), BANG ("!"), QUESTION ("?"), COLON (":"),
       BIT_AND ("&"), BIT_OR("|"), BIT_XOR ("^"),
       SL ("<<"), SR (">>"), MUL("*"), DIV("/"), MOD ("%"),
       TILDE ("~"), DOLLAR ("$"), EXP ("**"),
       LOGICAL_OR ("logical or"), LOGICAL_AND ("logical and"), LOGICAL_XOR ("logical xor") ;;

-- literals and identifiers:
%token INLINE_HTML ("inline html"), WHITESPACE ("whitespace"),
       CONSTANT_ENCAPSED_STRING ("constant encapsed string"),
       VARIABLE ("variable"), ENCAPSED_AND_WHITESPACE ("encapsed and whitespace"),
       DNUMBER ("double number"), LNUMBER ("long number"),
       NUM_STRING ("num string"), STRING ("string"),
       STRING_VARNAME ("string varname") ;; -- when in "${varname}"

-- open/close tags
%token OPEN_TAG ("<?"), CLOSE_TAG ("?>"), OPEN_TAG_WITH_ECHO ("<?=");;

-- token that makes the parser fail in any case:
%token INVALID ("invalid token") ;;



-- The actual grammar starts here.

#statements=outerTopStatement*
-> start ;;

  namespaceDeclaration=namespaceDeclarationStatement
  | statement=topStatement
-> outerTopStatement ;;

-- first/first conflict for FUNCTION
   (?[: (LA(1).kind == Token_FUNCTION && ((LA(2).kind == Token_BIT_AND && LA(3).kind == Token_LPAREN)
            || LA(2).kind == Token_LPAREN))
        || LA(1).kind != Token_FUNCTION :]
    statement=statement )
  | functionDeclaration=functionDeclarationStatement
  | classDeclaration=classDeclarationStatement
  | traitDeclaration=traitDeclarationStatement
  | interfaceDeclaration=interfaceDeclarationStatement
  | HALT_COMPILER LPAREN RPAREN SEMICOLON -- Lexer stops allready
-> topStatement ;;

[: bool reported = false; while ( true ) { :]
    try/recover(#statements=topStatement)*
[: if (yytoken != Token_RBRACE && yytoken != Token_EOF && yytoken != Token_CLOSE_TAG
       && yytoken != Token_ELSEIF && yytoken != Token_ELSE
       && yytoken != Token_ENDIF && yytoken != Token_ENDFOREACH && yytoken != Token_ENDFOR
       && yytoken != Token_ENDWHILE && yytoken != Token_ENDSWITCH && yytoken != Token_ENDDECLARE
       && yytoken != Token_CASE && yytoken != Token_DEFAULT) {
        if (!reported) {
            qint64 index = tokenStream->index() - 1;
            Token &token = tokenStream->at(index);
            QString tokenValue = token.kind != 0 ? tokenText(token.begin, token.end) : QStringLiteral("EOF");
            reportProblem(Error, QStringLiteral("Unexpected token \"%1\".").arg(tokenValue));
            reported = true;
        }
        yylex();
   } else {
        break;
   }
} :]
-> innerStatementList ;;

--Operator Precedence, from PHP Manual
--left    or
--left    xor
--left    and
--right   print
--right   = += -= *= /= .= %= &= |= ^= <<= >>=    assignment
--left    ? : ternary
--left    ||  logical
--left    &&  logical
--left    |   bitwise
--left    ^   bitwise
--left    &   bitwise and references
--non-associative == != === !==   comparison
--non-associative < <= > >=   comparison
--left    << >>   bitwise
--left    + - .   arithmetic and string
--left    * / %   arithmetic
--non-associative ! ~ - (int) (float) (string) (array) (object) @ types
--non-associative ++ --   increment/decrement
--left    [   array()
--non-associative new new

  expression=logicalOrExpression
-> expr ;;

   #expression=logicalXorExpression @ LOGICAL_OR
-> logicalOrExpression ;;

   #expression=logicalAndExpression @ LOGICAL_XOR
-> logicalXorExpression ;;

   #expression=printExpression @ LOGICAL_AND
-> logicalAndExpression ;;

  (print=PRINT*) expression=assignmentExpression
-> printExpression ;;

-- leftside must me a variable, we check afterwards if it was a variable and
-- if not we report an error
0 --needed for line below
[: m_state.varExpressionIsVariable = false; :] --reset flag
expression=conditionalExpression
(
  assignmentExpressionEqual=assignmentExpressionEqual | (
     (
        PLUS_ASSIGN   [: (*yynode)->operation = OperationPlus; :]
      | MINUS_ASSIGN  [: (*yynode)->operation = OperationMinus; :]
      | MUL_ASSIGN    [: (*yynode)->operation = OperationMul; :]
      | EXP_ASSIGN    [: (*yynode)->operation = OperationExp; :]
      | DIV_ASSIGN    [: (*yynode)->operation = OperationDiv; :]
      | CONCAT_ASSIGN [: (*yynode)->operation = OperationConcat; :]
      | MOD_ASSIGN    [: (*yynode)->operation = OperationMod; :]
      | AND_ASSIGN    [: (*yynode)->operation = OperationAnd; :]
      | OR_ASSIGN     [: (*yynode)->operation = OperationOr; :]
      | XOR_ASSIGN    [: (*yynode)->operation = OperationXor; :]
      | SL_ASSIGN     [: (*yynode)->operation = OperationSl; :]
      | SR_ASSIGN     [: (*yynode)->operation = OperationSr; :]
     )
     assignmentExpressionCheckIfVariable
     assignmentExpression=assignmentExpression)
   | 0)
-> assignmentExpression [
     member variable operation: OperationType;
];;

--=& is special:
  -- $foo =& $var; is allowed but not $foo =& 'static';
  -- $foo =& new bar(); is allowed too but deprecated and reports a warning
  --we set a flag (varExpressionState) with that var_expression accepts only valid parts
  --this is done in such a strage way because we need the full expression to allow
  --things like $foo =& $bar || e();
ASSIGN
    assignmentExpressionCheckIfVariable --as in assignmentExpression
    (BIT_AND [: if (yytoken == Token_NEW) {
                reportProblem(Warning, QStringLiteral("=& new foo() is deprecated"), -2);
                m_state.varExpressionState = OnlyNewObject;
              } else {
                m_state.varExpressionState = OnlyVariable;
              }:]
     | 0) assignmentExpression=assignmentExpression [: m_state.varExpressionState = Normal; :]
-> assignmentExpressionEqual ;;


-- check if var_expression was a variable, if not report an error
-- varExpressionIsVariable is set in var_expression
0 --to allow cpp-code
[:
    if (!m_state.varExpressionIsVariable) {
        reportProblem(Error, QStringLiteral("Left side is not a variable"));
        return false;
    }
:]
-> assignmentExpressionCheckIfVariable ;;

expression=booleanOrExpression
   (  QUESTION (ifExpression=expr|0)
      COLON    elseExpression=conditionalExpression
    | 0
   )
-> conditionalExpression ;;

  #expression=booleanAndExpression @ BOOLEAN_OR
-> booleanOrExpression ;;

   #expression=bitOrExpression @ BOOLEAN_AND
-> booleanAndExpression ;;

   #expression=bitXorExpression @ BIT_OR
-> bitOrExpression ;;

   #expression=bitAndExpression @ BIT_XOR
-> bitXorExpression ;;

   #expression=equalityExpression @ BIT_AND
-> bitAndExpression ;;

   expression=relationalExpression
   (#additionalExpression=equalityExpressionRest)*
-> equalityExpression ;;

   (  IS_EQUAL | IS_NOT_EQUAL | IS_IDENTICAL | IS_NOT_IDENTICAL )
   expression=relationalExpression
-> equalityExpressionRest ;;


   expression=shiftExpression
   (  (#additionalExpression=relationalExpressionRest)+
      --instanceof as in java.g (correct??)
    | INSTANCEOF instanceofType=classNameReference
    | 0
   )
-> relationalExpression ;;

   ( IS_SMALLER | IS_GREATER | IS_SMALLER_OR_EQUAL | IS_GREATER_OR_EQUAL )
   expression=shiftExpression
-> relationalExpressionRest ;;


   expression=additiveExpression
   (#additionalExpression=shiftExpressionRest)*
-> shiftExpression ;;

   ( SL | SR )
   expression=additiveExpression
-> shiftExpressionRest ;;


   expression=multiplicativeExpression
   (#additionalExpression=additiveExpressionRest)*
-> additiveExpression ;;

   (
       PLUS   [: (*yynode)->operation = OperationPlus; :]
     | MINUS  [: (*yynode)->operation = OperationMinus; :]
     | CONCAT [: (*yynode)->operation = OperationConcat; :]
   )
   expression=multiplicativeExpression
-> additiveExpressionRest [
     member variable operation: OperationType;
];;


   expression=unaryExpression
   (#additionalExpression=multiplicativeExpressionRest)*
-> multiplicativeExpression ;;

   (
       MUL [: (*yynode)->operation = OperationMul; :]
     | DIV [: (*yynode)->operation = OperationDiv; :]
     | EXP [: (*yynode)->operation = OperationExp; :]
     | MOD [: (*yynode)->operation = OperationMod; :]
   )
   expression=unaryExpression
-> multiplicativeExpressionRest [
     member variable operation: OperationType;
];;

 (
    MINUS unaryExpression=unaryExpression
  | PLUS  unaryExpression=unaryExpression
  | BANG unaryExpression=unaryExpression
  | TILDE unaryExpression=unaryExpression
  | INT_CAST unaryExpression=unaryExpression    [: (*yynode)->castType = CastInt; :]
  | DOUBLE_CAST unaryExpression=unaryExpression [: (*yynode)->castType = CastDouble; :]
  | STRING_CAST unaryExpression=unaryExpression [: (*yynode)->castType = CastString; :]
  | ARRAY_CAST unaryExpression=unaryExpression  [: (*yynode)->castType = CastArray; :]
  | OBJECT_CAST unaryExpression=unaryExpression [: (*yynode)->castType = CastObject; :]
  | BOOL_CAST unaryExpression=unaryExpression   [: (*yynode)->castType = CastBool; :]
  | UNSET_CAST unaryExpression=unaryExpression  [: (*yynode)->castType = CastUnset; :]
  | AT unaryExpression=unaryExpression
  | LIST LPAREN assignmentList=assignmentList RPAREN ASSIGN unaryExpression=unaryExpression
  | EXIT (LPAREN (expression=expr | 0) RPAREN | 0)
  | EVAL LPAREN expression=expr RPAREN
  | INCLUDE includeExpression=unaryExpression
  | INCLUDE_ONCE includeExpression=unaryExpression
  | REQUIRE includeExpression=unaryExpression
  | REQUIRE_ONCE includeExpression=unaryExpression

  | unaryExpressionNotPlusminus=unaryExpressionNotPlusminus
 )
-> unaryExpression [
     member variable castType: CastType;
];;

    (#prefixOperator=postprefixOperator)*
    varExpression=varExpression
    (#postfixOperator=postprefixOperator)*
-> unaryExpressionNotPlusminus ;;

   op=INC | op=DEC
-> postprefixOperator ;;

--first/first conflict - no problem because of ifs
    ?[: m_state.varExpressionState == OnlyVariable :] 0 [: m_state.varExpressionState = Normal; :] variable=variable
  | ?[: m_state.varExpressionState == OnlyNewObject :] 0 [: m_state.varExpressionState = Normal; :] newObject=varExpressionNewObject
  | varExpressionNormal=varExpressionNormal
  | varExpressionArray=varExpressionArray arrayIndex=arrayIndexSpecifier*
-> varExpression ;;

    (?[: LA(1).kind == Token_LPAREN && LA(2).kind == Token_FUNCTION && LA(3).kind == Token_LPAREN :] iife=iifeSyntax )
  | LPAREN try/rollback (newObject=varExpressionNewObject RPAREN (#variableProperties=instantiationAccess*))
    catch (expression=expr RPAREN)
  | BACKTICK encapsList=encapsList BACKTICK
  --try/rollback resolves conflict scalar vs. staticMember (foo::bar vs. foo::$bar)
  --varExpressionIsVariable flag is needed for assignmentExpression
  | try/rollback (variable=variable [: m_state.varExpressionIsVariable = true; :])
    catch (scalar=scalar)
  | ISSET LPAREN (#issetVariable=variable @ COMMA) RPAREN
  | EMPTY LPAREN emptyVarialbe=variable RPAREN
  | newObject=varExpressionNewObject
  | CLONE cloneCar=varExpressionNormal
  | closure=closure
-> varExpressionNormal ;;

    ARRAY LPAREN
        (#arrayValues=arrayPairValue
             -- break because array(1,) is allowed (solves FIRST/FOLLOW conflict)
          @ (COMMA [: if (yytoken == Token_RPAREN) { break; } :] ) | 0)
    RPAREN
  | LBRACKET
        (#arrayValues=arrayPairValue
             -- break because [1,] is allowed (solves FIRST/FOLLOW conflict)
          @ (COMMA [: if (yytoken == Token_RBRACKET) { break; } :] ) | 0)
    RBRACKET
-> varExpressionArray ;;

-- http://wiki.php.net/rfc/closures
    FUNCTION (isRef=BIT_AND|0) LPAREN parameters=parameterList RPAREN
        ( USE LPAREN lexicalVars=lexicalVarList RPAREN | 0)
        LBRACE try/recover(functionBody=innerStatementList) RBRACE
-> closure ;;

    LPAREN try/rollback (closure=closure RPAREN LPAREN parameterList=functionCallParameterList RPAREN)
    catch (expression=expr RPAREN)
-> iifeSyntax ;;

  (#lexicalVars=lexicalVar @ COMMA) | 0 [: reportProblem(Error, QStringLiteral("Use list of closure must not be empty.")); :]
-> lexicalVarList ;;

  (isRef=BIT_AND | 0) variable=variableIdentifier
-> lexicalVar ;;

    NEW className=classNameReference ctor=ctorArguments
-> varExpressionNewObject ;;

    LPAREN parameterList=functionCallParameterList RPAREN
  | 0
-> ctorArguments ;;

    #parameters=functionCallParameterListElement @ COMMA | 0
-> functionCallParameterList ;;

    (BIT_AND variable=variable) | expr=expr
-> functionCallParameterListElement ;;

    #element=assignmentListElement @COMMA
-> assignmentList ;;

    variable=variable
  | LIST LPAREN assignmentList=assignmentList RPAREN
  | 0
-> assignmentListElement ;;

    expr=expr (DOUBLE_ARROW (exprValue=expr | BIT_AND varValue=variable) | 0)
  | BIT_AND variable=variable
-> arrayPairValue ;;

   var=baseVariableWithFunctionCalls (#variableProperties=variableObjectProperty*)
-> variable ;;

    OBJECT_OPERATOR
  | PAAMAYIM_NEKUDOTAYIM
-> objectOperator ;;

    ( ?[: LA(1).kind == Token_DOLLAR:] LBRACE variable=variable RBRACE | objectProperty=objectProperty )
    (isFunctionCall=LPAREN parameterList=functionCallParameterList RPAREN arrayIndex=arrayIndexSpecifier* | 0)
-> variableProperty ;;

   objectOperator variableProperty=variableProperty
-> variableObjectProperty ;;

   OBJECT_OPERATOR variableProperty=variableProperty
-> instantiationAccess ;;

   --Conflict
   --   foo::$bar[0] (=baseVariable-staticMember)
   --vs.foo::$bar[0](); (=static function call)
   try/rollback (functionCall=functionCall arrayIndex=arrayIndexSpecifier*)
   catch (baseVariable=baseVariable)
-> baseVariableWithFunctionCalls ;;

   LBRACKET (expr=expr | 0) RBRACKET
-> arrayIndexSpecifier ;;

   LBRACKET (expr=expr) RBRACKET
-> stringIndexSpecifier ;;

    stringFunctionNameOrClass=namespacedIdentifier (
        LPAREN stringParameterList=functionCallParameterList RPAREN
      | PAAMAYIM_NEKUDOTAYIM
        (
            stringFunctionName=identifier LPAREN stringParameterList=functionCallParameterList RPAREN
            | varFunctionName=variableWithoutObjects LPAREN stringParameterList=functionCallParameterList RPAREN
            | LBRACE (expr=expr) RBRACE LPAREN stringParameterList=functionCallParameterList RPAREN
        )
    )
  | varFunctionName=variableWithoutObjects LPAREN varParameterList=functionCallParameterList RPAREN
-> functionCall ;;

    var=compoundVariableWithSimpleIndirectReference #offsetItems=dimListItem*
  | staticMember=staticMember
-> baseVariable ;;

    variable=variableIdentifier
  | DOLLAR LBRACE expr=expr RBRACE
-> compoundVariable ;;

  ( DOLLAR ( DOLLAR+ | 0 ) ( indirectVariable=variableIdentifier | LBRACE expr=expr RBRACE ) | variable=variableIdentifier )
-> compoundVariableWithSimpleIndirectReference ;;

    className=namespacedIdentifier PAAMAYIM_NEKUDOTAYIM variable=variableWithoutObjects
-> staticMember ;;

    LBRACE try/recover(statements=innerStatementList) RBRACE
  | IF LPAREN ifExpr=expr RPAREN
      (   COLON statements=innerStatementList newElseifList newElseSingle ENDIF semicolonOrCloseTag
        | ifStatement=statement elseifList=elseifList elseSingle=elseSingle
      )
  | WHILE LPAREN whileExpr=expr RPAREN whileStatement=whileStatement
  | FOR LPAREN forExpr1=forExpr SEMICOLON forExpr2=forExpr
    SEMICOLON forExpr3=forExpr RPAREN forStatement=forStatement
  | SWITCH LPAREN swtichExpr=expr RPAREN switchCaseList=switchCaseList

  | FOREACH LPAREN (
            -- allow $var as &$i and not expr() as &$i
        try/rollback(foreachVar=variable AS foreachVarAsVar=foreachVariable)
        catch(foreachExpr=expr AS foreachExprAsVar=variable))
        (DOUBLE_ARROW foreachVariable=foreachVariable | 0) RPAREN
        foreachStatement=foreachStatement
  | DECLARE LPAREN declareItem=declareItem @ COMMA RPAREN declareStatement
  | SEMICOLON     -- empty statement
  | TRY  LBRACE try/recover(statements=innerStatementList) RBRACE
    #catches=catchItem*
    (FINALLY LBRACE finallyBody=innerStatementList RBRACE | 0)
  | UNSET LPAREN #unsetVariables=variable @ COMMA RPAREN semicolonOrCloseTag
  -- fix first/follow with goto target
  | ( ?[: LA(1).kind != Token_STRING || LA(2).kind != Token_COLON :] expr=expr semicolonOrCloseTag )
  | DO doStatement=statement WHILE LPAREN whileExpr=expr RPAREN semicolonOrCloseTag
  | BREAK (breakExpr=expr | 0) semicolonOrCloseTag
  | CONTINUE (continueExpr=expr | 0) semicolonOrCloseTag
  | RETURN (returnExpr=expr | 0) semicolonOrCloseTag
  | GLOBAL #globalVars=globalVar @ COMMA semicolonOrCloseTag
  | STATIC #staticVars=staticVar @ COMMA semicolonOrCloseTag
  | ECHO #echoExprs=expr @ COMMA semicolonOrCloseTag
  | THROW throwExpr=expr semicolonOrCloseTag
  -- throws error in zend parser, so ignored | USE use_filename  semicolonOrCloseTag

  | CLOSE_TAG
  | OPEN_TAG
  | OPEN_TAG_WITH_ECHO expr=expr semicolonOrCloseTag
  | INLINE_HTML
  | CONST #consts=constantDeclaration @ COMMA SEMICOLON
  | USE #useNamespace=useNamespace @ COMMA SEMICOLON
  | GOTO gotoLabel=STRING SEMICOLON
  | gotoTarget=STRING COLON
-> statement ;;

    identifier=namespacedIdentifier (AS aliasIdentifier=identifier | 0)
-> useNamespace ;;

    identifier=identifier ASSIGN scalar=staticScalar
-> constantDeclaration ;;

   SEMICOLON | CLOSE_TAG
-> semicolonOrCloseTag ;;

    LBRACE (SEMICOLON | 0) try/recover(caseList=caseList) RBRACE
  | COLON (SEMICOLON | 0) caseList=caseList ENDSWITCH semicolonOrCloseTag
-> switchCaseList ;;

    #caseItems=case_item*
-> caseList ;;

    CASE expr=expr (COLON | SEMICOLON) statements=innerStatementList
  | def=DEFAULT (COLON | SEMICOLON) statements=innerStatementList
-> case_item ;;

    CATCH LPAREN catchClass=namespacedIdentifier var=variableIdentifier RPAREN
    LBRACE try/recover(statements=innerStatementList) RBRACE
-> catchItem ;;

    statement=statement
  | COLON statements=innerStatementList ENDDECLARE semicolonOrCloseTag
-> declareStatement ;;

    STRING ASSIGN scalar=staticScalar
-> declareItem ;;

    (BIT_AND | 0) variable=variable
-> foreachVariable ;;

    statement=statement
  | COLON statements=innerStatementList ENDFOREACH semicolonOrCloseTag
-> foreachStatement ;;

  var=variableIdentifier (ASSIGN value=staticScalar | 0)
-> staticVar ;;

    var=variableIdentifier
  | DOLLAR (dollarVar=variable | LBRACE expr=expr RBRACE)
-> globalVar ;;

    #exprs=expr @ COMMA | 0
-> forExpr ;;

    statement=statement
  | COLON statements=innerStatementList ENDFOR semicolonOrCloseTag
-> forStatement ;;

    statement=statement
  | COLON statements=innerStatementList ENDWHILE semicolonOrCloseTag
-> whileStatement ;;

    --first/follow conflict; todo check if this is a problem
    #elseifListItem=elseifListItem*
-> elseifList ;;

    ELSEIF LPAREN expr=expr RPAREN statement=statement
-> elseifListItem ;;

    ELSE statement=statement | 0
-> elseSingle ;;

    #newElseifListItem=newelseifListItem*
-> newElseifList ;;

    ELSEIF LPAREN expr=expr RPAREN COLON statements=innerStatementList
-> newelseifListItem ;;

    ELSE COLON statements=innerStatementList | 0
-> newElseSingle ;;

--TODO     --resolve STRING vs. staticMember conflict
--     ?[: LA(2).kind != Token_PAAMAYIM_NEKUDOTAYIM :]
    identifier=namespacedIdentifier
  | staticIdentifier = STATIC
  | dynamicClassNameReference=dynamicClassNameReference
-> classNameReference ;;

    baseVariable=baseVariable (OBJECT_OPERATOR objectProperty=objectProperty
                          properties=dynamicClassNameVariableProperties | 0)
-> dynamicClassNameReference ;;

    #properties=dynamicClassNameVariableProperty*
-> dynamicClassNameVariableProperties ;;

    OBJECT_OPERATOR property=objectProperty
-> dynamicClassNameVariableProperty ;;

    objectDimList=objectDimList
  | variableWithoutObjects=variableWithoutObjects
-> objectProperty ;;

    variableName=variableName #offsetItems=dimListItem*
-> objectDimList ;;

  variable=compoundVariableWithSimpleIndirectReference #offsetItems=dimListItem*
-> variableWithoutObjects ;;

arrayIndex=arrayIndexSpecifier | LBRACE expr=expr RBRACE
-> dimListItem ;;

    name=identifier
  | LBRACE expr=expr RBRACE
-> variableName ;;

    commonScalar=commonScalar
  | constantOrClassConst=constantOrClassConst
  | varname=STRING_VARNAME
  | DOUBLE_QUOTE encapsList=encapsList DOUBLE_QUOTE stringIndex=stringIndexSpecifier*
  | START_HEREDOC encapsList=encapsList END_HEREDOC
-> scalar ;;

  constant=namespacedIdentifier
  ( PAAMAYIM_NEKUDOTAYIM classConstant=classConstant | 0 )
-> constantOrClassConst ;;

    CLASS
  | identifier
-> classConstant ;;

    #encaps=encaps*
-> encapsList ;;

    var=encapsVar | value=ENCAPSED_AND_WHITESPACE
-> encaps ;;

     -- first/first conflict resolved by LA(2)
     --(expr allows STRING_VARNAME too - but without [expr])
    DOLLAR_OPEN_CURLY_BRACES ( ?[: LA(2).kind == Token_LBRACKET:] STRING_VARNAME arrayIndex=arrayIndexSpecifier RBRACE
      | expr=expr RBRACE )
  | variable=variableIdentifier (OBJECT_OPERATOR propertyIdentifier=identifier | LBRACKET offset=encapsVarOffset RBRACKET | 0)
  | CURLY_OPEN expr=expr RBRACE
-> encapsVar ;;

    STRING
  | NUM_STRING
  | variableIdentifier
-> encapsVarOffset ;;


    LNUMBER                  [: (*yynode)->scalarType = ScalarTypeInt; :]
  | DNUMBER                  [: (*yynode)->scalarType = ScalarTypeFloat; :]
  | string=CONSTANT_ENCAPSED_STRING [: (*yynode)->scalarType = ScalarTypeString; :] stringIndex=stringIndexSpecifier*
  | LINE                     [: (*yynode)->scalarType = ScalarTypeInt; :]
  | FILE                     [: (*yynode)->scalarType = ScalarTypeString; :]
  | CLASS_C                  [: (*yynode)->scalarType = ScalarTypeString; :]
  | METHOD_C                 [: (*yynode)->scalarType = ScalarTypeString; :]
  | FUNC_C                   [: (*yynode)->scalarType = ScalarTypeString; :]
  | NAMESPACE_C              [: (*yynode)->scalarType = ScalarTypeString; :]
  | START_NOWDOC STRING END_NOWDOC [: (*yynode)->scalarType = ScalarTypeString; :]
-> commonScalar [
     member variable scalarType: ScalarTypes;
] ;;

    FUNCTION (BIT_AND | 0) functionName=identifier
    LPAREN parameters=parameterList RPAREN LBRACE try/recover(functionBody=innerStatementList) RBRACE
-> functionDeclarationStatement ;;

    (#parameters=parameter @ COMMA) | 0
-> parameterList ;;

(parameterType=namespacedIdentifier | arrayType=ARRAY | callableType=CALLABLE | 0) (isRef=BIT_AND | 0)
    variable=variableIdentifier (ASSIGN defaultValue=staticScalar | 0)
-> parameter ;;

    value=commonScalar
  | constantOrClassConst=constantOrClassConst
  | PLUS plusValue=staticScalar
  | MINUS minusValue=staticScalar
  | array=ARRAY LPAREN
        (#arrayValues=staticArrayPairValue
             -- break because array(1,) is allowed
          @ (COMMA [: if (yytoken == Token_RPAREN) { break; } :] ) | 0)
    RPAREN
  | array=LBRACKET
        (#arrayValues=staticArrayPairValue
             -- break because [1,] is allowed
          @ (COMMA [: if (yytoken == Token_RBRACKET) { break; } :] ) | 0)
    RBRACKET
-> staticScalar ;;

    #val1=staticScalar (DOUBLE_ARROW #val2=staticScalar | 0)
-> staticArrayPairValue ;;

   (isGlobal=BACKSLASH | 0)
   #namespaceName=identifier+ @ BACKSLASH
-> namespacedIdentifier ;;

     string=STRING
-> identifier ;;

     variable=VARIABLE
-> variableIdentifier ;;

    NAMESPACE #namespaceName=identifier* @ BACKSLASH
    (
        -- the semicolon case needs at least one namespace identifier, the {...} case not...
        SEMICOLON [: if (!(*yynode)->namespaceNameSequence) { reportProblem(Error, QStringLiteral("Missing namespace identifier."), -2); } :]
    | LBRACE try/recover(body=innerStatementList) RBRACE )
-> namespaceDeclarationStatement ;;

    INTERFACE interfaceName=identifier (EXTENDS extends=classImplements | 0)
    LBRACE try/recover(body=classBody) RBRACE
-> interfaceDeclarationStatement ;;

    TRAIT traitName=identifier
    LBRACE body=classBody RBRACE
-> traitDeclarationStatement ;;

    modifier=optionalClassModifier CLASS className=identifier
        (EXTENDS extends=classExtends | 0)
        (IMPLEMENTS implements=classImplements | 0)
    LBRACE body=classBody RBRACE
-> classDeclarationStatement ;;

identifier=namespacedIdentifier
-> classExtends ;;

#implements=namespacedIdentifier @ COMMA
-> classImplements ;;

-- error recovery, to understand it you probably have to look at the generated code ;-)
[: bool reported = false; while ( true ) { :]
try/recover(#classStatements=classStatement)*
[: if (yytoken != Token_RBRACE && yytoken != Token_EOF && yytoken != Token_CLOSE_TAG) {
        if (!reported) {
            reportProblem(Error, QStringLiteral("Unexpected token in class context."));
            reported = true;
        }
        yylex();
   } else {
        break;
   }
} :]
 RBRACE [: rewind(tokenStream->index() - 2); :]
-> classBody ;;

    CONST #consts=constantDeclaration @ COMMA SEMICOLON
  | VAR variable=classVariableDeclaration SEMICOLON
  | modifiers=optionalModifiers
    ( variable=classVariableDeclaration SEMICOLON
      | FUNCTION (BIT_AND | 0) methodName=identifier LPAREN parameters=parameterList RPAREN
        methodBody=methodBody
    )
  | USE #traits=namespacedIdentifier @ COMMA (imports=traitAliasDeclaration|SEMICOLON)
-> classStatement ;;

    LBRACE #statements=traitAliasStatement
        @ (SEMICOLON [: if (yytoken == Token_RBRACE) { break; } :]) RBRACE
-> traitAliasDeclaration ;;

    importIdentifier=traitAliasIdentifier (AS (modifiers=optionalModifiers | 0) aliasIdentifier=identifier|INSTEADOF #conflictIdentifier=namespacedIdentifier @ COMMA)
-> traitAliasStatement ;;

    identifier=namespacedIdentifier PAAMAYIM_NEKUDOTAYIM methodIdentifier=identifier
-> traitAliasIdentifier ;;

    SEMICOLON -- abstract method
 |  LBRACE try/recover(statements=innerStatementList) RBRACE
-> methodBody ;;

#vars=classVariable @ COMMA
-> classVariableDeclaration ;;

    variable=variableIdentifier (ASSIGN value=staticScalar | 0)
-> classVariable ;;

  (
    PUBLIC     [: (*yynode)->modifiers |= ModifierPublic;      :]
  | PROTECTED  [: (*yynode)->modifiers |= ModifierProtected;      :]
  | PRIVATE    [: (*yynode)->modifiers |= ModifierPrivate;      :]
  | STATIC     [: (*yynode)->modifiers |= ModifierStatic;      :]
  | ABSTRACT   [: (*yynode)->modifiers |= ModifierAbstract;      :]
  | FINAL      [: (*yynode)->modifiers |= ModifierFinal;      :]
  | 0
  )*
-> optionalModifiers[
     member variable modifiers: unsigned int;
] ;;

  (
    ABSTRACT   [: (*yynode)->modifier = AbstractClass; :]
  | FINAL      [: (*yynode)->modifier = FinalClass;    :]
  | 0
  )
-> optionalClassModifier[
     member variable modifier: ClassModifier;
] ;;





-----------------------------------------------------------------
-- Code segments copied to the implementation (.cpp) file.
-- If existent, kdevelop-pg's current syntax requires this block
-- to occur at the end of the file.
-----------------------------------------------------------------

[:

#include <QtCore/QDebug>
#include <KTextEditor/Range>

namespace Php
{

void Parser::tokenize(const QString& contents, int initialState)
{
    m_contents = contents;
    Lexer lexer(tokenStream, contents, initialState);
    int kind = Parser::Token_EOF;
    int lastDocCommentBegin;
    int lastDocCommentEnd;

    do
    {
        lastDocCommentBegin = 0;
        lastDocCommentEnd = 0;
        kind = lexer.nextTokenKind();
        while (kind == Parser::Token_WHITESPACE || kind == Parser::Token_COMMENT || kind == Parser::Token_DOC_COMMENT) {
            if (kind == Parser::Token_COMMENT || kind == Parser::Token_DOC_COMMENT) {
                extractTodosFromComment(tokenText(lexer.tokenBegin(), lexer.tokenEnd()), lexer.tokenBegin());
            }
            if (kind == Parser::Token_DOC_COMMENT) {
                lastDocCommentBegin = lexer.tokenBegin();
                lastDocCommentEnd = lexer.tokenEnd();
            }
            kind = lexer.nextTokenKind();
        }
        if ( !kind ) // when the lexer returns 0, the end of file is reached
        {
            kind = Parser::Token_EOF;
        }
        Parser::Token &t = tokenStream->push();
        t.begin = lexer.tokenBegin();
        t.end = lexer.tokenEnd();
        t.kind = kind;
        t.docCommentBegin = lastDocCommentBegin;
        t.docCommentEnd = lastDocCommentEnd;
        //if ( m_debug ) qDebug() << kind << tokenText(t.begin,t.end) << t.begin << t.end;
    }
    while ( kind != Parser::Token_EOF );

    yylex(); // produce the look ahead token
}

void Parser::extractTodosFromComment(const QString& comment, qint64 startPosition)
{
    auto i = m_todoMarkers.globalMatch(comment);
    while (i.hasNext()) {
        auto match = i.next();
        auto p = reportProblem(Todo, match.captured(1), 0);
        if (!p) {
            continue;
        }

        qint64 line = 0;
        qint64 column = 0;
        tokenStream->locationTable()->positionAt(startPosition, &line, &column);

        auto location = p->finalLocation();
        location.setStart(KTextEditor::Cursor(line, column + match.capturedStart(1)));
        location.setEnd(KTextEditor::Cursor(line, column + match.capturedEnd(1)));
        p->setFinalLocation(location);
    };
}

void Parser::setTodoMarkers(const QStringList& markers)
{
    QString pattern = QStringLiteral("^(?:[/\\*\\s]*)(.*(?:");
    bool first = true;
    foreach(const QString& marker, markers) {
        if (!first) {
            pattern += '|';
        }
        pattern += QRegularExpression::escape(marker);
        first = false;
    }
    pattern += QStringLiteral(").*?)(?:[/\\*\\s]*)$");
    m_todoMarkers.setPatternOptions(QRegularExpression::MultilineOption);
    m_todoMarkers.setPattern(pattern);
}

QString Parser::tokenText(qint64 begin, qint64 end)
{
    return m_contents.mid(begin,end-begin+1);
}


KDevelop::ProblemPointer Parser::reportProblem( Parser::ProblemType type, const QString& message, int offset )
{
    qint64 sLine;
    qint64 sCol;
    qint64 index = tokenStream->index() + offset;
    if (index >= tokenStream->size()) {
        return {};
    }
    tokenStream->startPosition(index, &sLine, &sCol);
    qint64 eLine;
    qint64 eCol;
    tokenStream->endPosition(index, &eLine, &eCol);
    auto p = KDevelop::ProblemPointer(new KDevelop::Problem());
    p->setSource(KDevelop::IProblem::Parser);
    switch ( type ) {
        case Error:
            p->setSeverity(KDevelop::IProblem::Error);
            break;
        case Warning:
            p->setSeverity(KDevelop::IProblem::Warning);
            break;
        case Info:
            p->setSeverity(KDevelop::IProblem::Hint);
            break;
        case Todo:
            p->setSeverity(KDevelop::IProblem::Hint);
            p->setSource(KDevelop::IProblem::ToDo);
            break;
    }
    p->setDescription(message);
    KTextEditor::Range range(sLine, sCol, eLine, eCol + 1);
    p->setFinalLocation(KDevelop::DocumentRange(m_currentDocument, range));
    m_problems << p;
    return p;
}


// custom error recovery
void Parser::expectedToken(int /*expected*/, qint64 /*where*/, const QString& name)
{
    reportProblem( Parser::Error, QStringLiteral("Expected token \"%1\"").arg(name));
}

void Parser::expectedSymbol(int /*expectedSymbol*/, const QString& name)
{
    qint64 line;
    qint64 col;
    qint64 index = tokenStream->index()-1;
    Token &token = tokenStream->at(index);
    qCDebug(PARSER) << "token starts at:" << token.begin;
    qCDebug(PARSER) << "index is:" << index;
    tokenStream->startPosition(index, &line, &col);
    QString tokenValue = tokenText(token.begin, token.end);
    qint64 eLine;
    qint64 eCol;
    tokenStream->endPosition(index, &eLine, &eCol);
    reportProblem( Parser::Error,
                   QStringLiteral("Expected symbol \"%1\" (current token: \"%2\" [%3] at %4:%5 - %6:%7)")
                  .arg(name,
                       token.kind != 0 ? tokenValue : QStringLiteral("EOF"))
                  .arg(token.kind)
                  .arg(line)
                  .arg(col)
                  .arg(eLine)
                  .arg(eCol));
}

void Parser::setDebug( bool debug )
{
    m_debug = debug;
}

void Parser::setCurrentDocument(KDevelop::IndexedString url)
{
    m_currentDocument = url;
}


Parser::ParserState *Parser::copyCurrentState()
{
    ParserState *state = new ParserState();
    state->varExpressionState = m_state.varExpressionState;
    state->varExpressionIsVariable = m_state.varExpressionIsVariable;
    return state;
}

void Parser::restoreState( Parser::ParserState* state)
{
    m_state.varExpressionState = state->varExpressionState;
    m_state.varExpressionIsVariable = state->varExpressionIsVariable;
}

} // end of namespace Php

:]

-- kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on; auto-insert-doxygen on; mode KDevelop-PG[-Qt]
