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

/*!
**@file   Traits.h
**@author Francis MAES
**@date   Sat Jun 13 17:59:46 2009
**
**@brief  Type traits
**
**
*/

#ifndef LBCPP_TRAITS_H_
# define LBCPP_TRAITS_H_


# include <string>
# include <sstream>
# include <vector>
# include <set>
# include <typeinfo>
# include <cmath>
# include <assert.h>

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

  /*!
  **
  **
  ** @param value
  **
  ** @return
  */
  static inline std::string toString(const T& value)
    {std::ostringstream ostr; ostr << value; return ostr.str();}

  /*!
  **
  **
  ** @param ostr
  ** @param value
  */
  static inline void write(std::ostream& ostr, const T& value)
    {assert(false);}

  /*!
  **
  **
  ** @param istr
  ** @param result
  **
  ** @return
  */
  static inline bool read(std::istream& istr, T& result)
    {assert(false); return false;}
};


/*!
**
**
** @param value
**
** @return
*/
template<class T>
inline std::string toString(const T& value)
  {return Traits<T>::toString(value);}

/*!
**
**
** @param ostr
** @param value
*/
template<class T>
inline void write(std::ostream& ostr, const T& value)
  {Traits<T>::write(ostr, value);}

/*!
**
**
** @param istr
** @param result
**
** @return
*/
template<class T>
inline bool read(std::istream& istr, T& result)
  {return Traits<T>::read(istr, result);}

/*
** Builtin-type traits
*/
/*!
** @struct BuiltinTypeTraits
** @brief
*/
template<class T>
struct BuiltinTypeTraits
{
  typedef T Type;

  /*!
  **
  **
  ** @param value
  **
  ** @return
  */
  static inline std::string toString(const T& value)
// if this fails, you need to declare a "friend std::ostream& operator << (std::ostream& ostr, const T& value)" operator
    {std::ostringstream ostr; ostr << value; return ostr.str();}

  /*!
  **
  **
  ** @param ostr
  ** @param value
  */
  static inline void write(std::ostream& ostr, const T& value)
    {ostr.write((const char* )&value, sizeof (T));}

  /*!
  **
  **
  ** @param istr
  ** @param result
  **
  ** @return
  */
  static inline bool read(std::istream& istr, T& result)
    {istr.read((char* )&result, sizeof (T)); return istr.good();}
};


template<>
struct Traits<size_t> : public BuiltinTypeTraits<size_t> {};
template<>
struct Traits<int> : public BuiltinTypeTraits<int> {};
template<>
struct Traits<float> : public BuiltinTypeTraits<float> {};
template<>
struct Traits<double> : public BuiltinTypeTraits<double> {};

/*!
**
**
** @param number
**
** @return
*/
inline bool isNumberValid(double number)
{
#ifdef WIN32
    return number == number;
#else
    return !std::isnan(number) && !std::isinf(number);
#endif
}

/*!
**
**
** @param value
**
** @return
*/
inline bool isNumberNearlyNull(double value)
{
  static const double epsilon = 0.00001;
  return fabs(value) < epsilon;
}


/*
** Specialized builtin-type traits
*/
template<>
struct Traits<bool>
{
  typedef bool Type;

  static inline std::string toString(bool value)
    {return value ? "true" : "false";}

  static inline void write(std::ostream& ostr, bool value)
    {ostr.put((char)(value ? 1 : 0));}

  static inline bool read(std::istream& istr, bool& res)
  {
    char c = istr.get();
    if (istr.good() && (c == 0 || c <= 1))
    {
      res = c != 0;
      return true;
    }
    return false;
  }
};

template<>
struct Traits<char>
{
  typedef char Type;

  static inline std::string toString(char value)
    {return std::string("'") + value + "'";} // todo: improve

  static inline void write(std::ostream& ostr, char value)
    {ostr.put(value);}

  static inline bool read(std::istream& istr, char& res)
    {res = istr.get(); return istr.good();}
};

template<>
struct Traits<std::string>
{
  typedef std::string Type;

  static inline std::string toString(const std::string& value)
    {return "\"" + value + "\"";} // todo: improve

  static inline void write(std::ostream& ostr, const std::string& string)
    {ostr.write(string.c_str(), string.size() + 1);}

  static inline bool read(std::istream& istr, std::string& res)
  {
    res = "";
    char c = istr.get();
    while (istr.good())
    {
      if (!c)
        return true;
      res += c;
      c = istr.get();
    }
    return false; // stream failed before we read the terminal '0'
  }
};

/*
** Pointer Traits
*/
template<class TargetType>
struct Traits<const TargetType*>
{
  typedef const TargetType* Type;

  static inline std::string toString(const TargetType* value)
    {return value ? "&" + Traits<TargetType>::toString(*value) : "null";}

  static inline void write(std::ostream& ostr, const TargetType* value)
  {
    Traits<bool>::write(ostr, value != NULL);
    if (value)
      Traits<TargetType>::write(ostr, *value);
  }

  static inline bool read(std::istream& istr, TargetType*& result)
  {
    bool exists;
    if (!Traits<bool>::read(istr, exists))
      return false;
    if (exists)
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

  static inline std::string toString(const TargetType* value)
    {return value ? "&" + Traits<TargetType>::toString(*value) : "null";}

  static inline void write(std::ostream& ostr, const TargetType* value)
  {
    Traits<bool>::write(ostr, value != NULL);
    if (value)
      Traits<TargetType>::write(ostr, *value);
  }

  static inline bool read(std::istream& istr, TargetType*& result)
  {
    bool exists;
    if (!Traits<bool>::read(istr, exists))
      return false;
    if (exists)
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

  /*!
  **
  **
  ** @param value
  **
  ** @return
  */
  static inline std::string toString(const T& value)
    {return "&" + lbcpp::toString(*value);}

  /*!
  **
  **
  ** @param ostr
  ** @param value
  */
  static inline void write(std::ostream& ostr, const T& value)
    {assert(false);}

  /*!
  **
  **
  ** @param istr
  ** @param result
  **
  ** @return
  */
  static inline bool read(std::istream& istr, T& result)
    {assert(false); return false;}
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

  static inline std::string toString(const std::type_info& info)
  {
    std::string res = info.name();
  #ifdef WIN32
    size_t n = res.find("::");
    return res.substr(n == std::string::npos ? strlen("class ") : n + 2);
  #else // linux or macos x
    bool hasNamespace = res[0] == 'N';
    if (hasNamespace)
      res = trimAlphaLeft(trimDigitsLeft(res, 1));
    res = trimDigitsLeft(res);
    if (hasNamespace)
      res = res.substr(0, res.length() - 1);
    return res;
  #endif
  }

  static inline void write(std::ostream& ostr, const std::type_info& value)
    {Traits<std::string>::write(ostr, toString(value));}

  static inline bool read(std::istream& istr, std::type_info& res)
    {assert(false); return false;}

  static inline bool read(std::istream& istr, std::string& res)
    {return Traits<std::string>::read(istr, res);}

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

