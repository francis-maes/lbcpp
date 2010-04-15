/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: Traits.h                       | C++ Type Traits                 |
| Author  : Francis Maes                   |                                 |
| Started : 06/03/2009 17:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_TRAITS_H_
# define LBCPP_TRAITS_H_

# include "common.h"

namespace lbcpp
{

template<bool> struct StaticAssert;
template<> struct StaticAssert<true> {};

/*!
** @struct Traits
** @brief
*/
template<class T>
struct Traits
{
public:
  typedef T Type;

  static inline String toString(const T& value)
    {jassert(false); return T("<toString not implemented>");}

  static inline void write(OutputStream& ostr, const T& value)
    {jassert(false);}

  static inline bool read(InputStream& istr, T& result)
    {jassert(false); return false;}
};

/*
** Power guru functions
*/
template<class T>
inline String toString(const T& value)
  {return Traits<T>::toString(value);}

template<class T>
inline void write(OutputStream& ostr, const T& value)
  {Traits<T>::write(ostr, value);}

template<class T>
inline bool read(InputStream& istr, T& result)
  {return Traits<T>::read(istr, result);}

inline std::ostream& operator <<(std::ostream& ostr, const String& value)
  {return ostr << (const char* )value;}

inline bool isNumberValid(double number)
{
#ifdef JUCE_WIN32
    return number == number;
#else
    return !std::isnan(number) && !std::isinf(number);
#endif
}

inline bool isNumberNearlyNull(double value, double epsilon = 0.00001)
  {return fabs(value) < epsilon;}


/*
** Specialized builtin-type traits
*/
template<>
struct Traits<bool>
{
  typedef bool Type;

  static inline String toString(bool value)
    {return value ? T("true") : T("false");}

  static inline void write(OutputStream& ostr, bool value)
    {ostr.writeBool(value);}

  static inline bool read(InputStream& istr, bool& res)
  {
    if (istr.isExhausted())
      return false;
    res = istr.readBool();
    return true;
  }
};

template<>
struct Traits<char>
{
  typedef char Type;

  static inline String toString(char value)
    {String res; res += value; return res.quoted();}

  static inline void write(OutputStream& ostr, char value)
    {ostr.writeByte(value);}

  static inline bool read(InputStream& istr, char& res)
  {
    if (istr.isExhausted())
      return false;
    res = istr.readByte();
    return true;
  }
};


template<>
struct Traits<unsigned char>
{
  typedef unsigned char Type;

  static inline String toString(unsigned char value)
    {String res((int)value); return res;}

  static inline void write(OutputStream& ostr, unsigned char value)
    {ostr.writeByte((char)value);}

  static inline bool read(InputStream& istr, unsigned char& res)
  {
    if (istr.isExhausted())
      return false;
    res = (unsigned char)istr.readByte();
    return true;
  }
};

template<>
struct Traits<String>
{
  typedef String Type;

  static inline String toString(const String& value)
    {return value.quoted();}

  static inline void write(OutputStream& ostr, const String& string)
    {ostr.writeString(string);}

  static inline bool read(InputStream& istr, String& res)
  {
    if (istr.isExhausted())
      return false;
    res = istr.readString();
    return true;
  }
};

template<>
struct Traits<size_t>
{
  typedef size_t Type;
  
  static inline String toString(const Type& value)
    {return String((juce::int64)value);}

  static inline void write(OutputStream& ostr, const Type& value)
    {ostr.writeInt64BigEndian((juce::int64)value);}

  static inline bool read(InputStream& istr, Type& res)
  {
    if (istr.isExhausted())
      return false;
    res = (size_t)istr.readInt64BigEndian();
    return true;
  }
};

template<>
struct Traits<int>
{
  typedef int Type;
  
  static inline String toString(const Type& value)
    {return String(value);}

  static inline void write(OutputStream& ostr, const Type& value)
    {ostr.writeIntBigEndian(value);}

  static inline bool read(InputStream& istr, Type& res)
  {
    if (istr.isExhausted())
      return false;
    res = istr.readIntBigEndian();
    return true;
  }  
};

template<>
struct Traits<float>
{
  typedef float Type;
  
  static inline String toString(const Type& value)
    {return String(value);}

  static inline void write(OutputStream& ostr, const Type& value)
    {ostr.writeFloatBigEndian(value);}

