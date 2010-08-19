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
| Filename: FeatureDictionary.h            | A dictionary of feature and     |
| Author  : Francis Maes                   | feature-scope names             |
| Started : 06/03/2009 17:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_DICTIONARY_H_
# define LBCPP_FEATURE_DICTIONARY_H_

# include "StringDictionary.h"

namespace lbcpp
{

/**
** @class FeatureDictionary
** @brief Feature dictionary.
**
** A FeatureDictionary is a structure that keeps a link between
** feature indices and their names. So, a FeatureDictionary is the
** semantic of a particular type of data (inherited from
** FeatureGenerator). The two main purpose of a FeatureDictionary are:
** - feature vector semantic.
** - feature vector type comparison (a type of feature vector shares
** its FeatureDictionary with all feature vector of the same type).
*
** @see FeatureGenerator
*/
class FeatureDictionary : public NameableObject
{
public:
  /**
  ** Constructor.
  **
  ** @param name : dictionary name.
  ** @param features : feature string dictionary.
  ** @param scopes : scope string dictionary.
  ** @return a FeatureDictionary instance.
  */
  FeatureDictionary(const String& name, StringDictionaryPtr features, StringDictionaryPtr scopes);

  /**
  ** Constructor.
  **
  ** @param name : dictionary name.
  ** @return a FeatureDicitonary instance.
  */
  FeatureDictionary(const String& name = "unnamed");

  /**
  ** Checks if the dictionary is empty or not.
  **
  ** @return True if there is no entry in the dictionary.
  */
  bool empty() const
    {return getNumFeatures() == 0 && getNumScopes() == 0;}

  /**
  ** Checks if two feature dictionaries are equals.
  **
  ** @param otherDictionary : feature dictionary pointer.
  ** @return True if the two feature dictionaries are equals.
  */
  bool checkEquals(FeatureDictionaryPtr otherDictionary) const;

  /*
  ** Features
  */
  /**
  ** Features getter.
  **
  ** @return a feature string dictionary pointer.
  */
  StringDictionaryPtr getFeatures()
    {return featuresDictionary;}

  void setFeatures(StringDictionaryPtr features)
    {featuresDictionary = features;}

  /**
  ** Returns the number of features.
  **
  ** @return the number of features.
  */
  size_t getNumFeatures() const
    {return featuresDictionary ? featuresDictionary->getNumElements() : 0;}

  String getFeature(size_t index) const
    {return featuresDictionary ? featuresDictionary->getString(index) : String::empty;}

  /**
  ** Adds a new feature.
  **
  ** @param identifier : feature identifier.
  ** @return index of the new feature.
  */
  size_t addFeature(const String& identifier)
    {jassert(featuresDictionary); return featuresDictionary->add(identifier);}

  /*
  ** Scopes
  */
  /**
  ** Scopes getter.
  **
  ** @return a scope string dictionary pointer.
  */
  StringDictionaryPtr getScopes()
    {return scopesDictionary;}

  void setScopes(StringDictionaryPtr scopes)
    {scopesDictionary = scopes;}

  /**
  ** Returns the number of scopes.
  **
  ** @return the number of scopes.
  */
  size_t getNumScopes() const
    {return scopesDictionary ? scopesDictionary->getNumElements() : 0;}

  String getScope(size_t index) const
    {return scopesDictionary ? scopesDictionary->getString(index) : String::empty;}

  /**
  ** Adds a new scope.
  **
  ** @param name : scope name.
  ** @param subDictionary : subdictionary.
  */
  void addScope(const String& name, FeatureDictionaryPtr subDictionary)
    {ensureSubDictionary(getScopes()->add(name), subDictionary);}

  /*
  ** Related dictionaries
  */
  /**
  ** Returns the number of subdictionaries.
  **
  ** @return the number of subdictionaries.
  */
  size_t getNumSubDictionaries() const
    {return subDictionaries.size();}

  /**
  ** Subdictionary setter.
  **
  ** @param index : subdictionary index.
  ** @param dictionary : subdictionary.
  */
  void setSubDictionary(size_t index, FeatureDictionaryPtr dictionary)
    {if (subDictionaries.size() < index + 1) subDictionaries.resize(index + 1); subDictionaries[index] = dictionary;}

  /**
  ** Subdictionaries setter.
  **
  ** @param subDictionaries : subdictionaries.
  */
  void setSubDictionaries(const std::vector<FeatureDictionaryPtr>& subDictionaries)
    {this->subDictionaries = subDictionaries;}

  /**
  ** Subdictionary getter by index (read only).
  **
  ** @param index : index of the subdictionary.
  ** @return the corresponding subdictionary or throw an error.
  */
  const FeatureDictionaryPtr getSubDictionary(size_t index) const
    {jassert(index < subDictionaries.size()); return subDictionaries[index];}

