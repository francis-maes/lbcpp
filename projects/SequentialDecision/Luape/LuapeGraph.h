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

class BinaryKey : public Object
{
public:
  BinaryKey(size_t size)
    : values(size, 0), position(0), bitShift(1) {}
  BinaryKey() {}

  virtual int compare(const ObjectPtr& otherObject) const
  {
    const std::vector<unsigned char>& ovalues = otherObject.staticCast<BinaryKey>()->values;
    if (ovalues == values)
      return 0;
    else
      return values < ovalues ? -1 : 1;
  }

  void pushBit(bool value)
  {
    jassert(position < values.size());
    if (value)
      values[position] |= bitShift;
    bitShift <<= 1;
    if (bitShift == 256)
    {
      ++position;
      bitShift = 1;
    }
  }
  void fillBits()
  {
    if (bitShift > 1)
      bitShift = 1, ++position;
  }

  void pushByte(unsigned char c)
    {jassert(position < values.size()); values[position++] = c;}

  void pushBytes(unsigned char* data, size_t length)
  {
    jassert(position + length <= values.size());
    memcpy(&values[position], data, length);
    position += length;
  }

  void push32BitInteger(int value)
  {
    memcpy(&values[position], &value, 4);
    position += 4;
  }

  void pushInteger(juce::int64 value)
  {
    memcpy(&values[position], &value, sizeof (juce::int64));
    position += sizeof (juce::int64);
  }

  void pushPointer(const ObjectPtr& object)
  {
    void* pointer = object.get();
    memcpy(&values[position], &pointer, sizeof (void* ));
    position += sizeof (void* );
  }

  size_t computeHashValue() const
  {
    size_t res = 0;
    const unsigned char* ptr = &values[0];
    const unsigned char* lim = ptr + values.size();
    while (ptr < lim)
      res = 31 * res + *ptr++;
    return res;
  }

protected:
  std::vector<unsigned char> values;
  size_t position;
  size_t bitShift;
};
typedef ReferenceCountedObjectPtr<BinaryKey> BinaryKeyPtr;

//// CACHE

class LuapeNodeCache : public Object
{
public:
  LuapeNodeCache();

  void initialize(TypePtr type);
  void clear();
  
  /*
  ** Examples
  */
  void resizeSamples(bool isTrainingSamples, size_t size);
  void resizeSamples(size_t numTrainingSamples, size_t numValidationSamples);
  void setSample(bool isTrainingSample, size_t index, const Variable& value);

  size_t getNumTrainingSamples() const
    {return trainingSamples->getNumElements();}

  size_t getNumValidationSamples() const
    {return validationSamples->getNumElements();}

  size_t getNumSamples(bool isTrainingSamples) const
    {return (isTrainingSamples ? trainingSamples : validationSamples)->getNumElements();}

  Variable getTrainingSample(size_t index) const
    {return trainingSamples->getElement(index);}

  Variable getSample(bool isTrainingSample, size_t index) const
    {return (isTrainingSample ? trainingSamples : validationSamples)->getElement(index);}

  VectorPtr getSamples(bool isTrainingSample) const
    {return isTrainingSample ? trainingSamples : validationSamples;}

  const VectorPtr& getTrainingSamples() const
    {return trainingSamples;}

  const VectorPtr& getValidationSamples() const
    {return validationSamples;}

  void clearSamples(bool clearTrainingSamples = true, bool clearValidationSamples = true);

  BinaryKeyPtr makeKeyFromSamples(bool useTrainingSamples = true) const;

  /*
  ** Double values
  */
  bool isConvertibleToDouble() const
    {return convertibleToDouble;}

  const std::vector< std::pair<size_t, double> >& getSortedDoubleValues() const;

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
  VectorPtr trainingSamples;
  VectorPtr validationSamples;

  bool convertibleToDouble;
  std::vector< std::pair<size_t, double> > sortedDoubleValues; // only if isConvertibleToDouble

  bool scoreComputed;
  double score; // only for 'yield' nodes
};

typedef ReferenceCountedObjectPtr<LuapeNodeCache> LuapeNodeCachePtr;

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

  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const = 0;
  virtual size_t getDepth() const = 0;

  virtual void updateCache(ExecutionContext& context, bool isTrainingSamples)
    {}

  VariableSignaturePtr getSignature() const
    {return new VariableSignature(type, name);}

  const LuapeNodeCachePtr& getCache() const
    {return cache;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

  size_t getIndexInGraph() const
    {return indexInGraph;}

protected:
  friend class LuapeGraph;
  friend class LuapeNodeClass;

  TypePtr type;
  LuapeNodeCachePtr cache;
  size_t indexInGraph;
};

extern ClassPtr luapeNodeClass;

class LuapeInputNode : public LuapeNode
{
public:
  LuapeInputNode(const TypePtr& type, const String& name, size_t inputIndex);
  LuapeInputNode() {}

  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const;
  virtual size_t getDepth() const
    {return 0;}

  virtual String toShortString() const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  size_t inputIndex;
};

typedef ReferenceCountedObjectPtr<LuapeInputNode> LuapeInputNodePtr;

class LuapeFunctionNode;
typedef ReferenceCountedObjectPtr<LuapeFunctionNode> LuapeFunctionNodePtr;
/*
class LuapeOperator : public Object
{
public:
  virtual size_t getNumInputs() const = 0;
  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const = 0; 

  enum Flags
  {
    isCommutative = 0x01,  // f(x1..xn) = f(x_p_1 .. x_p_n) for any permutation p
  };
};*/

