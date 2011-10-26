/*-----------------------------------------.---------------------------------.
| Filename: LuapeGraph.h                   | Luape Graph                     |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2011 18:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_GRAPH_H_
# define LBCPP_LUAPE_GRAPH_H_

namespace lbcpp
{

class LuapeGraph;
typedef ReferenceCountedObjectPtr<LuapeGraph> LuapeGraphPtr;

class LuapeGraphCallback
{
public:
  virtual ~LuapeGraphCallback() {}

  virtual void valueYielded(const Variable& value) {}
};

typedef LuapeGraphCallback* LuapeGraphCallbackPtr;

//// CACHE

class LuapeNodeCache : public Object
{
public:
  LuapeNodeCache();

  void initialize(TypePtr type);
  
  /*
  ** Examples
  */
  void resizeExamples(size_t count)
    {examples->resize(count);}

  void setExample(size_t index, const Variable& value);

  size_t getNumExamples() const
    {return examples->getNumElements();}

  Variable getExample(size_t index) const
    {return examples->getElement(index);}

  const VectorPtr& getExamples() const
    {return examples;}

  /*
  ** Double values
  */
  bool isConvertibleToDouble() const
    {return convertibleToDouble;}

  const std::set<double>& getDoubleValues() const
    {return doubleValues;}

  /*
  ** Score
  */
  bool isScoreComputed() const
    {return scoreComputed;}

  double getScore() const
    {return score;}

  void setScore(double score)
    {this->score = score; scoreComputed = true;}

  void clearScore()
    {scoreComputed = false; score = 0.0;}

protected:
  VectorPtr examples;
  bool convertibleToDouble;
  std::set<double> doubleValues; // only if isConvertibleToDouble

  bool scoreComputed;
  double score; // only for 'yield' nodes
};

typedef ReferenceCountedObjectPtr<LuapeNodeCache> LuapeNodeCachePtr;

typedef std::vector<juce::int64> LuapeNodeKey;

class LuapeGraphCache : public Object
{
public:
  LuapeNodeCachePtr getNodeCache(const LuapeNodeKey& key, const TypePtr& nodeType);
  
  size_t getNumCachedNodes() const
    {ScopedLock _(lock); return m.size();}

  void clearScores();

protected:
  typedef std::map<LuapeNodeKey, LuapeNodeCachePtr> CacheMap;

  CriticalSection lock;
  CacheMap m; 
};
typedef ReferenceCountedObjectPtr<LuapeGraphCache> LuapeGraphCachePtr;


//////  GRAPH NODES

class LuapeNode;
typedef ReferenceCountedObjectPtr<LuapeNode> LuapeNodePtr;

class LuapeNode : public NameableObject
{
public:
  LuapeNode(const TypePtr& type, const String& name);
  LuapeNode() {}

  const TypePtr& getType() const
    {return type;}

  virtual bool initialize(ExecutionContext& context, const std::vector<LuapeNodePtr>& allNodes, const LuapeGraphCachePtr& cache);
  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const = 0;
  virtual void fillKey(const std::vector<LuapeNodePtr>& allNodes, LuapeNodeKey& res) const = 0;

  VariableSignaturePtr getSignature() const
    {return new VariableSignature(type, name);}

  const LuapeNodeCachePtr& getCache() const
    {return cache;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

protected:
  friend class LuapeNodeClass;

  TypePtr type;
  LuapeNodeCachePtr cache;
};

extern ClassPtr luapeNodeClass;

class LuapeInputNode : public LuapeNode
{
public:
  LuapeInputNode(const TypePtr& type, const String& name, size_t inputIndex);
  LuapeInputNode() {}

  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const;
  virtual void fillKey(const std::vector<LuapeNodePtr>& allNodes, LuapeNodeKey& res) const;

  virtual String toShortString() const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  size_t inputIndex;
};

typedef ReferenceCountedObjectPtr<LuapeInputNode> LuapeInputNodePtr;

class LuapeFunctionNode;
typedef ReferenceCountedObjectPtr<LuapeFunctionNode> LuapeFunctionNodePtr;

class LuapeFunctionNode : public LuapeNode
{
public:
  LuapeFunctionNode(const FunctionPtr& function, const std::vector<size_t>& arguments);
  LuapeFunctionNode() {}

  virtual bool initialize(ExecutionContext& context, const std::vector<LuapeNodePtr>& allNodes, const LuapeGraphCachePtr& cache);
  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const;
  virtual void fillKey(const std::vector<LuapeNodePtr>& allNodes, LuapeNodeKey& res) const;

  virtual String toShortString() const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

protected:
  friend class LuapeFunctionNodeClass;

  FunctionPtr function;
  std::vector<size_t> arguments;

  std::vector<LuapeNodePtr> inputNodes;

  void propagateCache(ExecutionContext& context);
};

class LuapeYieldNode : public LuapeNode
{
public:
  LuapeYieldNode(size_t argument = 0);

  virtual bool initialize(ExecutionContext& context, const std::vector<LuapeNodePtr>& allNodes, const LuapeGraphCachePtr& cache);
  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const;
  virtual void fillKey(const std::vector<LuapeNodePtr>& allNodes, LuapeNodeKey& res) const;

  virtual String toShortString() const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

  size_t getArgument() const
    {return argument;}

protected:
  friend class LuapeYieldNodeClass;

  size_t argument;
  LuapeNodePtr inputNode;
};

typedef ReferenceCountedObjectPtr<LuapeYieldNode> LuapeYieldNodePtr;
extern ClassPtr luapeYieldNodeClass;

//////  GRAPH

class LuapeGraph : public Object
{
public:
  LuapeGraph(bool useCache = false);

  size_t getNumNodes() const
    {return nodes.size();}

  LuapeNodePtr getNode(size_t index) const
    {jassert(index < nodes.size()); return nodes[index];}

  TypePtr getNodeType(size_t index) const
    {return getNode(index)->getType();}

  LuapeNodeKey getNodeKey(size_t nodeIndex) const
    {jassert(nodeIndex < nodes.size()); LuapeNodeKey key; nodes[nodeIndex]->fillKey(nodes, key); return key;}

  LuapeNodePtr getLastNode() const
    {return nodes.back();}

  bool pushNode(ExecutionContext& context, const LuapeNodePtr& node);
  void popNode();

  void resizeExamples(size_t count);
  void setExample(size_t index, const std::vector<Variable>& example);
  void setExample(size_t index, const ObjectPtr& example);

  void compute(ExecutionContext& context, std::vector<Variable>& state, size_t firstNodeIndex = 0, LuapeGraphCallbackPtr callback = 0) const;

  virtual String toShortString() const;
  String graphToString(size_t firstNodeIndex = 0) const;

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

  const LuapeGraphCachePtr& getCache() const
    {return cache;}

  void setCache(LuapeGraphCachePtr cache)
    {this->cache = cache;}

protected:
  friend class LuapeGraphClass;

  std::vector<LuapeNodePtr> nodes;
  size_t numExamples;
  LuapeGraphCachePtr cache;
};

extern ClassPtr luapeGraphClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_H_
