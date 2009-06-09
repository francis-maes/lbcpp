/*-----------------------------------------.---------------------------------.
| Filename: FeatureDictionary.cpp          | Feature dictionary              |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 19:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/FeatureDictionary.h>
using namespace lbcpp;

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
    return lbcpp::toString(index);
  else
    return indexToString[index];
}

void StringDictionary::save(std::ostream& ostr) const
{
  write(ostr, indexToString);
}

bool StringDictionary::load(std::istream& istr)
{
  clear();
  if (!read(istr, indexToString))
    return false;
  for (size_t i = 0; i < indexToString.size(); ++i)
    stringToIndex[indexToString[i]] = i;
  return true;
}


namespace lbcpp
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

}; /* namespace lbcpp */

/*
** FeatureDictionary
*/
FeatureDictionary::FeatureDictionary(const std::string& name, StringDictionaryPtr features, StringDictionaryPtr scopes)
  : name(name), featuresDictionary(features), scopesDictionary(scopes)
{
//  std::cout << "New FeatureDictionary '" << name << "'" << std::endl;
}

FeatureDictionary::FeatureDictionary(const std::string& name)
  : name(name), featuresDictionary(new StringDictionary()), scopesDictionary(new StringDictionary())
{
//  std::cout << "New FeatureDictionary '" << name << "'" << std::endl;
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
      (scopesDictionary->exists(index) ? scopesDictionary->getString(index) : lbcpp::toString(index)));
  return res;
}

FeatureDictionaryPtr FeatureDictionary::getDictionaryWithSubScopesAsFeatures()
{
  if (!dictionaryWithSubScopesAsFeatures)
    dictionaryWithSubScopesAsFeatures = new FeatureDictionary(name, scopesDictionary, StringDictionaryPtr());
  return dictionaryWithSubScopesAsFeatures;
}

void FeatureDictionary::enumerateUniqueDictionaries(std::map<FeatureDictionaryPtr, size_t>& indices, std::vector<FeatureDictionaryPtr>& res)
{
  if (indices.find(this) == indices.end())
  {
    indices[this] = indices.size();
    res.push_back(this);
  }
  for (size_t i = 0; i < getNumScopes(); ++i)
  {
    FeatureDictionaryPtr subDictionary = getSubDictionary(i);
    if (subDictionary)
      subDictionary->enumerateUniqueDictionaries(indices, res);
  }
}

bool FeatureDictionary::load(std::istream& istr)
{
  size_t numUniqueDictionaries;
  if (!read(istr, numUniqueDictionaries))
    return false;
  assert(numUniqueDictionaries > 0);

  std::vector< FeatureDictionaryPtr > dictionaries(numUniqueDictionaries);
  std::vector< std::vector<size_t> > subDictionaryIndices(numUniqueDictionaries);

  for (size_t i = 0; i < dictionaries.size(); ++i)
  {
    std::string name;
    if (!read(istr, name))
      return false;
    FeatureDictionaryPtr dictionary = new FeatureDictionary(name);
    if (!dictionary->getFeatures()->load(istr) ||
        !dictionary->getScopes()->load(istr))
      return false;
    dictionaries[i] = dictionary;
    subDictionaryIndices[i].resize(dictionary->getNumScopes());
    for (size_t j = 0; j < dictionary->getNumScopes(); ++j)
    {
      size_t subDictionaryIndex;
      if (!read(istr, subDictionaryIndex))
        return false;
      if (subDictionaryIndex >= dictionaries.size())
      {
        Object::error("FeatureDictionary::load", "Invalid sub-dictionary index");
        return false;
      }
      subDictionaryIndices[i][j] = subDictionaryIndex;
    }
  }
  
  for (size_t i = 0; i < dictionaries.size(); ++i)
  {
    FeatureDictionaryPtr dictionary = dictionaries[i];
    for (size_t j = 0; j < subDictionaryIndices[i].size(); ++j)
      dictionary->setSubDictionary(j, dictionaries[subDictionaryIndices[i][j]]);
  }
  FeatureDictionaryPtr topDictionary = dictionaries[0];
  assert(topDictionary);
  name = topDictionary->name;
  featuresDictionary = topDictionary->featuresDictionary;
  scopesDictionary = topDictionary->scopesDictionary;
  subDictionaries = topDictionary->subDictionaries;
  return true;
}

void FeatureDictionary::save(std::ostream& ostr)
{
  std::map<FeatureDictionaryPtr, size_t> indices;
  std::vector<FeatureDictionaryPtr> dictionaries;

  enumerateUniqueDictionaries(indices, dictionaries);
  assert(dictionaries.size() > 0);
  write(ostr, dictionaries.size());
  for (size_t i = 0; i < dictionaries.size(); ++i)
  {
    FeatureDictionaryPtr dictionary = dictionaries[i];
    write(ostr, dictionary->getName());
    dictionary->getFeatures()->save(ostr);
    dictionary->getScopes()->save(ostr);
    for (size_t j = 0; j < dictionary->getNumScopes(); ++j)
    {
      std::map<FeatureDictionaryPtr, size_t>::iterator it = indices.find(dictionary->getSubDictionary(j));
      assert(it != indices.end());
      write(ostr, it->second);
    }
  }
}
