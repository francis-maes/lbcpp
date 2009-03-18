/*-----------------------------------------.---------------------------------.
| Filename: FeatureDictionary.cpp          | Feature dictionary              |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 19:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/FeatureDictionary.h>
using namespace cralgo;

/*
** StringDictionary
*/
bool StringDictionary::exists(size_t index) const
{
  return index < indexToString.size();
}

int StringDictionary::getIndex(const std::string& str) const
{
  StringToIndexMap::const_iterator it = stringToIndex.find(str);
  if (it == stringToIndex.end())
    return -1;
  else
    return (size_t)it->second;
}

size_t StringDictionary::add(const std::string& str)
{
  StringToIndexMap::iterator it = stringToIndex.find(str);
  if (it == stringToIndex.end())
  {
    size_t res = indexToString.size();
    indexToString.push_back(str);
    stringToIndex[str] = res;
    assert(stringToIndex.find(str) != stringToIndex.end());
    return res;
  }
  else
    return it->second;
}

std::string StringDictionary::getString(size_t index) const
{
  if (index >= indexToString.size() || indexToString[index] == "")
    return cralgo::toString(index);
  else
    return indexToString[index];
}

namespace cralgo
{

  std::ostream& operator <<(std::ostream& ostr, const StringDictionary& strings)
  {
    for (size_t i = 0; i < strings.indexToString.size(); ++i)
    {
      ostr << strings.indexToString[i] << " [" << i << "]";
      if (i < strings.indexToString.size() - 1)
        ostr << ", ";
    }
    return ostr;
  }

}; /* namespace cralgo */

/*
** FeatureDictionary
*/
FeatureDictionary::FeatureDictionary(const std::string& name) : name(name)
{
}

void FeatureDictionary::clear()
{
  featuresDictionary.clear();
  scopesDictionary.clear();
  subDictionaries.clear();
}
  
FeatureDictionaryPtr FeatureDictionary::getSubDictionary(size_t index, FeatureDictionaryPtr defaultValue)
{
  if (subDictionaries.size() < index + 1)
    subDictionaries.resize(index + 1);
  FeatureDictionaryPtr& res = subDictionaries[index];
  if (res)
  {
    if (defaultValue && res != defaultValue)
    {
      std::cerr << "Error: sub-dictionary mismatch. This dictionary = '" << res->name << "', required dictionary = '" << defaultValue << "'" << std::endl;
      assert(false);
    }
  }
  else
    res = defaultValue ? defaultValue : new FeatureDictionary(name + "." + 
      (scopesDictionary.exists(index) ? scopesDictionary.getString(index) : cralgo::toString(index)));
  return res;
}
