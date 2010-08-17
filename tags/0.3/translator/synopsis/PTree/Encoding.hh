//
// Copyright (C) 1997 Shigeru Chiba
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_PTree_Encoding_hh_
#define Synopsis_PTree_Encoding_hh_

#include <string>
#include <iostream>
#include <cassert>
#include <cstring>
#include <cstdio>

namespace Synopsis
{
namespace PTree
{

class Node;
class Atom;

//. An Encoding represents a mangled (type) name. Here is a quick reference of the grammar:
//.
//. - `b`: boolean
//. - `c`: char
//. - `w`: wchar_t
//. - `i`: int (signed, unsigned)
//. - `s`: short (short int)
//. - `l`: long (long int)
//. - `j`: long long
//. - `f`: float
//. - `d`: double
//. - `r`: long double
//. - `v`: void
//. - `T`: template class (e.g. `Foo<int,char>` ==> `T[3]Foo[2]ic`.  `[2]` means
//.   the length of `ic`.  It doesn't mean the number of template arguments.
//.
//. - `e`: ...
//. - `?`: no return type.  the return type of constructors
//. - `*`: non-type template parameter
//.
//. - `S`: `signed`
//. - `U`: `unsigned`
//. - `C`: `const`
//. - `V`: `volatile`
//.
//. - `P`: pointer
//. - `R`: reference
//. - `A`: array (e.g. `char[16]` ==> `A16_c`)
//. - `F`: function (e.g. `char foo(int)` ==> `Fi_c`)
//. - `M`: pointer to member (e.g. `Type::*` ==> `M[4]Type`)
//.
//. - `Q`: qualified class (e.g. `X::YY` ==> `Q[2][1]X[2]YY`, `::YY` ==> `Q[2][0][2]YY`)
//.
//. - `[x]`: means `0x80 + x`
//. - `0`: means `::` (global scope)
//.
//. Special function names:
//.
//. - operator + ==> +
//. - operator new[] ==> new[]
//. - operator <type> ==> @<encoded type>		cast operator
//.
class Encoding 
{
public:
  struct char_traits
  {
    typedef unsigned char  char_type;
    typedef unsigned long  int_type;
    typedef std::streampos pos_type;
    typedef std::streamoff off_type;
    typedef std::mbstate_t state_type;

    static void assign(char_type &c1, const char_type &c2) { c1 = c2;}
    static bool eq(const char_type &c1, const char_type &c2) { return c1 == c2;}
    static bool lt(const char_type &c1, const char_type &c2) { return c1 < c2;}
    static int compare(const char_type *s1, const char_type *s2, std::size_t n) { return memcmp(s1, s2, n);}
    static std::size_t length(const char_type *s) { return strlen((const char *)s);}
    static const char_type *find(const char_type *s, std::size_t n, const char_type &a)
    { return static_cast<const char_type *>(memchr(s, a, n));}
    static char_type *move(char_type *s1, const char_type *s2, std::size_t n)
    { return static_cast<char_type *>(memmove(s1, s2, n));}
    static char_type *copy(char_type *s1, const char_type *s2, std::size_t n)
    { return static_cast<char_type *>(memcpy(s1, s2, n));}
    static char_type *assign(char_type *s, std::size_t n, char_type a)
    { return static_cast<char_type *>(memset(s, a, n));}
    static char_type to_char_type(const int_type &c) { return static_cast<char_type>(c);}
    static int_type to_int_type(const char_type &c) { return static_cast<int_type>(c);}
    static bool eq_int_type(const int_type &c1, const int_type &c2) { return c1 == c2;}
    static int_type eof() { return static_cast<int_type>(EOF);}
    static int_type not_eof(const int_type &c) { return !eq_int_type(c, eof()) ? c : to_int_type(char_type());}
  };

#pragma warning(disable:4996)
  typedef std::basic_string<unsigned char, char_traits> Code;
  typedef Code::const_iterator iterator;

  static void do_init_static();

  Encoding() {}
  Encoding(const Code &b) : my_buffer(b) {}
  Encoding(const char *b) : my_buffer(b, b + strlen(b)) {}
  Encoding(const char *b, size_t s) : my_buffer(b, b + s) {}
  Encoding(iterator b, iterator e) : my_buffer(b, e) {}
  static Encoding simple_name(PTree::Atom const *name);

  void clear() { my_buffer.clear();}
  bool empty() const { return my_buffer.empty();}
  size_t size() const { return my_buffer.size();}
  iterator begin() const { return my_buffer.begin();}
  iterator end() const { return my_buffer.end();}
  unsigned char front() const { return *begin();}
  unsigned char at(size_t i) const { return my_buffer.at(i);}
  //. return a copy of the underlaying buffer
  //. FIXME: this is a temporary workaround while there are
  //. still places that use raw strings
  const char *copy() const;

