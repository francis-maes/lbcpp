/*-----------------------------------------.---------------------------------.
| Filename: FeatureDictionary.cpp          | Feature dictionary              |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 19:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/FeatureGenerator/FeatureDictionary.h>
#include <lbcpp/Object/ObjectGraph.h>
#include <lbcpp/Object/Table.h>
using namespace lbcpp;

/*
** FeatureDictionary
*/
FeatureDictionary::FeatureDictionary(const String& name, StringDictionaryPtr features, StringDictionaryPtr scopes)
  : NameableObject(name), featuresDictionary(features), scopesDictionary(scopes)
{
  //std::cout << "New FeatureDictionary '" << name << "'" << std::endl;
}

FeatureDictionary::FeatureDictionary(const String& name)
  : NameableObject(name), featuresDictionary(new StringDictionary(name + T(" features"))), scopesDictionary(new StringDictionary(name + T(" scopes")))
{
  //std::cout << "New FeatureDictionary '" << name << "'" << std::endl;
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

FeatureDictionaryPtr FeatureDictionary::getSubDictionary(const String& name)
  {return getSubDictionary(scopesDictionary->add(name));}

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

  virtual void saveNode(OutputStream& ostr, const ObjectPtr node) const
  {
    FeatureDictionaryPtr dictionary = node.dynamicCast<FeatureDictionary>();
    jassert(dictionary);
    write(ostr, dictionary->getName());
    write(ostr, dictionary->getFeatures());
    write(ostr, dictionary->getScopes());    
  }
  
  virtual ObjectPtr loadNode(InputStream& istr) const
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

void FeatureDictionary::save(OutputStream& ostr) const
{
  FeatureDictionaryGraph graph(const_cast<FeatureDictionary* >(this));
  graph.save(ostr);
}

bool FeatureDictionary::load(InputStream& istr)
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

void FeatureDictionary::save(OutputStream& ostr, const FeatureDictionaryPtr dictionary1, const FeatureDictionaryPtr dictionary2)
{
  FeatureDictionaryGraph graph;
  graph.addDictionary(dictionary1);
  graph.addDictionary(dictionary2);
  graph.save(ostr);
}

bool FeatureDictionary::load(InputStream& istr, FeatureDictionaryPtr& dictionary1, FeatureDictionaryPtr& dictionary2)
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

/*
** FeatureDictionaryManager
*/
FeatureDictionaryManager& FeatureDictionaryManager::getInstance()
{
  static FeatureDictionaryManager manager;
  return manager;
}

FeatureDictionaryPtr FeatureDictionaryManager::readDictionaryNameAndGet(InputStream& istr)
{
  String name;
  if (!lbcpp::read(istr, name))
  {
    Object::error("FeatureDictionaryManager::readDictionaryNameAndGet", "Could not read dictionary name");
    return FeatureDictionaryPtr();
  }
  FeatureDictionaryPtr res = getOrCreateDictionary(name);
  jassert(res);
  return res;
}

FeatureDictionaryPtr FeatureDictionaryManager::getRootDictionary(const String& name) const
{
  DictionariesMap::const_iterator it = dictionaries.find(name);
  return it == dictionaries.end() ? FeatureDictionaryPtr() : it->second;
}

FeatureDictionaryPtr FeatureDictionaryManager::getOrCreateDictionary(const String& name)
{
  int i = name.lastIndexOfChar('.');
  if (i >= 0)
  {
    FeatureDictionaryPtr parent = getOrCreateDictionary(name.substring(0, i));
    return parent->getSubDictionary(name.substring(i + 1));
  }
  else
    return getOrCreateRootDictionary(name);
}

FeatureDictionaryPtr FeatureDictionaryManager::getOrCreateRootDictionary(const String& name, bool createEmptyFeatures, bool createEmptyScopes)
{
  DictionariesMap::const_iterator it = dictionaries.find(name);
  if (it == dictionaries.end())
  {
    FeatureDictionaryPtr res = new FeatureDictionary(name,
      createEmptyFeatures ? StringDictionaryPtr(new StringDictionary(name + T(" features"))) : StringDictionaryPtr(),
      createEmptyScopes ? StringDictionaryPtr(new StringDictionary(name + T(" scope"))) : StringDictionaryPtr());
    dictionaries[name] = res;
    return res;
  }
  else
    return it->second;
}

FeatureDictionaryPtr FeatureDictionaryManager::getCollectionDictionary(StringDictionaryPtr indices, FeatureDictionaryPtr elementsDictionary)
{
  FeatureDictionaryPtr res = getOrCreateRootDictionary(T("Collection(") + indices->getName() + T(", ") + elementsDictionary->getName() + T(")"), false, false);
  jassert(res);
  if (!res->getScopes())
  {
    res->setScopes(indices);
    for (size_t i = 0; i < indices->getNumElements(); ++i)
      res->setSubDictionary(i, elementsDictionary);
  }
  return res;
}

FeatureDictionaryPtr FeatureDictionaryManager::getFlatVectorDictionary(StringDictionaryPtr indices)
{
  FeatureDictionaryPtr res = getOrCreateRootDictionary(T("FlatVector(") + indices->getName() + T(")"), false, false);
  jassert(res);
  if (!res->getFeatures())
    res->setFeatures(indices);
  return res;
}

void FeatureDictionaryManager::addDictionary(FeatureDictionaryPtr dictionary)
{
  String name = dictionary->getName();
  jassert(dictionaries.find(name) == dictionaries.end());
  dictionaries[name] = dictionary;
}

/*
** BinaryClassificationDictionary
*/
FeatureDictionaryPtr BinaryClassificationDictionary::getInstance()
{
  static FeatureDictionaryPtr instance = new BinaryClassificationDictionary();
  return instance;
}

BinaryClassificationDictionary::BinaryClassificationDictionary()
  : FeatureDictionary(T("BinaryClassification"), new StringDictionary(T("BinaryClassification features")), StringDictionaryPtr())
{
  addFeature(T("negative"));
  addFeature(T("positive"));
}
