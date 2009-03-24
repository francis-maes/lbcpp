/*-----------------------------------------.---------------------------------.
| Filename: LabeledContentGraph.cpp        | A labeled graph of content      |
| Author  : Francis Maes                   |  elements                       |
| Started : 23/03/2009 18:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "GeneratedCode/LabeledContentGraph.lh"
using namespace cralgo;

/*
class InstanceSet;
typedef ReferenceCountedObjectPtr<InstanceSet> InstanceSetPtr;
class VectorInstanceSet;
typedef ReferenceCountedObjectPtr<VectorInstanceSet> VectorInstanceSetPtr;

class InstanceSet : public Object
{
public:
  virtual size_t getNumInstances() const = 0;
  virtual ObjectPtr getInstance(size_t i) const = 0;
  
  template<class T>
  ReferenceCountedObjectPtr<T> getInstanceCast(size_t i) const
    {return getInstance(i).dynamicCast<T>();}
};

class VectorInstanceSet : public InstanceSet
{
public:
  virtual size_t getNumInstances() const 
    {return instances.size();}
    
  virtual ObjectPtr getInstance(size_t i) const 
    {return instances[i];}

  void append(ObjectPtr instance)
    {instances.push_back(instance);}
    
protected:
  std::vector<ObjectPtr> instances;
  size_t index;
  
  ObjectPtr getCurrentInstance() const
    {return index < instances.size() ? instances[index] : ObjectPtr();}
};
*/

class ContentFileParser : public TextFileParser
{
public:
  ContentFileParser(LabeledContentGraphPtr res, FeatureDictionaryPtr features, std::map<std::string, size_t>& nodeIdentifiers, size_t maxNodes)
    : res(res), features(features), nodeIdentifiers(nodeIdentifiers), maxNodes(maxNodes) {}
    
  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    if (columns.size() < 3)
      return false;
    SparseVectorPtr content = new SparseVector(features);
    for (size_t i = 1; i < columns.size() - 1; ++i)
    {
      size_t index = i - 1;
      if (features->getNumFeatures() <= index)
        features->addFeature("word " + cralgo::toString(index));
      double value;
      if (!parse(columns[i], value))
        return false;
      if (value != 0.0)
        content->set(index, value);
    }
    nodeIdentifiers[columns[0]] = res->addNode(content, res->getLabelDictionary()->add(columns.back()));
    if (maxNodes && res->getNumNodes() >= maxNodes)
      breakParsing();
    return true;
  }

private:
  LabeledContentGraphPtr res;
  FeatureDictionaryPtr features;
  std::map<std::string, size_t>& nodeIdentifiers;
  size_t maxNodes;
};

class LinkFileParser : public TextFileParser
{
public:
  LinkFileParser(LabeledContentGraphPtr res, std::map<std::string, size_t>& nodeIdentifiers, size_t maxNodes)
    : res(res), nodeIdentifiers(nodeIdentifiers), maxNodes(maxNodes) {}

  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    if (columns.size() != 2)
    {
      Object::error("LinkFileParser::parseDataLine", "Invalid number of columns");
      return false;
    }
    std::map<std::string, size_t>::iterator it1, it2;
    it1 = nodeIdentifiers.find(columns[1]);
    it2 = nodeIdentifiers.find(columns[0]);
    if (it1 == nodeIdentifiers.end() || it2 == nodeIdentifiers.end())
    {
      if (maxNodes)
        return true;
      Object::error("LinkFileParser::parseDataLine", "Invalid node identifier");
      return false;
    }
    res->addLink(it1->second, it2->second);
    return true;
  }
  
private:
  LabeledContentGraphPtr res;
  std::map<std::string, size_t>& nodeIdentifiers;
  size_t maxNodes;
};

LabeledContentGraphPtr LabeledContentGraph::parseGetoorGraph(const std::string& contentFile, const std::string& linkFile, FeatureDictionaryPtr features, StringDictionaryPtr labels, size_t maxNodes)
{
  LabeledContentGraphPtr res = new LabeledContentGraph(labels);
  std::map<std::string, size_t> nodeIdentifiers;
  ContentFileParser contentParser(res, features, nodeIdentifiers, maxNodes);
  LinkFileParser linkParser(res, nodeIdentifiers, maxNodes);
  if (!contentParser.parseFile(contentFile) || !linkParser.parseFile(linkFile))
    return LabeledContentGraphPtr();
  return res;
}

inline size_t remapIndexWrtFold(size_t index, size_t foldBegin, size_t foldEnd)
{
  if (index < foldBegin)
    return index;
  else if (index < foldEnd)
    return index - foldBegin;
  else
    return index - (foldEnd - foldBegin);
}

void LabeledContentGraph::splitRandomly(size_t numFolds, std::vector<LabeledContentGraphPtr>& trainGraphs, std::vector<LabelsFold>& testGraphs)
{
  size_t numNodes = getNumNodes();
  assert(numFolds > 1 && numNodes);
  
  std::vector<size_t> order;
  Random::getInstance().sampleOrder(numNodes, order);
  
  trainGraphs.resize(numFolds);
  testGraphs.resize(numFolds);
  
  double foldMeanSize = getNumNodes() / (double)numFolds;
  for (size_t i = 0; i < numFolds; ++i)
  {
    size_t foldBegin = (size_t)(i * foldMeanSize);
    size_t foldEnd = (size_t)((i + 1) * foldMeanSize);
    
    /*
    ** Train graph
    */
    LabeledContentGraphPtr trainGraph = new LabeledContentGraph(getLabelDictionary()); // all but the current fold
    
    // create train nodes
    for (size_t j = 0; j < numNodes; ++j)
      if (j < foldBegin || j >= foldEnd)
        trainGraph->addNode(getNode(j), getLabel(j));
    
    // create train links
    for (size_t j = 0; j < numNodes; ++j)
      if (j < foldBegin || j >= foldEnd)
      {
        for (size_t k = 0; k < getNumSuccessors(j); ++k)
        {
          size_t succ = getSuccessor(j, k);
          if (succ < foldBegin || succ >= foldEnd)
          {
            size_t mappedSourceIndex = j < foldBegin ? j : j - (foldEnd - foldBegin);
            size_t mappedSuccIndex = succ < foldBegin ? succ : succ - (foldEnd - foldBegin);
            trainGraph->addLink(mappedSourceIndex, mappedSuccIndex);
          }
        }
      }
    
    trainGraphs[i] = trainGraph;
    
    /*
    ** Test graph
    */
    LabelsFold testGraph;
    testGraph.graph = LabeledContentGraphPtr(this);
    testGraph.foldBegin = foldBegin;
    testGraph.foldEnd = foldEnd;
    testGraphs[i] = testGraph;
  }
}
