/*-----------------------------------------.---------------------------------.
| Filename: FeatureDictionary.cpp          | Feature dictionary              |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 19:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/FeatureDictionary.h>
#include <lbcpp/ObjectGraph.h>
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
    ostr << "[";
    for (size_t i = 0; i < strings.indexToString.size(); ++i)
    {
      ostr << strings.indexToString[i]; // << " [" << i << "]";
      if (i < strings.indexToString.size() - 1)
        ostr << ", ";
    }
    ostr << "]";
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

bool FeatureDictionary::checkEquals(FeatureDictionaryPtr otherDictionary) const
{
  FeatureDictionaryPtr dictionary = const_cast<FeatureDictionary* >(this);
  if (dictionary != otherDictionary)
  {
    Object::error("FeatureGenerator::checkDictionaryEquals", 
                  "Dictionary mismatch. This dictionary = '" + dictionary->getName() + "', " + 
                  "required dictionary = '" + otherDictionary->getName() + "'");
    assert(false);
    return false;
  }
  return true;
}

inline std::string indentSpaces(size_t indent)
{
  std::string res;
  for (size_t i = 0; i < indent; ++i)
    res += "  ";
  return res;
}

void FeatureDictionary::toStringRec(size_t indent, std::string& res) const
{
  if (featuresDictionary)
    res += indentSpaces(indent) + "Features: " + lbcpp::toString(featuresDictionary) + "\n";
  if (scopesDictionary)
    res += indentSpaces(indent) + "Scopes: " + lbcpp::toString(scopesDictionary) + "\n";
  for (size_t i = 0; i < subDictionaries.size(); ++i)
  {
    res += indentSpaces(indent) + "Sub Dictionary " + lbcpp::toString(i+1) + "/" +
      lbcpp::toString(subDictionaries.size());
    if (scopesDictionary)
      res += ": " + scopesDictionary->getString(i);
    res += "\n";
    if (subDictionaries[i])
      subDictionaries[i]->toStringRec(indent + 1, res);
    else
      res += indentSpaces(indent+1) + "<null>" + "\n";
  }
}

std::string FeatureDictionary::toString() const
{
  std::string res;
  toStringRec(0, res);
  return res;
}
/*
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
    if (!read(istr, dictionary->featuresDictionary) || !read(istr, dictionary->scopesDictionary))
      return false;
    dictionaries[i] = dictionary;
    std::vector<size_t>& indices = subDictionaryIndices[i];
    if (!read(istr, indices))
      return false;
    for (size_t j = 0; j < indices.size(); ++j)
      if (indices[j] >= dictionaries.size())
      {
        Object::error("FeatureDictionary::load", "Invalid sub-dictionary index: " + lbcpp::toString(indices[j]));
        return false;
      }
  }
  
  for (size_t i = 0; i < dictionaries.size(); ++i)
  {
    FeatureDictionaryPtr dictionary = dictionaries[i];
    const std::vector<size_t>& indices = subDictionaryIndices[i];
    dictionary->subDictionaries.resize(indices.size());
    for (size_t j = 0; j < indices.size(); ++j)
      dictionary->subDictionaries[j] = dictionaries[indices[j]];
  }
  FeatureDictionaryPtr topDictionary = dictionaries[0];
  assert(topDictionary);
  name = topDictionary->name;
  featuresDictionary = topDictionary->featuresDictionary;
  scopesDictionary = topDictionary->scopesDictionary;
  subDictionaries = topDictionary->subDictionaries;
  return true;
}

void FeatureDictionary::enumerateUniqueDictionaries(std::map<FeatureDictionaryPtr, size_t>& indices, std::vector<FeatureDictionaryPtr>& res) const
{
  FeatureDictionaryPtr pthis = const_cast<FeatureDictionary* >(this);
  if (indices.find(pthis) == indices.end())
  {
    indices[pthis] = res.size();
    res.push_back(pthis);
  }
  for (size_t i = 0; i < subDictionaries.size(); ++i)
  {
    FeatureDictionaryPtr subDictionary = subDictionaries[i];
    if (subDictionary)
      subDictionary->enumerateUniqueDictionaries(indices, res);
  }
}

void FeatureDictionary::save(std::ostream& ostr) const
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
    write(ostr, dictionary->featuresDictionary);
    write(ostr, dictionary->scopesDictionary);
    std::vector<size_t> subDictionaryIndices(dictionary->subDictionaries.size());
    for (size_t j = 0; j < subDictionaryIndices.size(); ++j)
    {
      std::map<FeatureDictionaryPtr, size_t>::iterator it = indices.find(dictionary->subDictionaries[j]);
      assert(it != indices.end());
      assert(it->second < dictionaries.size());
      subDictionaryIndices[j] = it->second;
    }
    write(ostr, subDictionaryIndices);
  }
}
*/
class FeatureDictionaryGraph : public ObjectGraph
{
public:
  void addDictionary(FeatureDictionaryPtr dictionary)
    {roots.push_back(dictionary);}
  
