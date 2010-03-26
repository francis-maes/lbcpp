/*-----------------------------------------.---------------------------------.
| Filename: FeatureDictionary.cpp          | Feature dictionary              |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 19:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/FeatureDictionary.h>
#include <lbcpp/ObjectGraph.h>
#include <lbcpp/Table.h>
using namespace lbcpp;

/*
** StringDictionary
*/
bool StringDictionary::exists(size_t index) const
{
  return index < indexToString.size();
}

int StringDictionary::getIndex(const String& str) const
{
  StringToIndexMap::const_iterator it = stringToIndex.find(str);
  if (it == stringToIndex.end())
    return -1;
  else
    return (size_t)it->second;
}

size_t StringDictionary::add(const String& str)
{
  StringToIndexMap::iterator it = stringToIndex.find(str);
  if (it == stringToIndex.end())
  {
    size_t res = indexToString.size();
    indexToString.push_back(str);
    stringToIndex[str] = res;
    jassert(stringToIndex.find(str) != stringToIndex.end());
    return res;
  }
  else
    return it->second;
}

String StringDictionary::getString(size_t index) const
{
  if (index >= indexToString.size() || indexToString[index] == T(""))
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
      ostr << (const char* )strings.indexToString[i]; // << " [" << i << "]";
      if (i < strings.indexToString.size() - 1)
        ostr << ", ";
    }
    ostr << "]";
    return ostr;
  }

}; /* namespace lbcpp */

class StringDictionaryTable : public Table
{
public:
  StringDictionaryTable(StringDictionaryPtr dictionary)
    : dictionary(dictionary), header(new TableHeader())
  {
    header->addColumn("index", TableHeader::integerType);
    header->addColumn("string", TableHeader::stringType);
  }
    
  virtual TableHeaderPtr getHeader() const
    {return header;}
    
  virtual size_t getNumRows() const
    {return dictionary->getNumElements();}

  virtual int getInteger(size_t rowNumber, size_t columnNumber) const
    {jassert(columnNumber == 0); return (int)rowNumber;}
  
  virtual String getString(size_t rowNumber, size_t columnNumber) const
    {jassert(columnNumber == 1); return dictionary->getString(rowNumber);}
  
private:
  StringDictionaryPtr dictionary;
  TableHeaderPtr header;
};

TablePtr StringDictionary::toTable() const
{
  return new StringDictionaryTable(const_cast<StringDictionary* >(this));
}

/*
** FeatureDictionary
*/
FeatureDictionary::FeatureDictionary(const String& name, StringDictionaryPtr features, StringDictionaryPtr scopes)
  : name(name), featuresDictionary(features), scopesDictionary(scopes)
{
//  std::cout << "New FeatureDictionary '" << name << "'" << std::endl;
}

FeatureDictionary::FeatureDictionary(const String& name)
  : name(name), featuresDictionary(new StringDictionary()), scopesDictionary(new StringDictionary())
{
//  std::cout << "New FeatureDictionary '" << name << "'" << std::endl;
}

void FeatureDictionary::ensureSubDictionary(size_t index, FeatureDictionaryPtr subDictionary)
{
  if (subDictionaries.size() < index + 1)
    subDictionaries.resize(index + 1);
  FeatureDictionaryPtr& res = subDictionaries[index];
  if (res)
    res->checkEquals(subDictionary);
  else
    res = subDictionary;
}

FeatureDictionaryPtr FeatureDictionary::getSubDictionary(size_t index)
{
  if (subDictionaries.size() < index + 1)
    subDictionaries.resize(index + 1);
  FeatureDictionaryPtr& res = subDictionaries[index];
  if (!res)
    res = new FeatureDictionary(name + "." + 
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
    jassert(false);
    return false;
  }
  return true;
}

inline String indentSpaces(size_t indent)
{
  String res;
  for (size_t i = 0; i < indent; ++i)
    res += "  ";
  return res;
}

void FeatureDictionary::toStringRec(size_t indent, String& res) const
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

String FeatureDictionary::toString() const
{
  String res;
  toStringRec(0, res);
  return res;
}

class FeatureDictionaryGraph : public ObjectGraph
{
public:
  FeatureDictionaryGraph(FeatureDictionaryPtr dictionary)
    : roots(1, dictionary) {}
  FeatureDictionaryGraph() {}
  
