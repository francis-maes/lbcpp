/*-----------------------------------------.---------------------------------.
| Filename: FeatureDictionary.h            | A dictionary of feature and     |
| Author  : Francis Maes                   | feature-scope names             |
| Started : 06/03/2009 17:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   FeatureDictionary.h
**@author Francis MAES
**@date   Tue Jun 16 08:27:18 2009
**
**@brief  Dictionaries of features declarations.
**
**
*/

#ifndef LBCPP_FEATURE_DICTIONARY_H_
# define LBCPP_FEATURE_DICTIONARY_H_

# include "ObjectPredeclarations.h"
# include <map>

namespace lbcpp
{

/*!
** @class StringDictionary
** @brief String dictionary.
*/

class StringDictionary : public Object
{
public:
  /*!
  ** Clears dictionary. Removes all pairs <index, string>.
  **
  */
  void clear()
    {stringToIndex.clear(); indexToString.clear();}

  /*!
  ** Returns the number of elements (number of pairs <index,
  ** string>).
  **
  ** @return the number of elements.
  */
  size_t getNumElements() const
    {return (unsigned)indexToString.size();}

  /*!
  ** Checks if it exists an association for the index @a index.
  **
  ** @param index : index to check.
  **
  ** @return False if there is no association with @a index.
  */
  bool exists(size_t index) const;

  /*!
  ** Returns the string associated to @a index.
  **
  ** @param index : key.
  **
  ** @return the string associated to @a index if any, or convert
  ** @a index to string.
  */
  std::string getString(size_t index) const;

  /*!
  ** Returns the index associated to @a str.
  **
  ** @param str : string value used as key.
  **
  ** @return -1 if not found, the corresponding index otherwise.
  */
  int getIndex(const std::string& str) const;

  /*!
  ** Adds a new string value to the dictionary.
  **
  ** @param str : string value.
  **
  ** @return the corresponding index value.
  */
  size_t add(const std::string& str);

  /*!
  ** Converts a string dictionary to a stream.
  **
  ** Puts a string dictionary to an output stream following this form:
  ** [indexToString[0], ..., indexToString[indexToString.size()-1]]
  **
  ** @param ostr : output stream.
  ** @param strings : string dictionary.
  **
  ** @return a stream instance.
  */
  friend std::ostream& operator <<(std::ostream& ostr, const StringDictionary& strings);

  /*
  ** Object
  */
  /*!
  ** Converts a string dictionary to a string.
  **
  ** Puts a string dictionary to a string following this form:
  ** [indexToString[0], ..., indexToString[indexToString.size()-1]]
  **
  ** @return string value.
  */
  virtual std::string toString() const
    {std::ostringstream ostr; ostr << *this; return ostr.str();}

  /*!
  ** Converts a string dictionary to a Table.
  ** @see Table
  ** @return a table pointer.
  */
  virtual TablePtr toTable() const;

  /*!
  ** Saves a string dictionary to an output stream.
  **
  ** @param ostr : output stream.
  */
  virtual void save(std::ostream& ostr) const;

  /*!
  ** Loads a string dictionary from an input stream.
  **
  ** @param istr : input stream.
  **
  ** @return False if any error occurs.
  */
  virtual bool load(std::istream& istr);

protected:
  typedef std::map<std::string, size_t> StringToIndexMap;
  typedef std::vector<std::string> StringVector;

  StringToIndexMap stringToIndex; /*!< String to index correspondance. */
  StringVector indexToString;   /*!< Index to string correspondance. */
};

typedef ReferenceCountedObjectPtr<StringDictionary> StringDictionaryPtr;


/*!
** @class FeatureDictionary
** @brief Feature dictionary.
**
** A FeatureDictionary is a structure that keeps a link between
** feature indexes and their names. So, a FeatureDictionary is the
** semantic of a particular type of data (inherited from
** FeatureGenerator). The two main purpose of a FeatureDictionary are:
** - feature vector semantic.
** - feature vector type comparison (a type of feature vector shares
** its FeatureDictionary with all feature vector of the same type).
*
** @see FeatureGenerator
*/
class FeatureDictionary : public Object
{
public:
  /*!
  ** Constructor.
  **
  ** @param name : dictionary name.
  ** @param features : feature string dictionary.
  ** @param scopes : scope string dicitonary.
  **
  ** @return a FeatureDictionary instance.
  */
  FeatureDictionary(const std::string& name, StringDictionaryPtr features, StringDictionaryPtr scopes);

