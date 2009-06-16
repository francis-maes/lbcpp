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
**@brief  #FIXME: all
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
** @brief
*/

class StringDictionary : public Object
{
public:
  /*!
  **
  **
  */
  void clear()
    {stringToIndex.clear(); indexToString.clear();}

  /*!
  **
  **
  **
  ** @return
  */
  size_t getNumElements() const
    {return (unsigned)indexToString.size();}

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  bool exists(size_t index) const;
  std::string getString(size_t index) const;

  // returns -1 if not found
  /*!
  **
  **
  ** @param str
  **
  ** @return
  */
  int getIndex(const std::string& str) const;

  /*!
  **
  **
  ** @param str
  **
  ** @return
  */
  size_t add(const std::string& str);

  /*!
  **
  **
  ** @param ostr
  ** @param strings
  **
  ** @return
  */
  friend std::ostream& operator <<(std::ostream& ostr, const StringDictionary& strings);

  /*
  ** Object
  */
  /*!
  **
  **
  **
  ** @return
  */
  virtual std::string toString() const
    {std::ostringstream ostr; ostr << *this; return ostr.str();}

  /*!
  **
  **
  **
  ** @return
  */
  virtual TablePtr toTable() const;

  /*!
  **
  **
  ** @param ostr
  */
  virtual void save(std::ostream& ostr) const;

  /*!
  **
  **
  ** @param istr
  **
  ** @return
  */
  virtual bool load(std::istream& istr);

protected:
  typedef std::map<std::string, size_t> StringToIndexMap;
  typedef std::vector<std::string> StringVector;

  StringToIndexMap stringToIndex; /*!< */
  StringVector indexToString;   /*!< */
};

typedef ReferenceCountedObjectPtr<StringDictionary> StringDictionaryPtr;


/*!
** @class FeatureDictionary
** @brief
*/
class FeatureDictionary : public Object
{
public:
  /*!
  **
  **
  ** @param name
  ** @param features
  ** @param scopes
  **
  ** @return
  */
  FeatureDictionary(const std::string& name, StringDictionaryPtr features, StringDictionaryPtr scopes);
  /*!
  **
  **
  ** @param name
  **
  ** @return
  */
  FeatureDictionary(const std::string& name = "unnamed");

  /*!
  **
  **
  **
  ** @return
  */
  bool empty() const
    {return getNumFeatures() == 0 && getNumScopes() == 0;}

  /*!
  **
  **
  ** @param otherDictionary
  **
  ** @return
  */
  bool checkEquals(FeatureDictionaryPtr otherDictionary) const;

  /*
  ** Features
  */
  /*!
  **
  **
  **
  ** @return
  */
  StringDictionaryPtr getFeatures()
    {return featuresDictionary;}

  /*!
  **
  **
  **
  ** @return
  */
  size_t getNumFeatures() const
    {return featuresDictionary ? featuresDictionary->getNumElements() : 0;}

  /*!
  **
  **
  ** @param identifier
  **
  ** @return
  */
  size_t addFeature(const std::string& identifier)
    {assert(featuresDictionary); return featuresDictionary->add(identifier);}

  /*
  ** Scopes
  */
  /*!
  **
  **
  **
  ** @return
  */
  StringDictionaryPtr getScopes()
    {return scopesDictionary;}

  /*!
  **
  **
  **
  ** @return
  */
  size_t getNumScopes() const
    {return scopesDictionary ? scopesDictionary->getNumElements() : 0;}

  /*!
  **
  **
  ** @param name
  ** @param subDictionary
  */
  void addScope(const std::string& name, FeatureDictionaryPtr subDictionary)
    {ensureSubDictionary(getScopes()->add(name), subDictionary);}

  /*
  ** Related dictionaries
  */
  /*!
  **
  **
  **
  ** @return
  */
  size_t getNumSubDictionaries() const
    {return subDictionaries.size();}

  /*!
  **
  **
  ** @param index
  ** @param dictionary
  */
  void setSubDictionary(size_t index, FeatureDictionaryPtr dictionary)
    {if (subDictionaries.size() < index + 1) subDictionaries.resize(index + 1); subDictionaries[index] = dictionary;}

  /*!
  **
  **
  ** @param subDictionaries
  */
  void setSubDictionaries(const std::vector<FeatureDictionaryPtr>& subDictionaries)
    {this->subDictionaries = subDictionaries;}

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  const FeatureDictionaryPtr getSubDictionary(size_t index) const
    {assert(index < subDictionaries.size()); return subDictionaries[index];}

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  FeatureDictionaryPtr getSubDictionary(size_t index);

  /*!
  **
  **
  ** @param name
  **
  ** @return
  */
  FeatureDictionaryPtr getSubDictionary(const std::string& name)
    {assert(scopesDictionary); return getSubDictionary(scopesDictionary->getIndex(name));}

  /*!
  **
  **
  ** @param index
  ** @param subDictionary
  */
  void ensureSubDictionary(size_t index, FeatureDictionaryPtr subDictionary);

  /*!
  **
  **
  **
  ** @return
  */
  FeatureDictionaryPtr getDictionaryWithSubScopesAsFeatures();

  /*
  ** Object
  */
  /*!
  **
  **
  **
  ** @return
  */
  virtual std::string getName() const
    {return name;}

  /*!
  **
  **
  **
  ** @return
  */
  virtual std::string toString() const;
  /*!
  **
  **
  **
  ** @return
  */
  virtual ObjectGraphPtr toGraph() const;
  /*!
  **
  **
  **
  ** @return
  */
  virtual TablePtr toTable() const;

  /*!
  **
  **
  ** @param istr
  **
  ** @return
  */
  virtual bool load(std::istream& istr);
  /*!
  **
  **
  ** @param ostr
  */
  virtual void save(std::ostream& ostr) const;

  /*!
  **
  **
  ** @param istr
  ** @param dictionary1
  ** @param dictionary2
  **
  ** @return
  */
  static bool load(std::istream& istr, FeatureDictionaryPtr& dictionary1, FeatureDictionaryPtr& dictionary2);
  /*!
  **
  **
  ** @param ostr
  ** @param dictionary1
  ** @param dictionary2
  */
  static void save(std::ostream& ostr, const FeatureDictionaryPtr dictionary1, const FeatureDictionaryPtr dictionary2);

private:
  std::string name;             /*!< */
  StringDictionaryPtr featuresDictionary; /*!< */
  StringDictionaryPtr scopesDictionary; /*!< */
  std::vector<FeatureDictionaryPtr> subDictionaries; /*!< */
  FeatureDictionaryPtr dictionaryWithSubScopesAsFeatures; /*!< */

  /*!
  **
  **
  ** @param indent
  ** @param res
  */
  void toStringRec(size_t indent, std::string& res) const;
};

/*!
**
**
** @param filename
**
** @return
*/
inline FeatureDictionaryPtr loadFeatureDictionary(const std::string& filename)
  {return Object::loadFromFileAndCast<FeatureDictionary>(filename);}


}; /* namespace lbcpp */

#endif // !LBCPP_STRING_DICTIONARY_H_
