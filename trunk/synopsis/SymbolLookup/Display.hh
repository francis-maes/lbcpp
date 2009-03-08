//
// Copyright (C) 2005 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_SymbolLookup_Display_hh_
#define Synopsis_SymbolLookup_Display_hh_

#include <Synopsis/SymbolLookup.hh>

namespace Synopsis
{
namespace SymbolLookup
{

class SymbolDisplay : private SymbolVisitor
{
public:
  SymbolDisplay(std::ostream &os, size_t indent)
    : my_os(os), my_indent(indent, ' ') {}
  void display(PTree::Encoding const &, SymbolLookup::Symbol const *);
private:
  std::ostream &prefix(std::string const &type)
  { return my_os << my_indent << type;}
  virtual void visit(SymbolLookup::Symbol const *);
  virtual void visit(SymbolLookup::VariableName const *);
  virtual void visit(SymbolLookup::ConstName const *);
  virtual void visit(SymbolLookup::TypeName const *);
  virtual void visit(SymbolLookup::TypedefName const *);
  virtual void visit(SymbolLookup::ClassName const *);
  virtual void visit(SymbolLookup::EnumName const *);
  virtual void visit(SymbolLookup::ClassTemplateName const *);
  virtual void visit(SymbolLookup::FunctionName const *);
  virtual void visit(SymbolLookup::FunctionTemplateName const *);
  virtual void visit(SymbolLookup::NamespaceName const *);

  std::ostream &my_os;
  std::string   my_indent;
  std::string   my_name;
};

//. The ScopeDisplay class provides an annotated view of the symbol table,
//. for debugging purposes.
class ScopeDisplay : private SymbolLookup::ScopeVisitor
{
public:
  ScopeDisplay(std::ostream &os) : my_os(os), my_indent(0) {}
  virtual ~ScopeDisplay() {}
  void display(SymbolLookup::Scope const *s) 
  { const_cast<SymbolLookup::Scope *>(s)->accept(this);}
private:
  virtual void visit(SymbolLookup::TemplateParameterScope *);
  virtual void visit(SymbolLookup::LocalScope *);
  virtual void visit(SymbolLookup::PrototypeScope *);
  virtual void visit(SymbolLookup::FunctionScope *);
  virtual void visit(SymbolLookup::Class *);
  virtual void visit(SymbolLookup::Namespace *);
  virtual void visit(SymbolLookup::UserScope *); // francis

  void dump(SymbolLookup::Scope const *);
  std::ostream &indent();

  std::ostream &my_os;
  size_t        my_indent;
};

inline void display(SymbolLookup::Scope const *s, std::ostream &os)
{
  ScopeDisplay sd(os);
  sd.display(s);
}

}
}

#endif
