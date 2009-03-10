/*-----------------------------------------.---------------------------------.
| Filename: FeatureDictionary.h            | A dictionary of feature and     |
| Author  : Francis Maes                   | feature-scope names             |
| Started : 06/03/2009 17:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_FEATURE_DICTIONARY_H_
# define CRALGO_FEATURE_DICTIONARY_H_

# include "StringDictionary.h"
# include "Traits.h"

namespace cralgo
{

class FeatureDictionary
{
public:
  FeatureDictionary(const std::string& name);
  ~FeatureDictionary()
    {clear();}
    
  void clear();
  
  bool empty() const
    {return (!featuresDictionary || featuresDictionary->count() == 0) && 
          (!scopesDictionary || scopesDictionary->count() == 0);}
  
  void ensureHasFeatures()
    {if (!featuresDictionary) featuresDictionary = new StringDictionary();}
  
  StringDictionary& getFeatures()
    {ensureHasFeatures(); return *featuresDictionary;}
    
  void ensureHasScopes()
    {if (!scopesDictionary) scopesDictionary = new StringDictionary();}

  StringDictionary& getScopes()
    {ensureHasScopes(); return *scopesDictionary;}
    
  const FeatureDictionary& getSubDictionary(size_t index) const
  {
    assert(subDictionaries && index < subDictionaries->size());
    return *(*subDictionaries)[index];
  }
    
  FeatureDictionary& getSubDictionary(size_t index);
  
  FeatureDictionary& getSubDictionary(const std::string& name)
    {assert(scopesDictionary); return getSubDictionary(scopesDictionary->getIndex(name));}
  
  std::string getName() const
    {return name;}
    
  friend std::ostream& operator <<(std::ostream& ostr, const FeatureDictionary& dictionary)
  {
    if (dictionary.featuresDictionary)
      ostr << "Features: " << cralgo::toString(*dictionary.featuresDictionary) << std::endl;
    if (dictionary.scopesDictionary)
      ostr << "Scopes: " << cralgo::toString(*dictionary.scopesDictionary) << std::endl;
    return ostr;
  }
  
private:
  std::string name;
  StringDictionary* featuresDictionary;
  StringDictionary* scopesDictionary;
  std::vector<FeatureDictionary* >* subDictionaries;
};

}; /* namespace cralgo */

#endif // !CRALGO_STRING_DICTIONARY_H_