  void addDictionary(FeatureDictionaryPtr dictionary)
    {roots.push_back(dictionary);}
  
  virtual size_t getNumRoots() const
    {return roots.size();}
    
  virtual ObjectPtr getRoot(size_t index) const
    {jassert(index < roots.size()); return roots[index];}

  virtual void setRoots(const std::vector<ObjectPtr>& successors)
    {roots = *(const std::vector<FeatureDictionaryPtr>* )(&successors);}
  
  virtual void setSuccessors(ObjectPtr node, const std::vector<ObjectPtr>& successors)
  {
    FeatureDictionaryPtr dictionary = node.dynamicCast<FeatureDictionary>();
    jassert(dictionary);
    dictionary->setSubDictionaries(*(const std::vector<FeatureDictionaryPtr>* )(&successors));
  }
  
  virtual size_t getNumSuccessors(ObjectPtr node) const
  {
    FeatureDictionaryPtr dictionary = node.dynamicCast<FeatureDictionary>();
    jassert(dictionary);
    return dictionary->getNumSubDictionaries();
  }
  
  virtual ObjectPtr getSuccessor(ObjectPtr node, size_t index) const
  {
    FeatureDictionaryPtr dictionary = node.dynamicCast<FeatureDictionary>();
    jassert(dictionary);
    return dictionary->getSubDictionary(index);
  }

  virtual void saveNode(std::ostream& ostr, const ObjectPtr node) const
  {
    FeatureDictionaryPtr dictionary = node.dynamicCast<FeatureDictionary>();
    jassert(dictionary);
    write(ostr, dictionary->getName());
    write(ostr, dictionary->getFeatures());
    write(ostr, dictionary->getScopes());    
  }
  
  virtual ObjectPtr loadNode(std::istream& istr) const
  {
    String name;
    StringDictionaryPtr features, scopes;
    if (!read(istr, name) || !read(istr, features) || !read(istr, scopes))
      return ObjectPtr();
    return new FeatureDictionary(name, features, scopes);
  }

protected:
  std::vector<FeatureDictionaryPtr> roots;
};

ObjectGraphPtr FeatureDictionary::toGraph() const
{
  return new FeatureDictionaryGraph(const_cast<FeatureDictionary* >(this));
}

void FeatureDictionary::save(std::ostream& ostr) const
{
  FeatureDictionaryGraph graph(const_cast<FeatureDictionary* >(this));
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

class FeatureDictionaryTable : public Table
{
public:
  FeatureDictionaryTable(FeatureDictionaryPtr dictionary)
    : header(new TableHeader())
  {
    header->addColumn("index", TableHeader::stringType);
    header->addColumn("name", TableHeader::stringType);
    enumerateFeaturesRec(dictionary);
  }
  
  virtual TableHeaderPtr getHeader() const
    {return header;}
    
  virtual size_t getNumRows() const
    {return features.size();}

  virtual String getString(size_t rowNumber, size_t columnNumber) const
  {
    jassert(columnNumber <= 1);
    return columnNumber ? features[rowNumber].second : features[rowNumber].first;
  }
  
private:
  TableHeaderPtr header;
  std::vector< std::pair<String, String> > features;
  
 static String concatString(const String& str1, const String& str2)
    {return str1.isEmpty() ? str2 : str1 + "." + str2;}
  
  void enumerateFeaturesRec(FeatureDictionaryPtr dictionary, const String& indexPrefix = "", const String& namePrefix = "")
  {
    StringDictionaryPtr f = dictionary->getFeatures();
    if (f)
    {
      for (size_t i = 0; i < f->getNumElements(); ++i)
        features.push_back(std::make_pair(concatString(indexPrefix, lbcpp::toString(i)), 
          concatString(namePrefix, f->getString(i))));
    }

    StringDictionaryPtr s = dictionary->getScopes();
    for (size_t i = 0; i < dictionary->getNumSubDictionaries(); ++i)
    {
      jassert(s);
      enumerateFeaturesRec(dictionary->getSubDictionary(i),
        concatString(indexPrefix, lbcpp::toString(i)), concatString(namePrefix, s->getString(i)));
    }
  }
};

TablePtr FeatureDictionary::toTable() const
{
  return new FeatureDictionaryTable(const_cast<FeatureDictionary* >(this));
}
