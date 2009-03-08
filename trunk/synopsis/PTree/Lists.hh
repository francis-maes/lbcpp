//
// Copyright (C) 1997-2000 Shigeru Chiba
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_PTree_Lists_hh_
#define Synopsis_PTree_Lists_hh_

#include <synopsis/PTree/operations.hh>
#include <synopsis/PTree/Encoding.hh>

namespace Synopsis
{
namespace PTree
{

class Brace : public List 
{
public:
  Brace(Node *p, Node *q) : List(p, q) {}
  Brace(Node *ob, Node *body, Node *cb) : List(ob, list(body, cb)) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class Block : public Brace 
{
public:
  Block(Node *p, Node *q) : Brace(p, q) {}
  Block(Node *ob, Node *bdy, Node *cb) : Brace(ob, bdy, cb) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class ClassBody : public Brace 
{
public:
  ClassBody(Node *p, Node *q) : Brace(p, q) {}
  ClassBody(Node *ob, Node *bdy, Node *cb) : Brace(ob, bdy, cb) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class TemplateDecl : public List
{
public:
  TemplateDecl(Node *p, Node *q) : List(p, q) {}
  TemplateDecl(Node *p) : List(p, 0) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class TemplateInstantiation : public List
{
public:
  TemplateInstantiation(Node *p, Node *q) : List(p, q) {} // francis
  TemplateInstantiation(Node *p) : List(p, 0) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class ExternTemplate : public List
{
public:
  ExternTemplate(Node *p, Node *q) : List(p, q) {}
  ExternTemplate(Node *p) : List(p, 0) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class MetaclassDecl : public List
{
public:
  MetaclassDecl(Node *p, Node *q) : List(p, q) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class LinkageSpec : public List
{
public:
  LinkageSpec(Node *p, Node *q) : List(p, q) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class NamespaceSpec : public List
{
public:
  NamespaceSpec(Node *p, Node *q) : List(p, q), my_comments(0) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}  
  Node *get_comments() { return my_comments;}
  void set_comments(Node *c) { my_comments = c;}

private:
  Node *my_comments;
};

class Declaration : public List
{
public:
  Declaration(Node *p, Node *q) : List(p, q), my_comments(0) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
  Node *get_comments() { return my_comments;}
  void set_comments(Node *c) { my_comments = c;}

private:
  Node *my_comments;
};

class Typedef : public Declaration
{
public:
  Typedef(Node *p) : Declaration(p, 0) {}
  Typedef(Node *p, Node *q) : Declaration(p, q) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class UsingDirective : public Declaration
{
public:
  UsingDirective(Node *p, Node* q) : Declaration(p, q) {} // francis
  
  UsingDirective(Node *p) : Declaration(p, 0) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class UsingDeclaration : public Declaration
{
public:
  UsingDeclaration(Node *p, Node *q) : Declaration(p, q) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class NamespaceAlias : public Declaration
{
public:
  NamespaceAlias(Node *p, Node *q) : Declaration(p, q) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class FunctionDefinition : public Declaration
{
public:
  FunctionDefinition(Node *p, Node *q) : Declaration(p, q) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class ParameterDeclaration : public List 
{
public:
  // francis
  ParameterDeclaration(Node *p, Node *q) : List(p, q) {}
  ParameterDeclaration(Node *mod, Node *type, Node *decl)
    : List(mod, list(type, decl)) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class Declarator : public List
{
public:
  Declarator(Node *);
  Declarator(Node *, Node* ); // francis
  Declarator(Node *, Encoding const&, Encoding const&, Node *);
  Declarator(Encoding const&, Encoding const&, Node *);
  Declarator(Node *, Node *, Encoding const&, Encoding const&, Node *);
  Declarator(Node *, Encoding const&);
  Declarator(Encoding const&);
  Declarator(Declarator*, Node *, Node *);

  virtual void accept(Visitor *visitor) { visitor->visit(this);}
  Encoding encoded_type() const { return my_type;}
  Encoding encoded_name() const { return my_name;}
  void set_encoded_type(const Encoding &t) { my_type = t;}
  Node *name() { return my_declared_name;}
  Node *initializer();
  Node *get_comments() { return my_comments;}
  void set_comments(Node *c) { my_comments = c;}
private:
  Encoding my_type;
  Encoding my_name;
  Node    *my_declared_name;
  Node    *my_comments;
};

class Name : public List
{
public:
  Name(Node *, const Encoding &);
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
  Encoding encoded_name() const { return my_name;}
private:
  Encoding my_name;
};

class FstyleCastExpr : public List
{
public:
  FstyleCastExpr(const Encoding &, Node *, Node *);
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
  Encoding encoded_type() const { return my_type;}
private:
  Encoding my_type;
};

class ClassSpec : public List
{
public:
  ClassSpec(Node *, Node *, Node *);
  ClassSpec(const Encoding &, Node *, Node *, Node *);
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
  Encoding encoded_name() const { return my_name;}
  void set_encoded_name(const Encoding &n) { my_name = n;}
  Node *get_comments() { return my_comments;}
  //. The list of base classes, i.e. [: [public A] , [public virtual B] ...]
  Node const *base_clause() const 
  { return static_cast<List const *>(PTree::third(this));}
  //. The following assumes proper C++, i.e. no OpenC++ extension.
  ClassBody *body();
private:
  Encoding my_name;
  Node    *my_comments;
};

class EnumSpec : public List
{
public:
  EnumSpec(Node *p, Node *q) : List(p, q) {} // francis
  
  EnumSpec(Node *head) : List(head, 0) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
  Encoding encoded_name() const { return my_name;}
  void set_encoded_name(const Encoding &n) { my_name = n;}
private:
  Encoding my_name;
};

class TypeParameter : public List 
{
public:
  TypeParameter(Node *p, Node *q) : List(p, q) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class AccessSpec : public List
{
public:
  AccessSpec(Node *p, Node *q, Node *c = NULL) : List(p, q), my_comments(c) {} // francis: c = NULL
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
  Node *get_comments() { return my_comments;}
private:
  Node *my_comments;
};

class AccessDecl : public List
{
public:
  AccessDecl(Node *p, Node *q) : List(p, q) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class UserAccessSpec : public List
{
public:
  UserAccessSpec(Node *p, Node *q) : List(p, q) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class UserdefKeyword : public List
{
public:
  UserdefKeyword(Node *p, Node *q) : List(p, q) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

template <typename T>
class StatementT : public List
{
public:
  StatementT(Node *p, Node *q) : List(p, q) {}
  virtual void accept(Visitor *visitor) { visitor->visit(static_cast<T *>(this));}
};

class IfStatement : public StatementT<IfStatement> 
{
public:
  IfStatement(Node *p, Node *q) : StatementT<IfStatement>(p, q) {} 
};

class SwitchStatement : public StatementT<SwitchStatement> 
{
public:
  SwitchStatement(Node *p, Node *q) : StatementT<SwitchStatement>(p, q) {} 
};

class WhileStatement : public StatementT<WhileStatement> 
{
public:
  WhileStatement(Node *p, Node *q) : StatementT<WhileStatement>(p, q) {} 
};

class DoStatement : public StatementT<DoStatement> 
{
public:
  DoStatement(Node *p, Node *q) : StatementT<DoStatement>(p, q) {} 
};

class ForStatement : public StatementT<ForStatement> 
{
public:
  ForStatement(Node *p, Node *q) : StatementT<ForStatement>(p, q) {} 
};

class TryStatement : public StatementT<TryStatement> 
{
public:
  TryStatement(Node *p, Node *q) : StatementT<TryStatement>(p, q) {} 
};

class BreakStatement : public StatementT<BreakStatement> 
{
public:
  BreakStatement(Node *p, Node *q) : StatementT<BreakStatement>(p, q) {} 
};

class ContinueStatement : public StatementT<ContinueStatement> 
{
public:
  ContinueStatement(Node *p, Node *q) : StatementT<ContinueStatement>(p, q) {} 
};

class ReturnStatement : public StatementT<ReturnStatement> 
{
public:
  ReturnStatement(Node *p, Node *q) : StatementT<ReturnStatement>(p, q) {} 
};

class GotoStatement : public StatementT<GotoStatement> 
{
public:
  GotoStatement(Node *p, Node *q) : StatementT<GotoStatement>(p, q) {} 
};

class CaseStatement : public StatementT<CaseStatement> 
{
public:
  CaseStatement(Node *p, Node *q) : StatementT<CaseStatement>(p, q) {} 
};

class DefaultStatement : public StatementT<DefaultStatement> 
{
public:
  DefaultStatement(Node *p, Node *q) : StatementT<DefaultStatement>(p, q) {} 
};

class LabelStatement : public StatementT<LabelStatement> 
{
public:
  LabelStatement(Node *p, Node *q) : StatementT<LabelStatement>(p, q) {} 
};

class ExprStatement : public StatementT<ExprStatement> 
{
public:
  ExprStatement(Node *p, Node *q) : StatementT<ExprStatement>(p, q) {} 
};

// francis
class UserStatement : public StatementT<UserStatement>
{
public:
  UserStatement(Node *p, Node *q) : StatementT<UserStatement>(p, q) {} 
};

class Expression : public List
{
public:
  Expression(Node *p, Node *q) : List(p, q) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

template <typename T>
class ExpressionT : public List
{
public:
  ExpressionT(Node *p, Node *q) : List(p, q) {}
  virtual void accept(Visitor *visitor) { visitor->visit(static_cast<T *>(this));}
};

class AssignExpr : public ExpressionT<AssignExpr> 
{
public:
  AssignExpr(Node *p, Node *q) : ExpressionT<AssignExpr>(p, q) {} 
};

class CondExpr : public ExpressionT<CondExpr> 
{
public:
  CondExpr(Node *p, Node *q) : ExpressionT<CondExpr>(p, q) {} 
};

class InfixExpr : public ExpressionT<InfixExpr> 
{
public:
  InfixExpr(Node *p, Node *q) : ExpressionT<InfixExpr>(p, q) {} 
};

class PmExpr : public ExpressionT<PmExpr> 
{
public:
  PmExpr(Node *p, Node *q) : ExpressionT<PmExpr>(p, q) {} 
};

class CastExpr : public ExpressionT<CastExpr> 
{
public:
  CastExpr(Node *p, Node *q) : ExpressionT<CastExpr>(p, q) {} 
};

class UnaryExpr : public ExpressionT<UnaryExpr> 
{
public:
  UnaryExpr(Node *p, Node *q) : ExpressionT<UnaryExpr>(p, q) {} 
};

class ThrowExpr : public ExpressionT<ThrowExpr> 
{
public:
  ThrowExpr(Node *p, Node *q) : ExpressionT<ThrowExpr>(p, q) {} 
};

class SizeofExpr : public ExpressionT<SizeofExpr> 
{
public:
  SizeofExpr(Node *p, Node *q) : ExpressionT<SizeofExpr>(p, q) {} 
};

class OffsetofExpr : public ExpressionT<OffsetofExpr> 
{
public:
  OffsetofExpr(Node *p, Node *q) : ExpressionT<OffsetofExpr>(p, q) {} 
};

class TypeidExpr : public ExpressionT<TypeidExpr> 
{
public:
  TypeidExpr(Node *p, Node *q) : ExpressionT<TypeidExpr>(p, q) {} 
};

class TypeofExpr : public ExpressionT<TypeofExpr> 
{
public:
  TypeofExpr(Node *p, Node *q) : ExpressionT<TypeofExpr>(p, q) {} 
};

class NewExpr : public ExpressionT<NewExpr> 
{
public:
  NewExpr(Node *p, Node *q) : ExpressionT<NewExpr>(p, q) {} 
};

class DeleteExpr : public ExpressionT<DeleteExpr> 
{
public:
  DeleteExpr(Node *p, Node *q) : ExpressionT<DeleteExpr>(p, q) {} 
};

class ArrayExpr : public ExpressionT<ArrayExpr> 
{
public:
  ArrayExpr(Node *p, Node *q) : ExpressionT<ArrayExpr>(p, q) {} 
};

class FuncallExpr : public ExpressionT<FuncallExpr> 
{
public:
  FuncallExpr(Node *p, Node *q) : ExpressionT<FuncallExpr>(p, q) {} 
};

class PostfixExpr : public ExpressionT<PostfixExpr> 
{
public:
  PostfixExpr(Node *p, Node *q) : ExpressionT<PostfixExpr>(p, q) {} 
};

class UserStatementExpr : public ExpressionT<UserStatementExpr> 
{
public:
  UserStatementExpr(Node *p, Node *q) : ExpressionT<UserStatementExpr>(p, q) {} 
};

class DotMemberExpr : public ExpressionT<DotMemberExpr> 
{
public:
  DotMemberExpr(Node *p, Node *q) : ExpressionT<DotMemberExpr>(p, q) {} 
};

class ArrowMemberExpr : public ExpressionT<ArrowMemberExpr> 
{
public:
  ArrowMemberExpr(Node *p, Node *q) : ExpressionT<ArrowMemberExpr>(p, q) {} 
};

class ParenExpr : public ExpressionT<ParenExpr> 
{
public:
  ParenExpr(Node *p, Node *q) : ExpressionT<ParenExpr>(p, q) {} 
};

class StaticUserStatementExpr : public ExpressionT<StaticUserStatementExpr> 
{
public:
  StaticUserStatementExpr(Node *p, Node *q) : ExpressionT<StaticUserStatementExpr>(p, q) {} 
};

}
}

#endif