  static inline bool read(InputStream& istr, Type& res)
  {
    if (istr.isExhausted())
      return false;
    res = istr.readFloatBigEndian();
    return true;
  }
};

template<>
struct Traits<double>
{
  typedef double Type;
  
  static inline String toString(const Type& value)
    {return String(value);}

  static inline void write(OutputStream& ostr, const Type& value)
    {ostr.writeDoubleBigEndian(value);}

  static inline bool read(InputStream& istr, Type& res)
  {
    if (istr.isExhausted())
      return false;
    res = istr.readDoubleBigEndian();
    return true;
  }
};



/*
** Pointer Traits
*/
template<class TargetType>
struct Traits<const TargetType*>
{
  typedef const TargetType* Type;

  static inline String toString(const TargetType* value)
    {return value ? T("&") + Traits<TargetType>::toString(*value) : T("null");}

  static inline void write(OutputStream& ostr, const TargetType* value)
  {
    ostr.writeBool(value != NULL);
    if (value)
      Traits<TargetType>::write(ostr, *value);
  }

  static inline bool read(InputStream& istr, TargetType*& result)
  {
    if (istr.readBool())
    {
      result = new TargetType();
      return Traits<TargetType>::read(istr, *result);
    }
    result = NULL;
    return true;
  }
};

template<class TargetType>
struct Traits<TargetType*>
{
  typedef TargetType* Type;

  static inline String toString(const TargetType* value)
    {return value ? T("&") + Traits<TargetType>::toString(*value) : T("null");}

  static inline void write(OutputStream& ostr, const TargetType* value)
  {
    ostr.writeBool(value != NULL);
    if (value)
      Traits<TargetType>::write(ostr, *value);
  }

  static inline bool read(InputStream& istr, TargetType*& result)
  {
    if (istr.readBool())
    {
      result = new TargetType();
      return Traits<TargetType>::read(istr, *result);
    }
    result = NULL;
    return true;
  }
};

/*
** @struct IteratorTraits
** @brief Iterator Traits
*/
template<class T>
struct IteratorTraits
{
public:
  typedef T Type;

  static inline String toString(const T& value)
    {return T("&") + lbcpp::toString(*value);}

  static inline void write(OutputStream& ostr, const T& value)
    {jassert(false);}

  static inline bool read(InputStream& istr, T& result)
    {jassert(false); return false;}
};

template<>
struct Traits<std::set<size_t>::iterator>
  : public IteratorTraits< std::set<size_t>::iterator> {};

/*template<class T>
struct Traits< typename std::vector<T>::iterator >
  : public IteratorTraits< typename std::vector<T>::iterator > {};

template<class T>
struct Traits< typename std::vector<T>::const_iterator >
  : public IteratorTraits< typename std::vector<T>::const_iterator > {};

template<class T>
struct Traits< typename std::set<T>::iterator >
  : public IteratorTraits< typename std::set<T>::iterator > {};

template<class T>
struct Traits< typename std::set<T>::const_iterator >
  : public IteratorTraits< typename std::set<T>::const_iterator > {};
*/

/*
** Typeinfo Traits
*/
template<>
struct Traits<std::type_info>
{
  typedef std::type_info Type;

  static inline String toString(const std::type_info& info)
  {
    std::string res = info.name();
  #ifdef JUCE_WIN32
    size_t n = res.find("::");
    return res.substr(n == std::string::npos ? strlen("class ") : n + 2).c_str();
  #else // linux or macos x
    bool hasNamespace = res[0] == 'N';
    if (hasNamespace)
      res = trimAlphaLeft(trimDigitsLeft(res, 1));
    res = trimDigitsLeft(res);
    if (hasNamespace)
      res = res.substr(0, res.length() - 1);
    return res.c_str();
  #endif
  }

  static inline void write(OutputStream& ostr, const std::type_info& value)
    {ostr.writeString(toString(value));}

  static inline bool read(InputStream& istr, std::type_info& res)
    {jassert(false); return false;}

  static inline bool read(InputStream& istr, String& res)
    {return Traits<String>::read(istr, res);}

private:
  static std::string trimDigitsLeft(const std::string& str, size_t i = 0)
  {
    for (; i < str.length() && isdigit(str[i]); ++i);
    return str.substr(i);
  }

  static std::string trimAlphaLeft(const std::string& str, size_t i = 0)
  {
    for (; i < str.length() && isalpha(str[i]); ++i);
    return str.substr(i);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_TRAITS_H_

