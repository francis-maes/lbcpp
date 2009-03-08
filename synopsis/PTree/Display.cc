//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//

#include <Synopsis/PTree/Display.hh>
#include <typeinfo>
#include <memory>
#include <cstdlib>
#if defined(__GNUC__) &&  __GNUC__ >= 3
# include <cxxabi.h>

# if __GNUC__ == 3 && __GNUC_MINOR__ == 0
namespace abi
{
  extern "C" char* __cxa_demangle(char const*, char*, std::size_t*, int*);
}
# endif 
#endif

namespace
{

struct free_mem
{
  free_mem(char *p) : p(p) {}
  ~free_mem() { std::free(p);}
  char * p;
};

std::string demangle(char const *mangled)
{
#if defined(__GNUC__) &&  __GNUC__ >= 3
  std::string demangled;
  int status;
  free_mem keeper(abi::__cxa_demangle(mangled, 0, 0, &status));
  assert(status != -3); // invalid argument error
  if (status == -1) { throw std::bad_alloc();}
  else
    // On failure return the mangled name.
    demangled = status == -2 ? mangled : keeper.p;
  return demangled.substr(17); // skip 'Synopsis::PTree::' prefix
#else
  return mangled;
#endif
}
}

// francis
std::string typeIdName(char const *mangled)
  {return demangle(mangled);}

using namespace Synopsis;
using namespace PTree;

Display::Display(std::ostream &os, bool encoded)
  : my_os(os),
    my_indent(0),
    my_encoded(encoded)
{
}

void Display::display(Node const *n)
{
  if (n) const_cast<Node *>(n)->accept(this);
  else my_os << "nil";
  my_os.put('\n');
}

void Display::visit(Atom *a)
{
  char const *p = a->position();
  size_t n = a->length();

  // Recall that [, ], and @ are special characters.

  if(n < 1) return;
  else if(n == 1 && *p == '@')
  {
    my_os << "\\@";
    return;
  }

  char c = *p++;
  if(c == '[' || c == ']') my_os << '\\' << c; // [ and ] at the beginning are escaped.
  else my_os << c;
  while(--n > 0) my_os << *p++;
}

void Display::visit(List *l) 
{
  Node *rest = l;
  my_os << '[';
  while(rest != 0)
  {
    if(rest->is_atom())
    {
      my_os << "@ ";
      rest->accept(this);
      rest = 0;
    }
    else
    {
      Node *head = rest->car();
      if(head == 0) my_os << "nil";
      else
      {
	head->accept(this);
      }
      rest = rest->cdr();
      if(rest != 0) my_os << ' ';
    }
  }
  my_os << ']';
}

void Display::visit(DupAtom *a)
{
  char const *pos = a->position();
  size_t length = a->length();

  if(length == 1 && *pos == '@')
  {
    my_os << "\\@";
    return;
  }

  my_os << '`';
  for(size_t i = 0; i < length; ++i)
    if(pos[i] == '[' || pos[i] == ']') my_os << '\\' << pos[i];
    else my_os << pos[i];
  my_os << '`';
}

void Display::visit(Brace *l)
{
  ++my_indent;
  my_os << "[{";
  Node *body = second(l);
  if(!body)
  {
    newline();
    my_os << "nil";
  }
  else
    while(body)
    {
      newline();
      if(body->is_atom())
      {
	my_os << "@ ";
	body->accept(this);
      }
      else
      {
	Node *head = body->car();
	if(!head) my_os << "nil";
	else
	{
	  head->accept(this);
	}
      }
      body = body->cdr();
    }
  --my_indent;
  newline();
  my_os << "}]";
}

void Display::newline()
{
  my_os.put('\n');
  for(size_t i = 0; i != my_indent; ++i) my_os.put(' ');
}

void Display::print_encoded(List *l)
{
  if (my_encoded)
  {
    Encoding const &type = l->encoded_type();
    if(!type.empty()) my_os << '#' << type;
    Encoding const &name = l->encoded_name();
    if(!name.empty()) my_os << '@' << name;
  }
  visit(static_cast<List *>(l));
}

RTTIDisplay::RTTIDisplay(std::ostream &os, bool encoded)
  : my_os(os),
    my_indent(0),
    my_encoded(encoded)
{
}

void RTTIDisplay::display(Node const *n)
{
  if (n) const_cast<Node *>(n)->accept(this);
  else my_os << "nil";
  my_os.put('\n');
}

void RTTIDisplay::visit(Atom *a)
{
  newline();
  my_os << demangle(typeid(*a).name()) << ": ";
  char const *p = a->position();
  size_t n = a->length();

  if(n < 1) return;
  else if(n == 1 && *p == '@')
  {
    my_os << "\\@";
    return;
  }

  my_os << *p++;
  while(--n > 0) my_os << *p++;
}

void RTTIDisplay::visit(List *l) 
{
  newline();
  my_os << demangle(typeid(*l).name()) << ": ";
  if (my_encoded)
  {
    Encoding type = l->encoded_type();
    if(!type.empty())
    {
      my_os << "type=" << type << ' ';
    }
    Encoding name = l->encoded_name();
    if(!name.empty())
    {
      my_os << "name=" << name;
    }
  }
  ++my_indent;
  Node *rest = l;
  while(rest != 0)
  {
    if(rest->is_atom())
    {
      rest->accept(this);
      rest = 0;
    }
    else
    {
      Node *head = rest->car();
      if(head == 0)
      {
	newline();
	my_os << "nil";
      }
      else
      {
	head->accept(this);
      }
      rest = rest->cdr();
    }
  }
  --my_indent;
}

void RTTIDisplay::visit(DupAtom *a)
{
  newline();
  my_os << demangle(typeid(*a).name()) << ": ";
  char const *pos = a->position();
  size_t length = a->length();

  if(length == 1 && *pos == '@')
  {
    my_os << "\\@";
    return;
  }

  my_os << '`';
  for(size_t i = 0; i < length; ++i) my_os << pos[i];
  my_os << '`';
}

void RTTIDisplay::newline()
{
  my_os.put('\n');
  for(size_t i = 0; i != my_indent; ++i) my_os << "  ";
}

DotFileGenerator::DotFileGenerator(std::ostream &os) : my_os(os) {}
void DotFileGenerator::write(PTree::Node const *ptree)
{
  my_os << "digraph PTree\n{\n"
	<< "node[fillcolor=\"#ffffcc\", pencolor=\"#424242\" style=\"filled\"];\n";
  const_cast<Node *>(ptree)->accept(this);
  my_os << '}' << std::endl;
}

void DotFileGenerator::visit(PTree::Atom *a)
{
  my_os << (long)a 
	<< " [label=\"" << std::string(a->position(), a->length())
	<< "\" fillcolor=\"#ffcccc\"];\n";
}

void DotFileGenerator::visit(PTree::List *l)
{
  my_os << (long)l 
	<< " [label=\"" << demangle(typeid(*l).name()) << "\"];\n";
  if (l->car())
  {
    l->car()->accept(this);
    my_os << (long)l << "->" 
	  << (long)l->car() << ';' << std::endl;
  }
  if (l->cdr())
  {
    l->cdr()->accept(this);
    my_os << (long)l << "->" 
	  << (long)l->cdr() << ';' << std::endl;
  }
}

