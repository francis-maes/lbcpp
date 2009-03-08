//
// Copyright (C) 2005 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#include "Display.hh"

using namespace Synopsis;
using namespace SymbolLookup;

void SymbolDisplay::display(PTree::Encoding const &name,
			    SymbolLookup::Symbol const *symbol)
{
  my_name = name.unmangled();
  symbol->accept(this);
  my_os << std::endl;
}

void SymbolDisplay::visit(SymbolLookup::Symbol const *) {}

void SymbolDisplay::visit(SymbolLookup::VariableName const *name)
{
  prefix("Variable:          ") << my_name << ' ' << name->type().unmangled();
}

void SymbolDisplay::visit(SymbolLookup::ConstName const *name)
{
  prefix("Const:             ") << my_name << ' ' << name->type().unmangled();
  if (name->defined()) my_os << " (" << name->value() << ')';
}

void SymbolDisplay::visit(SymbolLookup::TypeName const *name)
{
  prefix("Type:              ") << my_name << ' ' << name->type().unmangled();
}

void SymbolDisplay::visit(SymbolLookup::TypedefName const *name)
{
  prefix("Typedef:           ") << my_name << ' ' << name->type().unmangled();
}

void SymbolDisplay::visit(SymbolLookup::ClassName const *name)
{
  prefix("Class:             ") << my_name << ' ' << name->type().unmangled();
}

void SymbolDisplay::visit(SymbolLookup::EnumName const *name)
{
  prefix("Enum:              ") << my_name << ' ' << name->type().unmangled();
}

void SymbolDisplay::visit(SymbolLookup::ClassTemplateName const *name)
{
  prefix("Class template:    ") << my_name << ' ' << name->type().unmangled();
}

void SymbolDisplay::visit(SymbolLookup::FunctionName const *name)
{
  prefix("Function:          ") << my_name << ' ' << name->type().unmangled();
}

void SymbolDisplay::visit(SymbolLookup::FunctionTemplateName const *name)
{
  prefix("Function template: ") << my_name << ' ' << name->type().unmangled();
}

void SymbolDisplay::visit(SymbolLookup::NamespaceName const *name)
{
  prefix("Namespace:         ") << my_name << ' ' << name->type().unmangled();
}

void ScopeDisplay::visit(SymbolLookup::TemplateParameterScope *s)
{
  indent() << "TemplateParameterScope:\n";
  dump(s);
}

void ScopeDisplay::visit(SymbolLookup::LocalScope *s)
{
  indent() << "LocalScope:\n";
  dump(s);
}

void ScopeDisplay::visit(SymbolLookup::PrototypeScope *s)
{
  indent() << "PrototypeScope '" << s->name() << "':\n";
  dump(s);
}

void ScopeDisplay::visit(SymbolLookup::FunctionScope *s)
{
  indent() << "FunctionScope '" << s->name() << "':\n";
  dump(s);
}

void ScopeDisplay::visit(SymbolLookup::Class *s)
{
  indent() << "Class '" << s->name() << "':\n";
  dump(s);
}

void ScopeDisplay::visit(SymbolLookup::Namespace *s)
{
  indent() << "Namespace '" << s->name() << "':\n";
  dump(s);
}

// francis
void ScopeDisplay::visit(SymbolLookup::UserScope *s)
{
  indent() << "UserScope '" << s->name() << "':\n";
  dump(s);
}

void ScopeDisplay::dump(SymbolLookup::Scope const *s)
{
  ++my_indent;
  for (SymbolLookup::Scope::symbol_iterator i = s->symbols_begin();
       i != s->symbols_end();
       ++i)
  {
    SymbolDisplay display(my_os, my_indent);
    display.display(i->first, i->second);
  }
  for (SymbolLookup::Scope::scope_iterator i = s->scopes_begin();
       i != s->scopes_end();
       ++i)
    i->second->accept(this);
  --my_indent;
}

std::ostream &ScopeDisplay::indent()
{
  size_t i = my_indent;
  while (i--) my_os.put(' ');
  return my_os;
}
