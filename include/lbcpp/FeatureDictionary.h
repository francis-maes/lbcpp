/*-----------------------------------------.---------------------------------.
| Filename: FeatureDictionary.h            | A dictionary of feature and     |
| Author  : Francis Maes                   | feature-scope names             |
| Started : 06/03/2009 17:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_FEATURE_DICTIONARY_H_
# define LBCPP_FEATURE_DICTIONARY_H_

# include "ObjectPredeclarations.h"
# include <map>

namespace lbcpp
{

class StringDictionary : public Object
{
public:
  void clear()
    {stringToIndex.clear(); indexToString.clear();}

  size_t getNumElements() const
    {return (unsigned)indexToString.size();}

  bool exists(size_t index) const;
  std::string getString(size_t index) const;
  
  // returns -1 if not found
  int getIndex(const std::string& str) const;
  
  size_t add(const std::string& str);

  friend std::ostream& operator <<(std::ostream& ostr, const StringDictionary& strings);

  /*
  ** Object
  */
  virtual std::string toString() const
    {std::ostringstream ostr; ostr << *this; return ostr.str();}
  virtual TablePtr toTable() const;

  virtual void save(std::ostream& ostr) const;
  virtual bool load(std::istream& istr);  

protected:
  typedef std::map<std::string, size_t> StringToIndexMap;
  typedef std::vector<std::string> StringVector;
 
  StringToIndexMap stringToIndex;
  StringVector indexToString;
};

typedef ReferenceCountedObjectPtr<StringDictionary> StringDictionaryPtr;

class FeatureDictionary : public Object
{
public:
  FeatureDictionary(const std::string& name, StringDictionaryPtr features, StringDictionaryPtr scopes);
  FeatureDictionary(const std::string& name = "unnamed");
    
  bool empty() const
    {return getNumFeatures() == 0 && getNumScopes() == 0;}
  
  bool checkEquals(FeatureDictionaryPtr otherDictionary) const;

  /*
  ** Features
  */
  StringDictionaryPtr getFeatures()
    {return featuresDictionary;}
  
  size_t getNumFeatures() const
    {return featuresDictionary ? featuresDictionary->getNumElements() : 0;}
    
  size_t addFeature(const std::string& identifier)
    {assert(featuresDictionary); return featuresDictionary->add(identifier);}
    
  /*
  ** Scopes
  */
  StringDictionaryPtr getScopes()
    {return scopesDictionary;}

  size_t getNumScopes() const
    {return scopesDictionary ? scopesDictionary->getNumElements() : 0;}
    
  void addScope(const std::string& name, FeatureDictionaryPtr subDictionary)
    {ensureSubDictionary(getScopes()->add(name), subDictionary);}
  
  /*
  ** Related dictionaries
  */
  size_t getNumSubDictionaries() const
    {return subDictionaries.size();}
    
  void setSubDictionary(size_t index, FeatureDictionaryPtr dictionary)
    {if (subDictionaries.size() < index + 1) subDictionaries.resize(index + 1); subDictionaries[index] = dictionary;}
    
  void setSubDictionaries(const std::vector<FeatureDictionaryPtr>& subDictionaries)
    {this->subDictionaries = subDictionaries;}
    
  const FeatureDictionaryPtr getSubDictionary(size_t index) const
    {assert(index < subDictionaries.size()); return subDictionaries[index];}
    
  FeatureDictionaryPtr getSubDictionary(size_t index);
  
  FeatureDictionaryPtr getSubDictionary(const std::string& name)
    {assert(scopesDictionary); return getSubDictionary(scopesDictionary->getIndex(name));}

  void ensureSubDictionary(size_t index, FeatureDictionaryPtr subDictionary);

  FeatureDictionaryPtr getDictionaryWithSubScopesAsFeatures();
  
  /*
  ** Object
  */
  virtual std::string getName() const
    {return name;}
    
  virtual std::string toString() const;
  virtual ObjectGraphPtr toGraph() const;
  virtual TablePtr toTable() const;

  virtual bool load(std::istream& istr);
  virtual void save(std::ostream& ostr) const;
  
  static bool load(std::istream& istr, FeatureDictionaryPtr& dictionary1, FeatureDictionaryPtr& dictionary2);
  static void save(std::ostream& ostr, const FeatureDictionaryPtr dictionary1, const FeatureDictionaryPtr dictionary2);
  
private:
  std::string name;
  StringDictionaryPtr featuresDictionary;
  StringDictionaryPtr scopesDictionary;
  std::vector<FeatureDictionaryPtr> subDictionaries;
  FeatureDictionaryPtr dictionaryWithSubScopesAsFeatures;

  void toStringRec(size_t indent, std::string& res) const;
};

inline FeatureDictionaryPtr loadFeatureDictionary(const std::string& filename)
  {return Object::loadFromFileAndCast<FeatureDictionary>(filename);}


}; /* namespace lbcpp */

#endif // !LBCPP_STRING_DICTIONARY_H_