class LuapeFunctionNode : public LuapeNode
{
public:
  LuapeFunctionNode(const FunctionPtr& function, const std::vector<LuapeNodePtr>& arguments);
  LuapeFunctionNode(const FunctionPtr& function, LuapeNodePtr argument);
  LuapeFunctionNode() {}

  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const;
  virtual size_t getDepth() const;
  virtual void updateCache(ExecutionContext& context, bool isTrainingSamples);

  virtual String toShortString() const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

  size_t getNumArguments() const
    {return arguments.size();}

  const LuapeNodePtr& getArgument(size_t index) const
    {jassert(index < arguments.size()); return arguments[index];}

protected:
  friend class LuapeFunctionNodeClass;

  FunctionPtr function;
  std::vector<LuapeNodePtr> arguments;

  bool initializeFunction(ExecutionContext& context);
};

class LuapeYieldNode : public LuapeNode
{
public:
  LuapeYieldNode(const LuapeNodePtr& argument = LuapeNodePtr());

  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const;
  virtual size_t getDepth() const;

  virtual String toShortString() const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

  const LuapeNodePtr& getArgument() const
    {return argument;}

protected:
  friend class LuapeYieldNodeClass;

  LuapeNodePtr argument;
};

typedef ReferenceCountedObjectPtr<LuapeYieldNode> LuapeYieldNodePtr;
extern ClassPtr luapeYieldNodeClass;

//////  Graph Universe

class LuapeGraphUniverse : public Object
{
public:
  void clear(bool clearTrainingSamples = true, bool clearValidationSamples = true, bool clearScores = true);

  void clearSamples(bool clearTrainingSamples = true, bool clearValidationSamples = true)
    {clear(clearTrainingSamples, clearValidationSamples, false);}

  void clearScores()
    {clear(false, false, true);}

  void addInputNode(const LuapeInputNodePtr& inputNode)
    {inputNodes.push_back(inputNode);}

  LuapeFunctionNodePtr makeFunctionNode(ClassPtr functionClass, const std::vector<Variable>& arguments, const std::vector<LuapeNodePtr>& inputs);
  LuapeFunctionNodePtr makeFunctionNode(const FunctionPtr& function, const std::vector<LuapeNodePtr>& inputs);

  LuapeFunctionNodePtr makeFunctionNode(const FunctionPtr& function, const LuapeNodePtr& input)
    {return makeFunctionNode(function, std::vector<LuapeNodePtr>(1, input));}

private:
  struct FunctionKey
  {
    ClassPtr functionClass;
    std::vector<Variable> arguments;
    std::vector<LuapeNodePtr> inputs;

    bool operator <(const FunctionKey& other) const
    {
      if (functionClass != other.functionClass)
        return functionClass < other.functionClass;
      if (arguments != other.arguments)
        return arguments < other.arguments;
      return inputs < other.inputs;
    }
  };
  typedef std::map<FunctionKey, LuapeFunctionNodePtr> FunctionNodesMap;
  FunctionNodesMap functionNodes;

  std::vector<LuapeInputNodePtr> inputNodes;
};

typedef ReferenceCountedObjectPtr<LuapeGraphUniverse> LuapeGraphUniversePtr;

//////  GRAPH

class LuapeGraph : public Object
{
public:
  LuapeGraph() : universe(new LuapeGraphUniverse()) {}

  size_t getNumNodes() const
    {return nodes.size();}

  LuapeNodePtr getNode(size_t index) const
    {jassert(index < nodes.size()); return nodes[index];}

  const std::vector<LuapeNodePtr>& getNodes() const
    {return nodes;}

  TypePtr getNodeType(size_t index) const
    {return getNode(index)->getType();}

  LuapeNodePtr getLastNode() const
    {return nodes.back();}

  bool containsNode(const LuapeNodePtr& node) const
    {return nodesMap.find(node) != nodesMap.end();}

  LuapeNodePtr pushNode(ExecutionContext& context, const LuapeNodePtr& node);
  LuapeNodePtr pushMissingNodes(ExecutionContext& context, const LuapeNodePtr& node);
  LuapeNodePtr pushFunctionNode(ExecutionContext& context, const FunctionPtr& function, const LuapeNodePtr& input);
  void popNode();

  size_t getNumTrainingSamples() const;
  size_t getNumValidationSamples() const;
  size_t getNumSamples(bool isTrainingSamples) const;
  void resizeSamples(bool isTrainingSamples, size_t numSamples);
  void resizeSamples(size_t numTrainingSamples, size_t numValidationSamples);
  void setSample(bool isTrainingSample, size_t index, const std::vector<Variable>& example);
  void setSample(bool isTrainingSample, size_t index, const ObjectPtr& example);
  void clearSamples(bool clearTrainingSamples = true, bool clearValidationSamples = true);
  void clearScores();

  void compute(ExecutionContext& context, std::vector<Variable>& state, size_t firstNodeIndex = 0, LuapeGraphCallbackPtr callback = 0) const;

  virtual String toShortString() const;
  String graphToString(size_t firstNodeIndex = 0) const;

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

  const LuapeGraphUniversePtr& getUniverse() const
    {return universe;}

protected:
  friend class LuapeGraphClass;

  typedef std::map<LuapeNodePtr, size_t> NodesMap;

  std::vector<LuapeNodePtr> nodes;
  NodesMap nodesMap;

  LuapeGraphUniversePtr universe;
};

extern ClassPtr luapeGraphClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_H_
