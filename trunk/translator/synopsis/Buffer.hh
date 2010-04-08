//
// Copyright (C) 1997-2000 Shigeru Chiba
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_Buffer_hh_
#define Synopsis_Buffer_hh_

#include <streambuf>
#include <vector>

namespace Synopsis
{

//. Buffer holds the memory on top of which a parse tree / syntax tree is
//. constructed. Besides giving access to individual characters, it provides
//. the means to register replacements for buffer chunks, such that when
//. the Buffer's write method is executed the new file will contain the
//. modified source.
class Buffer
{
public:
  Buffer(std::streambuf *, const std::string & = std::string("unknown"));

  //. return the size of the buffer
  unsigned long size() const { return my_buffer.size();}

  //. report the character at the current position and advance one character
  char get() { return my_cursor < my_buffer.size() ? my_buffer[my_cursor++] : '\0';}
  //. undo the last get
  void unget() { --my_cursor;}
  //. reset the current position to position c
  void reset(unsigned long c = 0) { my_cursor = c;}

  //. report the current position
  unsigned long position() const { return my_cursor - 1;}
  //. report the character at position p
  char at(unsigned long p) const { return my_buffer[p];}
  //. report the pointer at position p
  const char *ptr(unsigned long p = 0) const { return my_buffer.c_str() + p;}

  //. replace the text between from and to by the text between
  //. begin and begin + length
  void replace(const char *from, const char *to,
	       const char *begin, unsigned long length);


  bool has_origin(const char* ptr) const
    {return ptr >= my_buffer.data() && ptr < my_buffer.data() + my_buffer.size();}

  //. Return the origin of the given pointer (filename and line number)
  unsigned long origin(const char *, std::string &) const;
  //. Write the buffer into the given output stream
  //. The first line contains a line directive issuing the input file name;
  //. if filename is non-empty, use this to fake another one.
  void write(std::ostream &, const std::string &) const;

  // francis
  std::string filename() const
    {return my_filename;}
    
private:
  struct Replacement
  {
    Replacement(const char *from, const char *to,
		const char *begin, unsigned long length);
    static bool smaller(Replacement const &r1, Replacement const &r2)
    { return r1.from < r2.from;}
    const char   *from;
    const char   *to;
    const char   *begin;
    unsigned long length;
  };
  typedef std::vector<Replacement> Replacements;

  //. read a line directive starting at position pos, and return
  //. the line number found. Also report the begin and end of the filename
  //. (with respect to the internal buffer).
  //. line is the default line number that gets reported on error (in
  //. which case begin and end remain unchanged)
  long read_line_directive(unsigned long cursor, long line,
			   unsigned long &begin, unsigned long &end) const;

  std::string   my_filename;
  std::string   my_buffer;
  unsigned long my_cursor;
  Replacements  my_replacements;
};

}

#endif
