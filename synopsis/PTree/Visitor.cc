//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#include <Synopsis/PTree/Visitor.hh>
#include <Synopsis/PTree/Atoms.hh>
#include <Synopsis/PTree/Lists.hh>

using namespace Synopsis;
using namespace PTree;

void Visitor::visit(Literal *a) { visit(static_cast<Atom *>(a));}
void Visitor::visit(CommentedAtom *a) { visit(static_cast<Atom *>(a));}
void Visitor::visit(DupAtom *a) { visit(static_cast<Atom *>(a));}
void Visitor::visit(Identifier *a) { visit(static_cast<Atom *>(a));}
void Visitor::visit(Keyword *a) { visit(static_cast<Atom *>(a));}
void Visitor::visit(Kwd::Auto *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Bool *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Break *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Case *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Catch *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Char *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Class *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Continue *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Const *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Default *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Delete *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Double *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Do *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Else *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Extern *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Float *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::For *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Friend *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Goto *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Inline *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::If *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Int *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Long *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Mutable *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Namespace *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::New *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Operator *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Private *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Protected *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Public *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Register *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Return *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Short *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Signed *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Static *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Struct *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Switch *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Template *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::This *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Throw *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Try *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Typedef *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Typename *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Union *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Unsigned *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Using *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Virtual *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Void *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::Volatile *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::WChar *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Kwd::While *a) { visit(static_cast<Keyword *>(a));}
void Visitor::visit(Brace *l) { visit(static_cast<List *>(l));}
void Visitor::visit(Block *l) { visit(static_cast<Brace *>(l));}
void Visitor::visit(ClassBody *l) { visit(static_cast<Brace *>(l));}
void Visitor::visit(Typedef *l) { visit(static_cast<List *>(l));}
void Visitor::visit(TemplateDecl *l) { visit(static_cast<List *>(l));}
void Visitor::visit(TemplateInstantiation *l) { visit(static_cast<List *>(l));}
void Visitor::visit(ExternTemplate *l) { visit(static_cast<List *>(l));}
void Visitor::visit(MetaclassDecl *l) { visit(static_cast<List *>(l));}
void Visitor::visit(LinkageSpec *l) { visit(static_cast<List *>(l));}
void Visitor::visit(NamespaceSpec *l) { visit(static_cast<List *>(l));}
void Visitor::visit(Declaration *l) { visit(static_cast<List *>(l));}
void Visitor::visit(NamespaceAlias *l) { visit(static_cast<Declaration *>(l));}
void Visitor::visit(UsingDirective *l) { visit(static_cast<Declaration *>(l));}
void Visitor::visit(UsingDeclaration *l) { visit(static_cast<Declaration *>(l));}
void Visitor::visit(FunctionDefinition *l) { visit(static_cast<Declaration *>(l));}
void Visitor::visit(ParameterDeclaration *l) { visit(static_cast<List *>(l));}
void Visitor::visit(Declarator *l) { visit(static_cast<List *>(l));}
void Visitor::visit(Name *l) { visit(static_cast<List *>(l));}
void Visitor::visit(FstyleCastExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(ClassSpec *l) { visit(static_cast<List *>(l));}
void Visitor::visit(EnumSpec *l) { visit(static_cast<List *>(l));}
void Visitor::visit(TypeParameter *l) { visit(static_cast<List *>(l));}
void Visitor::visit(AccessSpec *l) { visit(static_cast<List *>(l));}
void Visitor::visit(AccessDecl *l) { visit(static_cast<List *>(l));}
void Visitor::visit(UserAccessSpec *l) { visit(static_cast<List *>(l));}
void Visitor::visit(IfStatement *l) { visit(static_cast<List *>(l));}
void Visitor::visit(SwitchStatement *l) { visit(static_cast<List *>(l));}
void Visitor::visit(WhileStatement *l) { visit(static_cast<List *>(l));}
void Visitor::visit(DoStatement *l) { visit(static_cast<List *>(l));}
void Visitor::visit(ForStatement *l) { visit(static_cast<List *>(l));}
void Visitor::visit(TryStatement *l) { visit(static_cast<List *>(l));}
void Visitor::visit(BreakStatement *l) { visit(static_cast<List *>(l));}
void Visitor::visit(ContinueStatement *l) { visit(static_cast<List *>(l));}
void Visitor::visit(ReturnStatement *l) { visit(static_cast<List *>(l));}
void Visitor::visit(GotoStatement *l) { visit(static_cast<List *>(l));}
void Visitor::visit(CaseStatement *l) { visit(static_cast<List *>(l));}
void Visitor::visit(DefaultStatement *l) { visit(static_cast<List *>(l));}
void Visitor::visit(LabelStatement *l) { visit(static_cast<List *>(l));}
void Visitor::visit(ExprStatement *l) { visit(static_cast<List *>(l));}
void Visitor::visit(UserStatement *l) { visit(static_cast<List *>(l));} // francis
void Visitor::visit(Expression *l) { visit(static_cast<List *>(l));}
void Visitor::visit(AssignExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(CondExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(InfixExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(PmExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(CastExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(UnaryExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(ThrowExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(SizeofExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(OffsetofExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(TypeidExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(TypeofExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(NewExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(DeleteExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(ArrayExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(FuncallExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(PostfixExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(DotMemberExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(ArrowMemberExpr *l) { visit(static_cast<List *>(l));}
void Visitor::visit(ParenExpr *l) { visit(static_cast<List *>(l));}

// francis
void Visitor::visit(UserStatementExpr *l) { visit(static_cast<List *>(l));}