  /**
  ** Subdictionary getter by index.
  **
  ** @param index : index of the subdictionary.
  ** @return the corresponding subdictionary or an empty dictionary if
  ** there is no correspondance with @a index.
  */
  FeatureDictionaryPtr getSubDictionary(size_t index);

  /**
  ** Subdictionary getter by name.
  **
  ** @param name : subdictionary name.
  ** @return the corresponding subdictionary or throw an error.
  */
  FeatureDictionaryPtr getSubDictionary(const String& name);

  /**
  ** Ensures that you don't try to override a subdictionary.
  ** @a ensureSubDictionary() adds @a subDictionary at the index @a
  ** index if there is nothing at that place. Otherwise it checks if
  ** @a subDictionary and subDictionaries[@a index] are equals or
  ** not. It throws an error to the ErrorHandler if you try to override
  ** an existing (and different) subdictionary.
  **
  ** @param index : index of the subdictionary.
  ** @param subDictionary : subdictionary to insert or check.
  ** @see ErrorHandler
  */
  void ensureSubDictionary(size_t index, FeatureDictionaryPtr subDictionary);

  /**
  ** Converts to a string.
  **
  ** @return a string corresponding to the dictionary.
  */
  virtual String toString() const;

  /**
  ** Converts to an ObjectGraph
  **
  ** @return an object graph pointer corresponding to the dictionary.
  ** @see ObjectGraph
  */
  virtual ObjectGraphPtr toGraph() const;

  /**
  ** Converts to a Table.
  **
  ** @return a table pointer corresponding to the dictionary.
  ** @see Table
  */
  virtual TablePtr toTable() const;

  /**
  ** Loads a dictionary from a stream.
  **
  ** @param istr : input stream.
  ** @return False if any error occurs.
  */
  virtual bool load(InputStream& istr);

  /**
  ** Saves to an output stream.
  **
  ** @param ostr : output stream.
  */
  virtual void save(OutputStream& ostr) const;

  /**
  ** Loads a FeatureDictionary graph with exactly two roots into two
  ** distinct feature dictionariers called @a dictionary1 and @a
  ** dictionary2.
  **
  ** @param istr : input stream.
  ** @param dictionary1 : first dictionary container.
  ** @param dictionary2 : second dictionary container.
  **
  ** @return False if any error occurs.
  ** @see ObjectGraph
  */
  static bool load(InputStream& istr, FeatureDictionaryPtr& dictionary1, FeatureDictionaryPtr& dictionary2);

  /**
  ** Saves @a dictionary1 and @a dictionary2 to @a ostr stream (as
  ** ObjectGraph).
  **
  ** @param ostr : output stream
  ** @param dictionary1 : first dictionary.
  ** @param dictionary2 : second dictionary.
  ** @see ObjectGraph
  */
  static void save(OutputStream& ostr, const FeatureDictionaryPtr dictionary1, const FeatureDictionaryPtr dictionary2);

private:
  StringDictionaryPtr featuresDictionary; /*!< Feature dictionary. */
  StringDictionaryPtr scopesDictionary; /*!< Scope dictionary. */
  std::vector<FeatureDictionaryPtr> subDictionaries; /*!< Subdictionaries. */

  void toStringRec(size_t indent, String& res) const;
};

class FeatureDictionaryManager
{
public:
  static FeatureDictionaryManager& getInstance();

  static FeatureDictionaryPtr get(const String& name)
    {return getInstance().getOrCreateDictionary(name);}

  void addDictionary(FeatureDictionaryPtr dictionary);

  bool hasRootDictionary(const String& name) const
    {return getRootDictionary(name) != FeatureDictionaryPtr();}

  FeatureDictionaryPtr readDictionaryNameAndGet(InputStream& istr);

  FeatureDictionaryPtr getRootDictionary(const String& name) const;
  FeatureDictionaryPtr getOrCreateRootDictionary(const String& name, bool createEmptyFeatures = true, bool createEmptyScopes = true);
  FeatureDictionaryPtr getOrCreateDictionary(const String& name);

  FeatureDictionaryPtr getFlatVectorDictionary(StringDictionaryPtr indices);
  FeatureDictionaryPtr getCollectionDictionary(StringDictionaryPtr indices, FeatureDictionaryPtr elementsDictionary);

private:
  typedef std::map<String, FeatureDictionaryPtr> DictionariesMap;
  DictionariesMap dictionaries;
};

#define LBCPP_DECLARE_DICTIONARY(ClassName) \
  lbcpp::FeatureDictionaryManager::getInstance().addDictionary(ClassName::getInstance())

class BinaryClassificationDictionary : public FeatureDictionary
{
public:
  static FeatureDictionaryPtr getInstance();

  enum Type
  {
    negative = 0,
    positive,
  };

private:
  BinaryClassificationDictionary();
};

}; /* namespace lbcpp */

#endif // !LBCPP_STRING_DICTIONARY_H_
