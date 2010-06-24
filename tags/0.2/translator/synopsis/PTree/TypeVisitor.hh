//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_PTree_TypeVisitor_hh_
#define Synopsis_PTree_TypeVisitor_hh_

#include <synopsis/PTree.hh>
#include <synopsis/Token.hh>
#include <cassert>

namespace Synopsis
{
namespace PTree
{

class TypeVisitor : public Visitor
{
public:
  TypeVisitor() : my_type(Token::BadToken) {}

  Token::Type type_of(Node *node) { node->accept(this); return my_type;}

  virtual void visit(Literal *) { my_type = Token::Constant;}
  virtual void visit(Identifier *) { my_type = Token::Identifier;}
  virtual void visit(Keyword *kwd) { my_type = kwd->token();}
  virtual void visit(Typedef *) {my_type = Token::ntTypedef;}
  virtual void visit(TemplateDecl *) { my_type = Token::ntTemplateDecl;}
  virtual void visit(TemplateInstantiation *) { my_type = Token::ntTemplateInstantiation;}
  virtual void visit(ExternTemplate *) { my_type = Token::ntExternTemplate;}
  virtual void visit(MetaclassDecl *) { my_type = Token::ntMetaclassDecl;}
  virtual void visit(ParameterDeclaration *) { my_type = Token::ntParameterDecl;}
  virtual void visit(LinkageSpec *) { my_type = Token::ntLinkageSpec;}
  virtual void visit(NamespaceSpec *) { my_type = Token::ntNamespaceSpec;}
  virtual void visit(NamespaceAlias *) { my_type = Token::ntNamespaceAlias;}
  virtual void visit(UsingDirective *) { my_type = Token::ntUsing;}
  virtual void visit(Declaration *) { my_type = Token::ntDeclaration;}
  virtual void visit(UsingDeclaration *) { my_type = Token::ntUsing;}
  virtual void visit(Declarator *) { my_type = Token::ntDeclarator;}
  virtual void visit(Name *) { my_type = Token::ntName;}
  virtual void visit(FstyleCastExpr *) { my_type = Token::ntFstyleCast;}
  virtual void visit(ClassSpec *) { my_type = Token::ntClassSpec;}
  virtual void visit(EnumSpec *) { my_type = Token::ntEnumSpec;}
  virtual void visit(TypeParameter *) { my_type = 0;} // FIXME !!!
  virtual void visit(AccessSpec *) { my_type = Token::ntAccessSpec;}
  virtual void visit(AccessDecl *) { my_type = Token::ntAccessDecl;}
  virtual void visit(UserAccessSpec *) { my_type = Token::ntUserAccessSpec;}
  virtual void visit(IfStatement *) { my_type = Token::ntIfStatement;}
  virtual void visit(SwitchStatement *) { my_type = Token::ntSwitchStatement;}
  virtual void visit(WhileStatement *) { my_type = Token::ntWhileStatement;}
  virtual void visit(DoStatement *) { my_type = Token::ntDoStatement;}
  virtual void visit(ForStatement *) { my_type = Token::ntForStatement;}
  virtual void visit(TryStatement *) { my_type = Token::ntTryStatement;}
  virtual void visit(BreakStatement *) { my_type = Token::ntBreakStatement;}
  virtual void visit(ContinueStatement *) { my_type = Token::ntContinueStatement;}
  virtual void visit(ReturnStatement *) { my_type = Token::ntReturnStatement;}
  virtual void visit(GotoStatement *) { my_type = Token::ntGotoStatement;}
  virtual void visit(CaseStatement *) { my_type = Token::ntCaseStatement;}
  virtual void visit(DefaultStatement *) { my_type = Token::ntDefaultStatement;}
  virtual void visit(LabelStatement *) { my_type = Token::ntLabelStatement;}
  virtual void visit(ExprStatement *) { my_type = Token::ntExprStatement;}
  virtual void visit(Expression *) { my_type = Token::ntCommaExpr;}
  virtual void visit(AssignExpr *) { my_type = Token::ntAssignExpr;}
  virtual void visit(CondExpr *) { my_type = Token::ntCondExpr;}
  virtual void visit(InfixExpr *) { my_type = Token::ntInfixExpr;}
  virtual void visit(PmExpr *) { my_type = Token::ntPmExpr;}
  virtual void visit(CastExpr *) { my_type = Token::ntCastExpr;}
  virtual void visit(UnaryExpr *) { my_type = Token::ntUnaryExpr;}
  virtual void visit(ThrowExpr *) { my_type = Token::ntThrowExpr;}
  virtual void visit(SizeofExpr *) { my_type = Token::ntSizeofExpr;}
  virtual void visit(TypeidExpr *) { my_type = Token::ntTypeidExpr;}
  virtual void visit(TypeofExpr *) { my_type = Token::ntTypeofExpr;}
  virtual void visit(NewExpr *) { my_type = Token::ntNewExpr;}
  virtual void visit(DeleteExpr *) { my_type = Token::ntDeleteExpr;}
  virtual void visit(ArrayExpr *) { my_type = Token::ntArrayExpr;}
  virtual void visit(FuncallExpr *) { my_type = Token::ntFuncallExpr;}
  virtual void visit(PostfixExpr *) { my_type = Token::ntPostfixExpr;}
  virtual void visit(DotMemberExpr *) { my_type = Token::ntDotMemberExpr;}
  virtual void visit(ArrowMemberExpr *) { my_type = Token::ntArrowMemberExpr;}
  virtual void visit(ParenExpr *) { my_type = Token::ntParenExpr;}
  
  // francis
  virtual void visit(UserStatementExpr* ) {my_type = Token::ntUserStatementExpr;}
private:
  Token::Type my_type;
};

inline Token::Type type_of(const Node *node)
{
  assert(node);
  TypeVisitor v;
  return v.type_of(const_cast<Node *>(node));
}

inline bool is_a(const Node *node, Token::Type t)
{
  if (!node) return false;
  TypeVisitor v;
  Token::Type type = v.type_of(const_cast<Node *>(node));
  return type == t;
}

inline bool is_a(const Node *node, Token::Type t1, Token::Type t2)
{
  if (!node) return false;
  TypeVisitor v;
  Token::Type type = v.type_of(const_cast<Node *>(node));
  return type == t1 || type == t2;
}

inline bool is_a(const Node *node, Token::Type t1, Token::Type t2, Token::Type t3)
{
  if (!node) return false;
  TypeVisitor v;
  Token::Type type = v.type_of(const_cast<Node *>(node));
  return type == t1 || type == t2 || type == t3;
}

}
}

#endif
