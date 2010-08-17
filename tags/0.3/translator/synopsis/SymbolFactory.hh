//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_SymbolFactory_hh_
#define Synopsis_SymbolFactory_hh_

#include <synopsis/SymbolLookup/Scope.hh>
#include <stack>

namespace Synopsis
{
namespace SymbolLookup
{
class PrototypeScope;
class TemplateParameterScope;
}

//. SymbolFactory populates a symbol table.
class SymbolFactory
{
public:
  //.
  enum Language { NONE = 0x00, C99 = 0x01, CXX = 0x02};

  //. Create a symbol lookup table for the given language.
  //. Right now only CXX is supported.
  SymbolFactory(Language = CXX);

  SymbolLookup::Scope *current_scope() { return my_scopes.top();}
  SymbolLookup::PrototypeScope *pop_current_prototype(); // francis

  void enter_scope(PTree::NamespaceSpec const *);
  void enter_scope(PTree::ClassSpec const *);
  void enter_scope(PTree::Node const *);
  void enter_scope(PTree::FunctionDefinition const *);
  
  // francis
  void enter_scope(PTree::UserStatement const *);

  void enter_scope(PTree::TemplateDecl const *);
  void enter_scope(PTree::Block const *);
  void leave_scope();

  void declare(PTree::Declaration const *);
  void declare(PTree::Typedef const *);
  //. declare the enumeration as a new TYPE as well as all the enumerators as CONST
  void declare(PTree::EnumSpec const *);
  //. declare the namespace as a new NAMESPACE
  void declare(PTree::NamespaceSpec const *);
  //. declare the class as a new TYPE
  void declare(PTree::ClassSpec const *);
  void declare(PTree::TemplateDecl const *);
  void declare(PTree::TypeParameter const *);
  void declare(PTree::UsingDirective const *);
  void declare(PTree::ParameterDeclaration const *);
  void declare(PTree::UsingDeclaration const *);

private:
  typedef std::stack<SymbolLookup::Scope *> Scopes;

  //. Lookup the scope of a qualified name.
  //. The encoded name is modified in place to
  //. refer to the unqualified name.
  SymbolLookup::Scope *lookup_scope_of_qname(PTree::Encoding &, PTree::Node const *);

  Language                      my_language;
  Scopes                        my_scopes;
  //. When parsing a function definition the declarator is seen first,
  //. and thus a prototype is created to hold the parameters.
  //. Later, when the function definition proper is seen, the symbols
  //. are transfered and the prototype is deleted.
  SymbolLookup::PrototypeScope *my_prototype;
  //. When parsing a class or function template the template-parameter-list
  //. is seen first. Since ClassSpec and Declarator don't know they are part
  //. of a template declaration, we cache it here so it gets consumed when
  //. the Class or PrototypeScope are created.
  // FIXME: Should ClassSpec get a flag so it knows it's a template, similar
  //        to Encodings helt in Declarators ?
  SymbolLookup::TemplateParameterScope *my_template_parameters;
};

}

#endif
