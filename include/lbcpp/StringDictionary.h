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
| Filename: StringDictionary.h             | A dictionary of strings         |
| Author  : Francis Maes                   |                                 |
| Started : 07/04/2010 19:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_STRING_DICTIONARY_H_
# define LBCPP_STRING_DICTIONARY_H_

# include "ObjectPredeclarations.h"
# include <map>

namespace lbcpp
{

/*!
** @class StringDictionary
** @brief String dictionary.
*/
class StringDictionary : public NameableObject
{
public:
  StringDictionary(const StringDictionary& otherDictionary);
  StringDictionary(const String& name, const juce::tchar* strings[]);
  StringDictionary(const String& name) : NameableObject(name) {}
  StringDictionary() {}

  /**
  ** Clears dictionary. Removes all pairs <index, string>.
  **
  */
  void clear()
    {stringToIndex.clear(); indexToString.clear();}

  /**
  ** Returns the number of elements (number of pairs <index,
  ** string>).
  **
  ** @return the number of elements.
  */
  size_t getNumElements() const
    {return (unsigned)indexToString.size();}

  /**
  ** Checks if it exists an association for the index @a index.
  **
  ** @param index : index to check.
  ** @return False if there is no association with @a index.
  */
  bool exists(size_t index) const;

  /**
  ** Returns the string associated to @a index.
  **
  ** @param index : key.
  ** @return the string associated to @a index if any, or convert
  ** @a index to string.
  */
  String getString(size_t index) const;

  /**
  ** Returns the index associated to @a str.
  **
  ** @param str : string value used as key.
  ** @return -1 if not found, the corresponding index otherwise.
  */
  int getIndex(const String& str) const;

  /**
  ** Adds a string value to the dictionary if it not already exists
  **
  ** @param str : string value.
  ** @return the corresponding index value.
  */
  size_t add(const String& str);

  /**
  ** Converts a string dictionary to a stream.
  **
  ** Puts a string dictionary to an output stream following this form:
  ** [indexToString[0], ..., indexToString[indexToString.size()-1]]
  **
  ** @param ostr : output stream.
  ** @param strings : string dictionary.
  ** @return a stream instance.
  */
  friend std::ostream& operator <<(std::ostream& ostr, const StringDictionary& strings);

  /*
  ** Object
  */
  /**
  ** Converts a string dictionary to a string.
  **
  ** Puts a string dictionary to a string following this form:
  ** [indexToString[0], ..., indexToString[indexToString.size()-1]]
  **
  ** @return string value.
  */
  virtual String toString() const
    {std::ostringstream ostr; ostr << *this; return ostr.str().c_str();}

  /**
  ** Converts a string dictionary to a Table.
  ** @return a table pointer.
  ** @see Table
  */
  virtual TablePtr toTable() const;

  /**
  ** Saves a string dictionary to an output stream.
  **
  ** @param ostr : output stream.
  */
  virtual void save(OutputStream& ostr) const;

  /**
  ** Loads a string dictionary from an input stream.
  **
  ** @param istr : input stream.
  ** @return False if any error occurs.
  */
  virtual bool load(InputStream& istr);

  void writeIdentifier(OutputStream& ostr, size_t index);
  bool readIdentifier(InputStream& istr, size_t& index);

protected:
  typedef std::map<String, size_t> StringToIndexMap;
  typedef std::vector<String> StringVector;

  StringToIndexMap stringToIndex; /**< String to index correspondance. */
  StringVector indexToString;   /**< Index to string correspondance. */
};

typedef ReferenceCountedObjectPtr<StringDictionary> StringDictionaryPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_STRING_DICTIONARY_H_
