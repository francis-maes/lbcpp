//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//

#include "Synopsis/Lexer.hh"
#include "Synopsis/Buffer.hh"
#include <iostream>
#include <cassert>
#include <string>

using namespace Synopsis;

Lexer::Lexer(Buffer *buffer, int tokenset)
  : my_buffer(buffer),
    my_token(my_buffer->ptr(), 0, '\n')
{
  my_keywords["asm"] = Token::ATTRIBUTE;
  my_keywords["auto"] = Token::AUTO;
  my_keywords["break"] = Token::BREAK;
  my_keywords["case"] = Token::CASE;
  my_keywords["char"] = Token::CHAR;
  // FIXME: Add support for _Complex to Parser.
  my_keywords["_Complex"] = Token::Ignore;
  my_keywords["const"] = Token::CONST;
  my_keywords["continue"] = Token::CONTINUE;
  my_keywords["default"] = Token::DEFAULT;
  my_keywords["do"] = Token::DO;
  my_keywords["double"] = Token::DOUBLE;
  my_keywords["else"] = Token::ELSE;
  my_keywords["enum"] = Token::ENUM;
  my_keywords["extern"] = Token::EXTERN;
  my_keywords["float"] = Token::FLOAT;
  my_keywords["for"] = Token::FOR;
  my_keywords["goto"] = Token::GOTO;
  my_keywords["if"] = Token::IF;
  my_keywords["inline"] = Token::INLINE;
  my_keywords["int"] = Token::INT;
  my_keywords["long"] = Token::LONG;
  my_keywords["register"] = Token::REGISTER;
  my_keywords["return"] = Token::RETURN;
  my_keywords["short"] = Token::SHORT;
  my_keywords["signed"] = Token::SIGNED;
  my_keywords["sizeof"] = Token::SIZEOF;
  my_keywords["static"] = Token::STATIC;
  my_keywords["struct"] = Token::STRUCT;
  my_keywords["switch"] = Token::SWITCH;
  my_keywords["typedef"] = Token::TYPEDEF;
  my_keywords["union"] = Token::UNION;
  my_keywords["unsigned"] = Token::UNSIGNED;
  my_keywords["void"] = Token::VOID;
  my_keywords["volatile"] = Token::VOLATILE;
  my_keywords["while"] = Token::WHILE;
  if (tokenset & CXX)
  {
    my_keywords["bool"] = Token::BOOLEAN;
    my_keywords["catch"] = Token::CATCH;
    my_keywords["class"] = Token::CLASS;
    my_keywords["delete"] = Token::DELETE;
    my_keywords["false"] = Token::Constant;
    my_keywords["friend"] = Token::FRIEND;
    my_keywords["mutable"] = Token::MUTABLE;
    my_keywords["namespace"] = Token::NAMESPACE;
    my_keywords["new"] = Token::NEW;
    my_keywords["operator"] = Token::OPERATOR;
    my_keywords["private"] = Token::PRIVATE;
    my_keywords["protected"] = Token::PROTECTED;
    my_keywords["public"] = Token::PUBLIC;
    my_keywords["template"] = Token::TEMPLATE;
    my_keywords["this"] = Token::THIS;
    my_keywords["throw"] = Token::THROW;
    my_keywords["true"] = Token::Constant;
    my_keywords["try"] = Token::TRY;
    my_keywords["typeid"] = Token::TYPEID;
    my_keywords["typename"] = Token::TYPENAME;
    my_keywords["using"] = Token::USING;
    my_keywords["virtual"] = Token::VIRTUAL;
    my_keywords["wchar_t"] = Token::WCHAR;
  }
  if (tokenset & GCC)
  {
    my_keywords["__alignof__"] = Token::SIZEOF;
    my_keywords["__asm"] = Token::ATTRIBUTE;
    my_keywords["__asm__"] = Token::ATTRIBUTE;
    my_keywords["__attribute__"] = Token::ATTRIBUTE;
    my_keywords["__builtin_offsetof"] = Token::OFFSETOF;
    my_keywords["__builtin_va_arg"] = Token::EXTENSION; // Is this correct ?
    my_keywords["__complex__"] = Token::Ignore;
    my_keywords["__const"] = Token::CONST;
    my_keywords["__extension__"] = Token::EXTENSION;
    my_keywords["__imag__"] = Token::Ignore;
    my_keywords["__inline"] = Token::INLINE;
    my_keywords["__inline__"] = Token::INLINE;
    my_keywords["__real__"] = Token::Ignore;
    my_keywords["__restrict"] = Token::Ignore;
    my_keywords["__restrict__"] = Token::Ignore;
    my_keywords["__signed"] = Token::SIGNED;
    my_keywords["__signed__"] = Token::SIGNED;
    my_keywords["typeof"] = Token::TYPEOF;
    my_keywords["__typeof"] = Token::TYPEOF;
    my_keywords["__typeof__"] = Token::TYPEOF;
  }
  if (tokenset & MSVC)
  {
    my_keywords["cdecl"] = Token::Ignore;
    my_keywords["_cdecl"] = Token::Ignore;
    my_keywords["__cdecl"] = Token::Ignore;
    my_keywords["_fastcall"] = Token::Ignore;
    my_keywords["__fastcall"] = Token::Ignore;
    my_keywords["_stdcall"] = Token::Ignore;
    my_keywords["__stdcall"] = Token::Ignore;
    my_keywords["__thiscall"] = Token::Ignore;
    my_keywords["_based"] = Token::Ignore;
    my_keywords["__based"] = Token::Ignore;
    my_keywords["_asm"] = Token::ASM;
    my_keywords["__asm"] = Token::ASM;
    my_keywords["_inline"] = Token::INLINE;
    my_keywords["__inline"] = Token::INLINE;
    my_keywords["__declspec"] = Token::DECLSPEC;
    my_keywords["__pragma"] = Token::PRAGMA;
    my_keywords["__int8"] = Token::CHAR;
    my_keywords["__int16"] = Token::SHORT;
    my_keywords["__int32"] = Token::INT;
    my_keywords["__int64"] = Token::INT64;
    my_keywords["__w64"] = Token::Ignore;
  }
}

