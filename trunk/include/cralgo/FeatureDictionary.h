/*-----------------------------------------.---------------------------------.
| Filename: FeatureDictionary.h            | A dictionary of feature and     |
| Author  : Francis Maes                   | feature-scope names             |
| Started : 06/03/2009 17:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_FEATURE_DICTIONARY_H_
# define CRALGO_FEATURE_DICTIONARY_H_

# include "ObjectPredeclarations.h"
# include <map>

namespace cralgo
{

class StringDictionary
{
public:
  void clear()
    {stringToIndex.clear(); indexToString.clear();}

  size_t count() const
    {return (unsigned)indexToString.size();}

  bool exists(size_t index) const;
  std::string getString(size_t index) const;
  
  // returns -1 if not found
  int getIndex(const std::string& str) const;
  
  size_t add(const std::string& str);

  friend std::ostream& operator <<(std::ostream& ostr, const StringDictionary& strings);

protected:
  typedef std::map<std::string, size_t> StringToIndexMap;
  typedef std::vector<std::string> StringVector;
 
  StringToIndexMap stringToIndex;
  StringVector indexToString;
};

class FeatureDictionary : public Object
{
public:
  FeatureDictionary(const std::string& name);
  FeatureDictionary() {}
  ~FeatureDictionary()
    {clear();}
    
  void clear();
  
  bool empty() const
    {return featuresDictionary.count() == 0 && scopesDictionary.count() == 0;}
  
  /*
  ** Features
  */
  StringDictionary& getFeatures()
    {return featuresDictionary;}
  
  size_t getNumFeatures() const
    {return featuresDictionary.count();}
    
  /*
  ** Scopes
  */
  StringDictionary& getScopes()
    {return scopesDictionary;}

  size_t getNumScopes() const
    {return scopesDictionary.count();}
    
  const FeatureDictionaryPtr getSubDictionary(size_t index) const
  {
    assert(index < subDictionaries.size());
    return subDictionaries[index];
  }
    
  FeatureDictionaryPtr getSubDictionary(size_t index, FeatureDictionaryPtr defaultValue = FeatureDictionaryPtr());
  
  FeatureDictionaryPtr getSubDictionary(const std::string& name, FeatureDictionaryPtr defaultValue = FeatureDictionaryPtr())
    {return getSubDictionary(scopesDictionary.getIndex(name), defaultValue);}
  
  /*
  ** Object
  */
  virtual std::string getName() const
    {return name;}
    
  virtual std::string toString() const
  {
    return "Features: " + cralgo::toString(featuresDictionary) + "\n"
           "Scopes: " + cralgo::toString(scopesDictionary) + "\n";
  }
  
private:
  std::string name;
  StringDictionary featuresDictionary;
  StringDictionary scopesDictionary;
  std::vector<FeatureDictionaryPtr> subDictionaries;
};

template<>
struct Traits<FeatureDictionaryPtr> : public ObjectPtrTraits<FeatureDictionary> {};

}; /* namespace cralgo */

#endif // !CRALGO_STRING_DICTIONARY_H_
