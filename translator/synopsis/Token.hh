//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_Token_hh_
#define Synopsis_Token_hh_

#include <cstring>

#include <string> // francis

namespace Synopsis
{

//. A Token is what the Lexer splits an input stream into.
//. It refers to a region in the underlaying buffer and it
//. has a type.
//.
//. - line directive:   `^"#"{blank}*{digit}+({blank}+.*)?\n`
//. - pragma directive: `^"#"{blank}*"pragma".*\n`
//. - Constant:
//.   
//.   - `{digit}+{int_suffix}*`
//.   - `"0"{xletter}{hexdigit}+{int_suffix}*`
//.   - `{digit}*\.{digit}+{float_suffix}*`
//.   - `{digit}+\.{float_suffix}*`
//.   - `{digit}*\.{digit}+"e"("+"|"-")*{digit}+{float_suffix}*`
//.   - `{digit}+\."e"("+"|"-")*{digit}+{float_suffix}*`
//.   - `{digit}+"e"("+"|"-")*{digit}+{float_suffix}*`
//. 
//. - CharConst:        `\'([^'\n]|\\[^\n])\'`
//. - WideCharConst:    `L\'([^'\n]|\\[^\n])\'`
//. - StringL:          `\"([^"\n]|\\["\n])*\"`
//. - WideStringL:      `L\"([^"\n]|\\["\n])*\"`
//. - Identifier:       `{letter}+({letter}|{digit})*`
//. - AssignOp:         `*= /= %= += -= &= ^= <<= >>=`
//. - EqualOp:          `== !=`
//. - RelOp:            `<= >=`
//. - ShiftOp:          `<< >>`
//. - LogOrOp:          `||`
//. - LogAndOp:         `&&`
//. - IncOp:            `++ --`
//. - Scope:            `::`
//. - Ellipsis:         `...`
//. - PmOp:             `.* ->*`
//. - ArrowOp:          `->`
//. - others:           `!%^&*()-+={}|~[];:<>?,./`
//. - BadToken:         `others`
//. 
struct Token 
{
  typedef int Type;
  enum
  {
    Identifier = 258, //.< The first 256 are representing character literals.
    Constant,
    CharConst,
    StringL,
    AssignOp,
    EqualOp,
    RelOp,
    ShiftOp,
    LogOrOp,
    LogAndOp,
    IncOp,
    Scope,
    Ellipsis,
    PmOp,
    ArrowOp,
    BadToken,
    AUTO,
    CHAR,
    CLASS,
    CONST,
    DELETE,
    DOUBLE,
    ENUM,
    EXTERN,
    FLOAT,
    FRIEND,
    INLINE,
    INT,
    LONG,
    NEW,
    OPERATOR,
    PRIVATE,
    PROTECTED,
    PUBLIC,
    REGISTER,
    SHORT,
    SIGNED,
    STATIC,
    STRUCT,
    TYPEDEF,
    TYPENAME,
    UNION,
    UNSIGNED,
    VIRTUAL,
    VOID,
    VOLATILE,
    TEMPLATE,
    MUTABLE,
    BREAK,
    CASE,
    CONTINUE,
    DEFAULT,
    DO,
    ELSE,
    FOR,
    GOTO,
    IF,
    OFFSETOF,
    RETURN,
    SIZEOF,
    SWITCH,
    THIS,
    WHILE,
    ATTRIBUTE,    //=g++,
    METACLASS,    //=opencxx
    UserKeyword,  // opencxx, class specifier, such as STRUCT | CLASS | UNION

/*      : UserKeyword '(' function.arguments ')' compound.statement
  | UserKeyword3 '(' expr.statement {expression} ';'
			{expression} ')' compound.statement
*/

    UserKeyword2, // opencxx, closure statement, followed by '(' arg.decl.list ')' compound.statement
    UserKeyword3, // opencxx, for ?
    UserKeyword4, // OK opencxx, access specifier, such as PRIVATE | PROTECTED | PUBLIC
    BOOLEAN,
    EXTENSION,    //=g++,
    TRY,
    CATCH,
    THROW,
    UserKeyword5,  // OK opencxx, member specifier such as FRIEND | INLINE | VIRTUAL
    NAMESPACE,
    USING,
    TYPEID,
    TYPEOF,
    WideStringL,
    WideCharConst,
    WCHAR,
    
    //francis
    firstUserKeyword,
    
    //=non terminals,

    ntDeclarator = 400,
    ntName,
    ntFstyleCast,
    ntClassSpec,
    ntEnumSpec,
    ntDeclaration,
    ntTypedef,
    ntTemplateDecl,
    ntMetaclassDecl,
    ntParameterDecl,
    ntLinkageSpec,
    ntAccessSpec,
    ntUserAccessSpec,
    ntUserdefKeyword,
    ntExternTemplate,
    ntAccessDecl,
    ntNamespaceSpec,
    ntUsing,
    ntTemplateInstantiation,
    ntNamespaceAlias,
    ntIfStatement,
    ntSwitchStatement,
    ntWhileStatement,
    ntDoStatement,
    ntForStatement,
    ntBreakStatement,
    ntContinueStatement,
    ntReturnStatement,
    ntGotoStatement,
    ntCaseStatement,
    ntDefaultStatement,
    ntLabelStatement,
    ntExprStatement,
    ntTryStatement,

    ntCommaExpr,
    ntAssignExpr,
    ntCondExpr,
    ntInfixExpr,
    ntPmExpr,
    ntCastExpr,
    ntUnaryExpr,
    ntSizeofExpr,
    ntNewExpr,
    ntDeleteExpr,
    ntArrayExpr,
    ntFuncallExpr,
    ntPostfixExpr,
    ntUserStatementExpr,
    ntDotMemberExpr,
    ntArrowMemberExpr,
    ntParenExpr,
    ntStaticUserStatementExpr,
    ntThrowExpr,
    ntTypeidExpr,
    ntTypeofExpr,

    Ignore  = 500,
    ASM,
    DECLSPEC,
    PRAGMA,
    INT64,
    Comment // This could eventually be used to report all comments.
  };

  Token() : ptr(0), length(0), type(BadToken) {}
  Token(const char *s, size_t l, Type t) : ptr(s), length(l), type(t) {}
  bool operator == (char c) const { return *ptr == c && length == 1;}
  
  // francis
  bool operator == (const char* c) const {return ptr && c && length > 0 && strncmp(ptr, c, length) == 0;}
  bool operator != (const char* c) const {return !(*this == c);}
  std::string toString() const {std::string res; for (size_t i = 0; i < length; ++i) res += ptr[i]; return res;}
  
  const char *ptr;
  size_t      length;
  Type        type;
};

}

#endif