Token::Type Lexer::get_token(Token &t)
{
  if (!fill(1)) return Token::BadToken;
  t = my_tokens.front();
  my_tokens.pop();
  return t.type;
}

Token::Type Lexer::look_ahead(size_t offset)
{
  if (!fill(offset + 1)) return Token::BadToken;
  return my_tokens.at(offset).type;
}

Token::Type Lexer::look_ahead(size_t offset, Token &t)
{
  if (!fill(offset + 1)) return Token::BadToken;
  t = my_tokens.at(offset);
  return t.type;
}

const char *Lexer::save()
{
  if (!fill(1)) throw std::runtime_error("unexpected EOF");
  Token current = my_tokens.front();
  return current.ptr;
}

void Lexer::restore(const char *pos)
{
  my_token.type = '\n';
  my_token.ptr = my_buffer->ptr();
  my_token.length = 0;
  my_tokens.clear();
  rewind(pos);
}

unsigned long Lexer::origin(const char *ptr, std::string &filename) const
{
  return my_buffer->origin(ptr, filename);
}

void Lexer::rewind(const char *p)
{
  my_buffer->reset(p - my_buffer->ptr());
}

Token::Type Lexer::read_token(const char *&ptr, size_t &length)
{
  Token::Type t = Token::BadToken;
  while(true)
  {
    t = read_line();
    if(t == Token::Ignore) continue;
    my_token.type = t;

    if(t == Token::ATTRIBUTE)
    {
      skip_attribute();
      continue;
    }
    else if(t == Token::EXTENSION)
    {
      t = skip_extension(ptr, length);
      if(t == Token::Ignore) continue;
      else return t;
    }
    else if(t == Token::ASM)
    {
      skip_asm();
      continue;
    }
    else if(t == Token::DECLSPEC)
    {
      skip_declspec();
      continue;
    }
    else if(t == Token::PRAGMA)
    {
      skip_pragma();
      continue;
    }
    if(t != '\n') break;
  }

  ptr = my_token.ptr;
  length = my_token.length;
  return t;
}

bool Lexer::fill(size_t o)
{
  while (my_tokens.size() < o)
  {
    Token t;
    t.type = read_token(t.ptr, t.length);
    if (t.type == Token::BadToken) return false;
    my_tokens.push(t);
  }
  return true;
}

void Lexer::skip_attribute()
{
  char c;
  do { c = my_buffer->get();}
  while(c != '(' && c != '\0');
  if (c == '\0') return;
  skip_paren();
}