  /*!
  ** Constructor.
  **
  ** @param name : dictionary name.
  **
  ** @return a FeatureDicitonary instance.
  */
  FeatureDictionary(const std::string& name = "unnamed");

  /*!
  ** Checks if the dictionary is empty or not.
  **
  ** @return True if there is no entry in the dictionary.
  */
  bool empty() const
    {return getNumFeatures() == 0 && getNumScopes() == 0;}

  /*!
  ** Checks if two feature dictionaries are equals.
  **
  ** @param otherDictionary : feature dictionary pointer.
  **
  ** @return True if the two feature dictionaries are equals.
  */
  bool checkEquals(FeatureDictionaryPtr otherDictionary) const;

  /*
  ** Features
  */
  /*!
  ** Features getter.
  **
  ** @return a feature string dictionary pointer.
  */
  StringDictionaryPtr getFeatures()
    {return featuresDictionary;}

  /*!
  ** Returns the number of features.
  **
  ** @return the number of features.
  */
  size_t getNumFeatures() const
    {return featuresDictionary ? featuresDictionary->getNumElements() : 0;}

  /*!
  ** Adds a new feature.
  **
  ** @param identifier : feature identifier.
  **
  ** @return index of the new feature.
  */
  size_t addFeature(const std::string& identifier)
    {assert(featuresDictionary); return featuresDictionary->add(identifier);}

  /*
  ** Scopes
  */
  /*!
  ** Scopes getter.
  **
  ** @return a scope string dictionary pointer.
  */
  StringDictionaryPtr getScopes()
    {return scopesDictionary;}

  /*!
  ** Returns the number of scopes.
  **
  ** @return the number of scopes.
  */
  size_t getNumScopes() const
    {return scopesDictionary ? scopesDictionary->getNumElements() : 0;}

  /*!
  ** Adds a new scope.
  **
  ** @param name : scope name.
  ** @param subDictionary : subdictionary.
  */
  void addScope(const std::string& name, FeatureDictionaryPtr subDictionary)
    {ensureSubDictionary(getScopes()->add(name), subDictionary);}

  /*
  ** Related dictionaries
  */
  /*!
  ** Returns the number of subdictionaries.
  **
  ** @return the number of subdictionaries.
  */
  size_t getNumSubDictionaries() const
    {return subDictionaries.size();}

  /*!
  ** Subdictionary setter.
  **
  ** @param index : subdictionary index.
  ** @param dictionary : subdictionary.
  */
  void setSubDictionary(size_t index, FeatureDictionaryPtr dictionary)
    {if (subDictionaries.size() < index + 1) subDictionaries.resize(index + 1); subDictionaries[index] = dictionary;}

  /*!
  ** Subdictionaries setter.
  **
  ** @param subDictionaries : subdictionaries.
  */
  void setSubDictionaries(const std::vector<FeatureDictionaryPtr>& subDictionaries)
    {this->subDictionaries = subDictionaries;}

  /*!
  ** Subdictionary getter by index (read only).
  **
  ** @param index : index of the subdictionary.
  **
  ** @return the corresponding subdictionary or throw an error.
  */
  const FeatureDictionaryPtr getSubDictionary(size_t index) const
    {assert(index < subDictionaries.size()); return subDictionaries[index];}

  /*!
  ** Subdictionary getter by index.
  **
  ** @param index : index of the subdictionary.
  **
  ** @return the corresponding subdictionary or an empty dictionary if
  ** there is no correspondance with @a index.
  */
  FeatureDictionaryPtr getSubDictionary(size_t index);

