//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_PTree_Visitor_hh_
#define Synopsis_PTree_Visitor_hh_

#include <synopsis/PTree/NodesFwd.hh>

namespace Synopsis
{
namespace PTree
{

//. The Visitor class is used to dynamically resolve
//. type information about a given Node.
//. The default implementation does nothing, so you
//. only need to implement the methods you actually need.
//. Any types for which no corresponding 'visit' methods
//. exist will be caught by the 'visit' of the closest parent. 
class Visitor
{
public:
  virtual ~Visitor() {}
  virtual void visit(Node *) {}
  virtual void visit(Atom *) {}
  virtual void visit(List *) {}
  // atoms...
  virtual void visit(Literal *);
  virtual void visit(CommentedAtom *);
  virtual void visit(DupAtom *);
  virtual void visit(Identifier *);
  virtual void visit(Keyword *);
  virtual void visit(Kwd::Auto *);
  virtual void visit(Kwd::Break *);
  virtual void visit(Kwd::Bool *);
  virtual void visit(Kwd::Case *);
  virtual void visit(Kwd::Catch *);
  virtual void visit(Kwd::Char *);
  virtual void visit(Kwd::Class *);
  virtual void visit(Kwd::Continue *);
  virtual void visit(Kwd::Const *);
  virtual void visit(Kwd::Default *);
  virtual void visit(Kwd::Delete *);
  virtual void visit(Kwd::Double *);
  virtual void visit(Kwd::Do *);
  virtual void visit(Kwd::Else *);
  virtual void visit(Kwd::Extern *);
  virtual void visit(Kwd::Float *);
  virtual void visit(Kwd::For *);
  virtual void visit(Kwd::Friend *);
  virtual void visit(Kwd::Goto *);
  virtual void visit(Kwd::Inline *);
  virtual void visit(Kwd::If *);
  virtual void visit(Kwd::Int *);
  virtual void visit(Kwd::Long *);
  virtual void visit(Kwd::Mutable *);
  virtual void visit(Kwd::Namespace *);
  virtual void visit(Kwd::New *);
  virtual void visit(Kwd::Operator *);
  virtual void visit(Kwd::Private *);
  virtual void visit(Kwd::Protected *);
  virtual void visit(Kwd::Public *);
  virtual void visit(Kwd::Register *);
  virtual void visit(Kwd::Return *);
  virtual void visit(Kwd::Short *);
  virtual void visit(Kwd::Signed *);
  virtual void visit(Kwd::Static *);
  virtual void visit(Kwd::Struct *);
  virtual void visit(Kwd::Switch *);
  virtual void visit(Kwd::Template *);
  virtual void visit(Kwd::This *);
  virtual void visit(Kwd::Throw *);
  virtual void visit(Kwd::Try *);
  virtual void visit(Kwd::Typedef *);
  virtual void visit(Kwd::Typename *);
  virtual void visit(Kwd::Union *);
  virtual void visit(Kwd::Unsigned *);
  virtual void visit(Kwd::Using *);
  virtual void visit(Kwd::Virtual *);
  virtual void visit(Kwd::Void *);
  virtual void visit(Kwd::Volatile *);
  virtual void visit(Kwd::WChar *);
  virtual void visit(Kwd::While *);
  //. [ { [ <statement>* ] } ]
  virtual void visit(Brace *);
  //. [ { [ <statement>* ] } ]
  virtual void visit(Block *);
  virtual void visit(ClassBody *);
  virtual void visit(Typedef *);
  //. [ template < [types] > [decl] ]
  virtual void visit(TemplateDecl *);
  virtual void visit(TemplateInstantiation *);
  virtual void visit(ExternTemplate *);
  virtual void visit(MetaclassDecl *);
  //. [ extern ["C++"] [{ body }] ]
  virtual void visit(LinkageSpec *);
  //. [ namespace <identifier> [{ body }] ]
  virtual void visit(NamespaceSpec *);
  //. [ using namespace Foo ; ]
  virtual void visit(UsingDirective *);
  //. One of:
  //.
  //. - Variables: [ [modifiers] name [declarators] ; ]
  //. - Function: prototype: [ [modifiers] name [declarators] ; ]
  //. - Typedef: ?
  //. - Class definition: [ [modifiers] [class foo ...] [declarators]? ; ]
  virtual void visit(Declaration *);
  //. [ namespace Foo = Bar ; ]
  virtual void visit(NamespaceAlias *);
  //. Function definition: [ [modifiers] name declarator [ { ... } ] ]
  virtual void visit(FunctionDefinition *);
  //. One of:
  //.
  //. - [ decl-specifier-seq ]
  //. - [ decl-specifier-seq declarator ]
  //. - [ decl-specifier-seq declarator = assignment-expression ]
  //. - [ decl-specifier-seq abstract-declarator ]
  //. - [ decl-specifier-seq abstract-declarator = assignment-expression ]
  //. - [ decl-specifier-seq = assignment-expression ]
  virtual void visit(ParameterDeclaration *);
  //. [ using Foo `::` x ; ]
  virtual void visit(UsingDeclaration *);
  //. [ [ declarator { = <expr> } ] , ... ]
  virtual void visit(Declarator *);
  virtual void visit(Name *);
  //. [ [type] ( [expr] ) ]
  virtual void visit(FstyleCastExpr *);
  virtual void visit(ClassSpec *);
  //. [ enum [name] [{ [name [= value] ]* }] ]
  virtual void visit(EnumSpec *);
  //. One of:
  //.
  //. - [typename]
  //. - [typename identifier]
  //. - [typename identifier = type-id]
  virtual void visit(TypeParameter *);
  virtual void visit(AccessSpec *);
  virtual void visit(AccessDecl *);
  virtual void visit(UserAccessSpec *);
  //. [ if ( expr ) statement (else statement)? ]
  virtual void visit(IfStatement *);
  //. [ switch ( expr ) statement ]
  virtual void visit(SwitchStatement *);
  //. [ while ( expr ) statement ]
  virtual void visit(WhileStatement *);
  //. [ do [{ ... }] while ( [...] ) ; ]
  virtual void visit(DoStatement *);
  //. [ for ( stmt expr ; expr ) statement ]
  virtual void visit(ForStatement *);
  //. [ try [{}] [catch ( arg ) [{}] ]* ]
  virtual void visit(TryStatement *);
  //. [ break ; ]
  virtual void visit(BreakStatement *);
  virtual void visit(ContinueStatement *);
  virtual void visit(ReturnStatement *);
  virtual void visit(GotoStatement *);
  //. [ case expr : [expr] ]
  virtual void visit(CaseStatement *);
  //. [ default : [expr] ]
  virtual void visit(DefaultStatement *);
  virtual void visit(LabelStatement *);
  virtual void visit(ExprStatement *);
  virtual void visit(UserStatement *); // francis
  //. [ expr (, expr)* ]
  virtual void visit(Expression *);
  //. [left = right]
  virtual void visit(AssignExpr *);
  virtual void visit(CondExpr *);
  //. [left op right]
  virtual void visit(InfixExpr *);
  virtual void visit(PmExpr *);
  //. ( type-expr ) expr   ..type-expr is type encoded
  virtual void visit(CastExpr *);
  //. [op expr]
  virtual void visit(UnaryExpr *);
  //. [ throw [expr] ]
  virtual void visit(ThrowExpr *);
  //. [ sizeof ( [type [???] ] ) ]
  virtual void visit(SizeofExpr *);
  virtual void visit(OffsetofExpr *);
  virtual void visit(TypeidExpr *);
  virtual void visit(TypeofExpr *);
  virtual void visit(NewExpr *);
  //. [ delete [expr] ]
  virtual void visit(DeleteExpr *);
  //. <postfix> \[ <expr> \]
  virtual void visit(ArrayExpr *);
  //. [ postfix ( args ) ]
  virtual void visit(FuncallExpr *);
  //. [ expr ++ ]
  virtual void visit(PostfixExpr *);
  //. [ postfix . name ]
  virtual void visit(DotMemberExpr *);
  //. [ postfix -> name ]
  virtual void visit(ArrowMemberExpr *);
  //. [ ( expr ) ]
  virtual void visit(ParenExpr *);
  
  // francis
  virtual void visit(UserStatementExpr* );
};

}
}

#endif