Token::Type Lexer::skip_extension(const char *&ptr, size_t &length)
{
  ptr = my_token.ptr;
  length = my_token.length;

  char c;
  do { c = my_buffer->get();}
  while(is_blank(c) || c == '\n');

#if 0
  // FIXME: Figure out under what circumstances we need to skip
  //        __extension__ (...)
  if(c != '(')
  {
    my_buffer->unget();
    return Token::Ignore; // if no (..) follows, ignore __extension__
  }
  skip_paren();
  return Token::Identifier; // regards it as the identifier __extension__
#else
  my_buffer->unget();
  return Token::Ignore; // if no (..) follows, ignore __extension__
#endif
}

inline bool check_end_of_instruction(Buffer *buffer, char c, const char *delimiter)
{
  if (c == '\0') return true;
  if (strchr(delimiter, c))
  {
    buffer->unget();
    return true;
  }
  return false;
}

void Lexer::skip_paren()
{
  size_t i = 1;
  do
  {
    char c = my_buffer->get();
    if (c == '\0') return;
    if(c == '(') ++i;
    else if(c == ')') --i;
  } while(i > 0);
}

void Lexer::skip_line()
{
  char c;
  do { c = my_buffer->get();}
  while(c != '\n' && c != '\0');
}

/* You can have the following :

   Just count the '{' and '}' and it should be ok
   __asm { mov ax,1
           mov bx,1 }

   Stop when EOL found. Note that the first ';' after
   an __asm instruction is an ASM comment !
   int v; __asm mov ax,1 __asm mov bx,1; v=1;

   Stop when '}' found
   if (cond) {__asm mov ax,1 __asm mov bx,1}

   and certainly more...
*/
void Lexer::skip_asm()
{
  char c;

  do
  {
    c = my_buffer->get();
    if (check_end_of_instruction(my_buffer, c, "")) return;
  }
  while(is_blank(c) || c == '\n');

  if(c == '{')
  {
    size_t i = 1;
    do
    {
      c = my_buffer->get();
      if (check_end_of_instruction(my_buffer, c, "")) return;
      if(c == '{') ++i;
      else if(c == '}') --i;
    } while(i > 0);
  }
  else
  {
    while(true)
    {
      if (check_end_of_instruction(my_buffer, c, "}\n")) return;
      c = my_buffer->get();
    }
  }
}

void Lexer::skip_declspec()
{
  char c;
  do
  {
    c = my_buffer->get();
    if (check_end_of_instruction(my_buffer, c, "")) return;
  } while(is_blank(c));

  if (c == '(')
  {
    size_t i = 1;
    do
    {
      c = my_buffer->get();
      if (check_end_of_instruction(my_buffer, c, "};")) return;
      if(c == '(') ++i;
      else if(c == ')') --i;
    } while(i > 0);
  }
}

void Lexer::skip_pragma()
{
  char c = get_next_non_white_char();

  if (c == '(')
  {
    size_t i = 1;
    do
    {
      c = my_buffer->get();
      if (check_end_of_instruction(my_buffer, c, "};")) return;
      if(c == '(') ++i;
      else if(c == ')') --i;
    } while(i > 0);

    c = get_next_non_white_char(); // assume ';'
  }
}

char Lexer::get_next_non_white_char()
{
  char c;
  while(true)
  {
    do { c = my_buffer->get();}
    while(is_blank(c));

    if(c != '\\') break;

    c = my_buffer->get();
    if(c != '\n' && c!= '\r') 
    {
      my_buffer->unget();
      break;
    }
  }
  return c;
}

Token::Type Lexer::read_line()
{
  char c = get_next_non_white_char();
  unsigned long top = my_buffer->position();
  my_token.ptr = my_buffer->ptr(top);
  if(c == '\0')
  {
    my_buffer->unget();
    return '\0';
  }
  else if(c == '\n') return '\n';
  else if(c == '#' && my_token.type == '\n')
  {
    skip_line();
    return '\n';
  }
  else if(c == '\'' || c == '"')
  {
    if(c == '\'')
    {
      if(read_char_const(top)) return Token::CharConst;
    }
    else
    {
      if(read_str_const(top)) return Token::StringL;
    }
    my_buffer->reset(top + 1);
    my_token.length = 1;
    return single_char_op(c);
  }
  else if(is_digit(c)) return read_number(c, top);
  else if(c == '.')
  {
    c = my_buffer->get();
    if(is_digit(c)) return read_float(top);
    else
    {
      my_buffer->unget();
      return read_separator('.', top);
    }
  }
  else if(is_letter(c))
  {
    if (c == 'L')
    {
      c = my_buffer->get();
      if (c == '\'' || c == '"')
      {
	if (c == '\'')
	{
	  if (read_char_const(top+1))
	  {
	    ++my_token.length;
	    return Token::WideCharConst;
	  }
	} 
	else
	{
	  if(read_str_const(top+1))
	  {
	    ++my_token.length;
	    return Token::WideStringL;
	  }
	}
      }
      my_buffer->reset(top);
    }
    return read_identifier(top);
  }
  else return read_separator(c, top);
}