  /*!
  ** Subdictionary getter by name.
  **
  ** @param name : subdictionary name.
  **
  ** @return the corresponding subdictionary or throw an error.
  */
  FeatureDictionaryPtr getSubDictionary(const std::string& name)
    {assert(scopesDictionary); return getSubDictionary(scopesDictionary->getIndex(name));}

  /*!
  ** Ensures that you don't try to overide a subdictionary.
  ** @a ensureSubDictionary() adds @a subDictionary at the index @a
  ** index if there is nothing at that place. Otherwise it checks if
  ** @a subDictionary and subDictionaries[@a index] are equals or
  ** not. It throws an error to the ErrorHandler if you try to overide
  ** an existing (and different) subdictionary.
  **
  ** @see ErrorHandler
  ** @param index : index of the subdictionary.
  ** @param subDictionary : subdictionary to insert or check.
  */
  void ensureSubDictionary(size_t index, FeatureDictionaryPtr subDictionary);

  /*!
  ** Returns a FeatureDictionary with subscopes as features.
  **
  ** @return a FeatureDictionary with subscopes as features.
  */
  FeatureDictionaryPtr getDictionaryWithSubScopesAsFeatures();

  /*
  ** Object
  */
  /*!
  ** Dictionary name getter.
  **
  ** @return dictionary name.
  */
  virtual std::string getName() const
    {return name;}

  /*!
  ** Converts to a string.
  **
  ** @return a string corresponding to the dictionary.
  */
  virtual std::string toString() const;

  /*!
  ** Converts to an ObjectGraph.
  ** @see ObjectGraph
  ** @return an object graph pointer corresponding to the dictionary.
  */
  virtual ObjectGraphPtr toGraph() const;

  /*!
  ** Converts to a Table.
  ** @see Table
  ** @return a table pointer corresponding to the dictionary.
  */
  virtual TablePtr toTable() const;

  /*!
  ** Loads a dictionary from a stream.
  **
  ** @param istr : input stream.
  **
  ** @return False if any error occurs.
  */
  virtual bool load(std::istream& istr);

  /*!
  ** Saves to an output stream.
  **
  ** @param ostr : output stream.
  */
  virtual void save(std::ostream& ostr) const;

  /*!
  ** Loads a FeatureDictionary graph with exactly two roots into two
  ** distinct feature dictionariers called @a dictionary1 and @a
  ** dictionary2.
  **
  ** @see ObjectGraph
  ** @param istr : input stream.
  ** @param dictionary1 : first dictionary container.
  ** @param dictionary2 : second dictionary container.
  **
  ** @return False if any error occurs.
  */
  static bool load(std::istream& istr, FeatureDictionaryPtr& dictionary1, FeatureDictionaryPtr& dictionary2);

  /*!
  ** Saves @a dictionary1 and @a dictionary2 to @a ostr stream (as
  ** ObjectGraph).
  **
  ** @see ObjectGraph
  ** @param ostr : output stream
  ** @param dictionary1 : first dictionary.
  ** @param dictionary2 : second dictionary.
  */
  static void save(std::ostream& ostr, const FeatureDictionaryPtr dictionary1, const FeatureDictionaryPtr dictionary2);

private:
  std::string name;             /*!< Dictionary name. */
  StringDictionaryPtr featuresDictionary; /*!< Feature dictionary. */
  StringDictionaryPtr scopesDictionary; /*!< Scope dictionary. */
  std::vector<FeatureDictionaryPtr> subDictionaries; /*!< Subdictionaries. */
  FeatureDictionaryPtr dictionaryWithSubScopesAsFeatures;

  void toStringRec(size_t indent, std::string& res) const;
};

/*!
** Loads a feature dictionary from a file.
**
** @param filename : file name.
**
** @return a feature dictionary pointer.
*/
inline FeatureDictionaryPtr loadFeatureDictionary(const std::string& filename)
  {return Object::loadFromFileAndCast<FeatureDictionary>(filename);}


}; /* namespace lbcpp */

#endif // !LBCPP_STRING_DICTIONARY_H_