  bool operator == (const Encoding &e) const { return my_buffer == e.my_buffer;}
  bool operator == (const std::string &s) const { return my_buffer == (const unsigned char *)s.c_str();}
  bool operator == (const char *s) const { return my_buffer == (const unsigned char *)s;}

  void prepend(unsigned char c) { my_buffer.insert(my_buffer.begin(), c);}
  void prepend(const char *p, size_t s) { my_buffer.insert(0, (const unsigned char *)p, s);}
  void prepend(const Encoding &e) { my_buffer.insert(0, e.my_buffer);}

  void append(unsigned char c) { my_buffer.append(1, c);}
  void append(const char *p, size_t s) { my_buffer.append((const unsigned char *)p, s);}
  void append(const Encoding &e) { my_buffer.append(e.my_buffer);}
  void append_with_length(const char *s, size_t n) { append(0x80 + n); append((const char *)s, n);}
  void append_with_length(const Encoding &e) { append(0x80 + e.size()); append(e);}

  unsigned char pop();
  void pop(size_t n) { my_buffer.erase(my_buffer.begin(), my_buffer.begin() + n);}

  void cv_qualify(const Node *, const Node * = 0);
  void simple_const() { append("Ci", 2);}
  void global_scope();
  void simple_name(const Node *);
  void anonymous();
  void template_(const Node *, const Encoding &);
  void qualified(int);
  void destructor(const Node *);
  void ptr_operator(int);
  void ptr_to_member(const Encoding &, int);
  void cast_operator(const Encoding &);
  void array() { prepend("A_", 2);}
  void array(unsigned long s);
  void function(const Encoding &e) { prepend(e);}
  void recursion(const Encoding &e) { prepend(e);}
  void start_func_args() { append('F');}
  void end_func_args() { append('_');}
  void void_() { append('v');}
  void ellipsis_arg() { append('e');}
  void no_return_type() { append('?');}
  void value_temp_param() { append('*');}

  //. if this Encoding represents a qualified name,
  //. return the name of the outer scope
  Encoding get_scope() const;
  //. if this Encoding represents a qualified name,
  //. return the name of the symbol inside the outer scope,
  //. else return the unmodified name
  Encoding get_symbol() const;
  Encoding get_template_arguments() const;
  
  std::string unmangled() const;

  PTree::Node *make_name();
  PTree::Node *make_qname();
  PTree::Node *make_ptree(PTree::Node *);
  bool is_simple_name() const { return front() >= 0x80;}
  bool is_global_scope() const { return front() == 0x80 && size() == 1;}
  bool is_qualified() const { return front() == 'Q';}
  bool is_function() const;
  bool is_template() const { return front() == 'T';}
  PTree::Node *name_to_ptree();

  friend bool operator < (const Encoding &, const Encoding &);
  friend std::ostream &operator << (std::ostream &, const Encoding &);

private:

  iterator end_of_scope() const;

  Code my_buffer;

public:
  static PTree::Node *bool_t, *char_t, *wchar_t_t, *int_t, *short_t, *long_t,
		 *float_t, *double_t, *void_t;

  static PTree::Node *signed_t, *unsigned_t, *const_t, *volatile_t;

  static PTree::Node *operator_name, *new_operator, *anew_operator,
		 *delete_operator, *adelete_operator;

  static PTree::Node *star, *ampersand, *comma, *dots, *scope, *tilder,
		 *left_paren, *right_paren, *left_bracket, *right_bracket,
		 *left_angle, *right_angle;
};

inline bool operator < (const Encoding &e1, const Encoding &e2) 
{
  return e1.my_buffer < e2.my_buffer;
}

inline std::ostream &operator << (std::ostream &os, const Encoding &e)
{
  for (Encoding::iterator i = e.begin();
       i != e.end();
       ++i)
    if(*i < 0x80) os.put(static_cast<char>(*i));
    else os << '[' << static_cast<int>(*i - 0x80) << ']';
  return os;
}

inline unsigned char Encoding::pop() 
{
  unsigned char code = my_buffer[0]; 
  my_buffer.erase(0, 1); 
  return code;
}

inline bool Encoding::is_function() const 
{
  if (front() == 'F') return true;
  // const (member) function.
  else if (front() == 'C' && *(begin() + 1) == 'F') return true;
  else return false;
}

}
}

#endif