bool Lexer::read_char_const(unsigned long top)
{
  while(true)
  {
    char c = my_buffer->get();
    if(c == '\\')
    {
      c = my_buffer->get();
      if(c == '\0') return false;
    }
    else if(c == '\'')
    {
      my_token.length = static_cast<size_t>(my_buffer->position() - top + 1);
      return true;
    }
    else if(c == '\n' || c == '\0') return false;
  }
}

/*
  If text is a sequence of string constants like:
	"string1" "string2"  L"string3"
  then the string constants are delt with as a single constant.
*/
bool Lexer::read_str_const(unsigned long top)
{
  // Skip the L if there is one
  if (my_buffer->at(top) == 'L') my_buffer->get();
  while(true)
  {
    char c = my_buffer->get();
    if(c == '\\')
    {
      c = my_buffer->get();
      if(c == '\0') return false;
    }
    else if(c == '"')
    {
      // We are past one string literal token now.
      // Any following whitespace needs to be skipped
      // before looking for anything else.
      unsigned long pos = my_buffer->position() + 1;
      while (true)
      {
	int nline = 0;
	// Consume whitespace.
	do
	{
	  c = my_buffer->get();
	  if(c == '\n') ++nline;
	} while(is_blank(c) || c == '\n');
	// Consume comment.
	if (c == '/')
	{
	  char d = my_buffer->get();
	  if (d == '/' || d == '*')
	    read_comment(d, my_buffer->position() - 2);
	  else
	  {
	    my_buffer->unget();
	    break;
	  }
	}
	else break;
      }
      if(c == '"')
	/* line_number += nline; */ ;
      else
      {
	my_token.length = static_cast<size_t>(pos - top);
	my_buffer->reset(pos);
	return true;
      }
    }
    else if(c == '\n' || c == '\0') return false;
  }
}

Token::Type Lexer::read_number(char c, unsigned long top)
{
  char c2 = my_buffer->get();

  if(c == '0' && is_xletter(c2))
  {
    do { c = my_buffer->get();}
    while(is_hexdigit(c));
    while(is_int_suffix(c)) c = my_buffer->get();

    my_buffer->unget();
    my_token.length = static_cast<size_t>(my_buffer->position() - top + 1);
    return Token::Constant;
  }

  while(is_digit(c2)) c2 = my_buffer->get();

  if(is_int_suffix(c2))
    do { c2 = my_buffer->get();}
    while(is_int_suffix(c2));
  else if(c2 == '.') return read_float(top);
  else if(is_eletter(c2))
  {
    my_buffer->unget();
    return read_float(top);
  }

  my_buffer->unget();
  my_token.length = static_cast<size_t>(my_buffer->position() - top + 1);
  return Token::Constant;
}

Token::Type Lexer::read_float(unsigned long top)
{
  char c;
    
  do { c = my_buffer->get();}
  while(is_digit(c));
  if(is_float_suffix(c))
    do { c = my_buffer->get();}
    while(is_float_suffix(c));
  else if(is_eletter(c))
  {
    unsigned long p = my_buffer->position();
    c = my_buffer->get();
    if(c == '+' || c == '-')
    {
      c = my_buffer->get();
      if(!is_digit(c))
      {
	my_buffer->reset(p);
	my_token.length = static_cast<size_t>(p - top);
	return Token::Constant;
      }
    }
    else if(!is_digit(c))
    {
      my_buffer->reset(p);
      my_token.length = static_cast<size_t>(p - top);
      return Token::Constant;
    }

    do { c = my_buffer->get();}
    while(is_digit(c));

    while(is_float_suffix(c)) c = my_buffer->get();
  }
  my_buffer->unget();
  my_token.length = static_cast<size_t>(my_buffer->position() - top + 1);
  return Token::Constant;
}

