//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_Lexer_hh_
#define Synopsis_Lexer_hh_

#include <synopsis/Token.hh>
#include <vector>
#include <map>
#include <deque>
#include <stdexcept>

namespace Synopsis
{

class Buffer;

//. a Lexer reads tokens from a stream.
class Lexer
{
public:
  //. Define sets of token that are to be recognized as special
  //. keywords (as opposed to identifiers). They can be or'ed.
  //. If CXX is not specified, the Lexer will operate in 'C mode'.
  enum TokenSet { C=0x0, CXX=0x01, GCC=0x02, MSVC=0x04};
  typedef std::vector<Token> Comments;

  struct InvalidChar : std::runtime_error
  {
    InvalidChar(const std::string &msg) : std::runtime_error(msg) {}
  };

  //. Construct a Lexer on the given Buffer using the given
  //. token set. The default token set is CXX with GCC extensions.
  Lexer(Buffer *, int tokenset = CXX|GCC);
  Token::Type get_token(Token &);
  Token::Type look_ahead(size_t);
  Token::Type look_ahead(size_t, Token &);

  const char *save();
  void restore(const char *);

  Comments get_comments();

  //. Return the origin of the given pointer
  //. (filename and line number)
  unsigned long origin(const char *, std::string &) const;

// francis: protected instead of private
protected:
  //. a Queue is used to read in tokens from a stream
  //. without consuming them
  class Queue
  {
  public:
    typedef std::deque<Token> Container;
    typedef Container::size_type size_type;

    bool empty() const { return my_container.empty();}
    size_type size() const { return my_container.size();}
    const Token &front() const { return my_container.front();}
    const Token &back() const { return my_container.back();}
    const Token &at(size_type i) const { return my_container.at(i);}
    void push(const Token &t) { my_container.push_back(t);}
    void pop() { my_container.pop_front();}
    void clear() { my_container.clear();}
  private:
    Container my_container;
  };
  typedef std::map<std::string, Token::Type> Dictionary;

  void rewind(const char *);

  Token::Type read_token(const char *&, size_t &);
  //. try to fill the token cache to contain
  //. at least o tokens. Returns false if
  //. there are not enough tokens.
  bool fill(size_t o);

  //. skip till end of paren
  void skip_paren();
  //. skip till end of line
  void skip_line();
  //. skip __attribute__(...), ___asm__(...), ...
  void skip_attribute();
  //. skip __extension__(...).
  Token::Type skip_extension(const char *&, size_t &);
  //. skip __asm ...
  void skip_asm();
  //. skip __declspec(...).
  void skip_declspec();
  //. skip __pragma(...);.
  void skip_pragma();

  char get_next_non_white_char();
  Token::Type read_line();
  bool read_char_const(unsigned long top);
  bool read_str_const(unsigned long top);
  Token::Type read_number(char c, unsigned long top);
  Token::Type read_float(unsigned long top);
  Token::Type read_identifier(unsigned long top);
  Token::Type screen(const char *identifier, size_t len);
  Token::Type read_separator(char c, unsigned long top);
  Token::Type single_char_op(unsigned char c);
  Token::Type read_comment(char c, unsigned long top);

  Buffer    *my_buffer;
  Queue      my_tokens;
  Dictionary my_keywords;
  Token      my_token;
  Comments   my_comments;
};

inline bool is_blank(char c)
{
  return c == ' ' || c == '\t' || c == '\f' || c == '\r';
}

inline bool is_letter(char c)
{
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || (c == '_' || c == '$');
}

inline bool is_digit(char c){ return '0' <= c && c <= '9';}

inline bool is_xletter(char c){ return c == 'X' || c == 'x';}

inline bool is_eletter(char c){ return c == 'E' || c == 'e';}

inline bool is_hexdigit(char c)
{
  return is_digit(c) || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
}

inline bool is_int_suffix(char c)
{
  return c == 'U' || c == 'u' || c == 'L' || c == 'l';
}

inline bool is_float_suffix(char c)
{
  return c == 'F' || c == 'f' || c == 'L' || c == 'l';
}

}

#endif
