//
// Copyright (C) 2005 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_PTree_NodesFwd_hh_
#define Synopsis_PTree_NodesFwd_hh_

#include <synopsis/Token.hh>

namespace Synopsis
{
namespace PTree
{

class Node;
class Atom;
class List;
class Literal;
class CommentedAtom;
class DupAtom;
class Identifier;
class Keyword;
template <Token::Type> class KeywordT;

namespace Kwd
{
typedef KeywordT<Token::AUTO> Auto;
typedef KeywordT<Token::BOOLEAN> Bool;
typedef KeywordT<Token::BREAK> Break;
typedef KeywordT<Token::CASE> Case;
typedef KeywordT<Token::CATCH> Catch;
typedef KeywordT<Token::CHAR> Char;
typedef KeywordT<Token::CLASS> Class;
typedef KeywordT<Token::CONTINUE> Continue;
typedef KeywordT<Token::CONST> Const;
typedef KeywordT<Token::DEFAULT> Default;
typedef KeywordT<Token::DELETE> Delete;
typedef KeywordT<Token::DOUBLE> Double;
typedef KeywordT<Token::DO> Do;
typedef KeywordT<Token::ELSE> Else;
typedef KeywordT<Token::ENUM> Enum;
typedef KeywordT<Token::EXTERN> Extern;
typedef KeywordT<Token::FLOAT> Float;
typedef KeywordT<Token::FOR> For;
typedef KeywordT<Token::FRIEND> Friend;
typedef KeywordT<Token::GOTO> Goto;
typedef KeywordT<Token::INLINE> Inline;
typedef KeywordT<Token::IF> If;
typedef KeywordT<Token::INT> Int;
typedef KeywordT<Token::LONG> Long;
typedef KeywordT<Token::MUTABLE> Mutable;
typedef KeywordT<Token::NAMESPACE> Namespace;
typedef KeywordT<Token::NEW> New;
typedef KeywordT<Token::OPERATOR> Operator;
typedef KeywordT<Token::PRIVATE> Private;
typedef KeywordT<Token::PROTECTED> Protected;
typedef KeywordT<Token::PUBLIC> Public;
typedef KeywordT<Token::REGISTER> Register;
typedef KeywordT<Token::RETURN> Return;
typedef KeywordT<Token::SHORT> Short;
typedef KeywordT<Token::SIGNED> Signed;
typedef KeywordT<Token::STATIC> Static;
typedef KeywordT<Token::STRUCT> Struct;
typedef KeywordT<Token::SWITCH> Switch;
typedef KeywordT<Token::TEMPLATE> Template;
typedef KeywordT<Token::THIS> This;
typedef KeywordT<Token::THROW> Throw;
typedef KeywordT<Token::TRY> Try;
typedef KeywordT<Token::TYPEDEF> Typedef;
typedef KeywordT<Token::TYPENAME> Typename;
typedef KeywordT<Token::TYPEOF> Typeof;
typedef KeywordT<Token::UNION> Union;
typedef KeywordT<Token::UNSIGNED> Unsigned;
typedef KeywordT<Token::USING> Using;
typedef KeywordT<Token::VIRTUAL> Virtual;
typedef KeywordT<Token::VOID> Void;
typedef KeywordT<Token::VOLATILE> Volatile;
typedef KeywordT<Token::WCHAR> WChar;
typedef KeywordT<Token::WHILE> While;
}

class Brace;
class Block;
class ClassBody;
class Typedef;
class TemplateDecl;
class TemplateInstantiation;
class ExternTemplate;
class MetaclassDecl;
class LinkageSpec;
class NamespaceSpec;
class NamespaceAlias;
class UsingDirective;
class Declaration;
class FunctionDefinition;
class ParameterDeclaration;
class UsingDeclaration;
class Declarator;
class Name;
class FstyleCastExpr;
class ClassSpec;
class EnumSpec;
class TypeParameter;
class AccessSpec;
class AccessDecl;
class UserAccessSpec;
class IfStatement;
class SwitchStatement;
class WhileStatement;
class DoStatement;
class ForStatement;
class TryStatement;
class BreakStatement;
class ContinueStatement;
class ReturnStatement;
class GotoStatement;
class CaseStatement;
class DefaultStatement;
class LabelStatement;
class ExprStatement;
class UserStatement; // francis
class Expression;
class AssignExpr;
class CondExpr;
class InfixExpr;
class PmExpr;
class CastExpr;
class UnaryExpr;
class ThrowExpr;
class SizeofExpr;
class OffsetofExpr;
class TypeidExpr;
class TypeofExpr;
class NewExpr;
class DeleteExpr;
class ArrayExpr;
class FuncallExpr;
class PostfixExpr;
class DotMemberExpr;
class ArrowMemberExpr;
class ParenExpr;

// francis
class UserStatementExpr;

}
}

#endif