Token::Type Lexer::read_identifier(unsigned long top)
{
  char c;

  do { c = my_buffer->get();}
  while(is_letter(c) || is_digit(c));
  my_token.length = static_cast<size_t>(my_buffer->position() - top);
  my_buffer->unget();
  return screen(my_buffer->ptr(top), my_token.length);
}

Token::Type Lexer::screen(const char *identifier, size_t length)
{
  Dictionary::iterator i = my_keywords.find(std::string(identifier, length));
  if (i != my_keywords.end()) return i->second;
  return Token::Identifier;
}

Token::Type Lexer::read_separator(char c, unsigned long top)
{
  char c1 = my_buffer->get();
  if (c1 == '\0') return Token::BadToken;
  my_token.length = 2;
  if(c1 == '=')
  {
    switch(c)
    {
      case '*' :
      case '/' :
      case '%' :
      case '+' :
      case '-' :
      case '&' :
      case '^' :
      case '|' : return Token::AssignOp;
      case '=' :
      case '!' : return Token::EqualOp;
      case '<' :
      case '>' : return Token::RelOp;
      default : 
	my_buffer->unget();
	my_token.length = 1;
	return single_char_op(c);
    }
  }
  else if(c == c1)
  {
    switch(c)
    {
      case '<' :
      case '>' :
	if(my_buffer->get() != '=')
	{
	  my_buffer->unget();
	  return Token::ShiftOp;
	}
	else
	{
	  my_token.length = 3;
	  return Token::AssignOp;
	}
      case '|' : return Token::LogOrOp;
      case '&' : return Token::LogAndOp;
      case '+' :
      case '-' : return Token::IncOp;
      case ':' : return Token::Scope;
      case '.' :
	if(my_buffer->get() == '.')
	{
	  my_token.length = 3;
	  return Token::Ellipsis;
	}
	else my_buffer->unget();
      case '/' : return read_comment(c1, top);
      default :
	my_buffer->unget();
	my_token.length = 1;
	return single_char_op(c);
    }
  }
  else if(c == '.' && c1 == '*') return Token::PmOp;
  else if(c == '-' && c1 == '>')
    if(my_buffer->get() == '*')
    {
      my_token.length = 3;
      return Token::PmOp;
    }
    else
    {
      my_buffer->unget();
      return Token::ArrowOp;
    }
  else if(c == '/' && c1 == '*') return read_comment(c1, top);
  else
  {
    my_buffer->unget();
    my_token.length = 1;
    return single_char_op(c);
  }

  std::cerr << "*** An invalid character has been found! ("
	    << (int)c << ',' << (int)c1 << ")\n";
  return Token::BadToken;
}

Token::Type Lexer::single_char_op(unsigned char c)
{
  /* !"#$%&'()*+,-./0123456789:;<=>? */
  static char valid[] = "x   xx xxxxxxxx          xxxxxx";

  if('!' <= c && c <= '?' && valid[c - '!'] == 'x') return c;
  else if(c == '[' || c == ']' || c == '^') return c;
  else if('{' <= c && c <= '~') return c;
  else if(c == '#') 
  {
    // Skip to end of line
    do{ c = my_buffer->get();}
    while(c != '\n' && c != '\0');
    return Token::Ignore;
  }
  else
  {
    std::cerr << "*** An invalid character has been found! ("<<(char)c<<")"<< std::endl;
    return Token::BadToken;
  }
}

Token::Type Lexer::read_comment(char c, unsigned long top)
{
  unsigned long len = 0;
  if (c == '*')	// a nested C-style comment is prohibited.
    do 
    {
      c = my_buffer->get();
      if (c == '*')
      {
	c = my_buffer->get();
	if (c == '/')
	{
	  len = 1;
	  break;
	}
	else my_buffer->unget();
      }
    } while(c != '\0');
  else /* if (c == '/') */
    do { c = my_buffer->get();}
    while(c != '\n' && c != '\0');

  len += my_buffer->position() - top;
  my_token.length = static_cast<size_t>(len);
  my_comments.push_back(Token(my_buffer->ptr(top), my_token.length, Token::Comment));
  return Token::Ignore;
}

Lexer::Comments Lexer::get_comments()
{
  Comments c = my_comments;
  my_comments.clear();
  return c;
}