  virtual size_t getNumRoots() const
    {return roots.size();}
    
  virtual ObjectPtr getRoot(size_t index) const
    {assert(index < roots.size()); return roots[index];}

  virtual void setRoots(const std::vector<ObjectPtr>& successors)
    {roots = *(const std::vector<FeatureDictionaryPtr>* )(&successors);}
  
  virtual void setSuccessors(ObjectPtr node, const std::vector<ObjectPtr>& successors)
  {
    FeatureDictionaryPtr dictionary = node.dynamicCast<FeatureDictionary>();
    assert(dictionary);
    dictionary->setSubDictionaries(*(const std::vector<FeatureDictionaryPtr>* )(&successors));
  }
  
  virtual size_t getNumSuccessors(ObjectPtr node) const
  {
    FeatureDictionaryPtr dictionary = node.dynamicCast<FeatureDictionary>();
    assert(dictionary);
    return dictionary->getNumSubDictionaries();
  }
  
  virtual ObjectPtr getSuccessor(ObjectPtr node, size_t index) const
  {
    FeatureDictionaryPtr dictionary = node.dynamicCast<FeatureDictionary>();
    assert(dictionary);
    return dictionary->getSubDictionary(index);
  }

  virtual void saveNode(std::ostream& ostr, const ObjectPtr node) const
  {
    FeatureDictionaryPtr dictionary = node.dynamicCast<FeatureDictionary>();
    assert(dictionary);
    write(ostr, dictionary->getName());
    write(ostr, dictionary->getFeatures());
    write(ostr, dictionary->getScopes());    
  }
  
  virtual ObjectPtr loadNode(std::istream& istr) const
  {
    std::string name;
    StringDictionaryPtr features, scopes;
    if (!read(istr, name) || !read(istr, features) || !read(istr, scopes))
      return ObjectPtr();
    return new FeatureDictionary(name, features, scopes);
  }

protected:
  std::vector<FeatureDictionaryPtr> roots;
};

void FeatureDictionary::save(std::ostream& ostr) const
{
  FeatureDictionaryGraph graph;
  graph.addDictionary(const_cast<FeatureDictionary* >(this));
  graph.save(ostr);
}

bool FeatureDictionary::load(std::istream& istr)
{
  FeatureDictionaryGraph graph;
  if (!graph.load(istr))
    return false;
  if (graph.getNumRoots() != 1)
  {
    Object::error("FeatureDictionary::load", "Invalid number of roots in the FeatureDictionary graph");
    return false;
  }
  FeatureDictionaryPtr rootDictionary = graph.getRoot(0);
  name = rootDictionary->name;
  featuresDictionary = rootDictionary->featuresDictionary;
  scopesDictionary = rootDictionary->scopesDictionary;
  subDictionaries = rootDictionary->subDictionaries;  
  return true;
}

void FeatureDictionary::save(std::ostream& ostr, const FeatureDictionaryPtr dictionary1, const FeatureDictionaryPtr dictionary2)
{
  FeatureDictionaryGraph graph;
  graph.addDictionary(dictionary1);
  graph.addDictionary(dictionary2);
  graph.save(ostr);
}

bool FeatureDictionary::load(std::istream& istr, FeatureDictionaryPtr& dictionary1, FeatureDictionaryPtr& dictionary2)
{
  FeatureDictionaryGraph graph;
  if (!graph.load(istr))
    return false;
  if (graph.getNumRoots() != 2)
  {
    Object::error("FeatureDictionary::load", "Invalid number of roots in the FeatureDictionary graph");
    return false;
  }
  dictionary1 = graph.getRoot(0).dynamicCast<FeatureDictionary>();
  dictionary2 = graph.getRoot(1).dynamicCast<FeatureDictionary>();
  if (!dictionary1 || !dictionary2)
  {
    Object::error("FeatureDictionary::load", "Could not load one of the dictionaries");
    return false;
  }
  return true;
}
