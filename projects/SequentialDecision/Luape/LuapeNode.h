/*-----------------------------------------.---------------------------------.
| Filename: LuapeNode.h                    | Luape Graph Node                |
| Author  : Francis Maes                   |                                 |
| Started : 18/11/2011 15:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_H_
# define LBCPP_LUAPE_NODE_H_

# include "LuapeFunction.h"
# include "LuapeCache.h"
//# include <lbcpp/Data/BinaryKey.h>

namespace lbcpp
{

class LuapeGraph;
typedef ReferenceCountedObjectPtr<LuapeGraph> LuapeGraphPtr;

#if 0
class LuapeYieldNode;
typedef ReferenceCountedObjectPtr<LuapeYieldNode> LuapeYieldNodePtr;

//// CACHE

class LuapeNodeCache : public Object
{
public:
  LuapeNodeCache(TypePtr elementsType);
  LuapeNodeCache() {}

  virtual String toShortString() const;
  
  /*
  ** Examples
  */
  void resizeSamples(bool isTrainingSamples, size_t size);
  void resizeSamples(size_t numTrainingSamples, size_t numValidationSamples);
  void setSample(bool isTrainingSample, size_t index, const Variable& value);
  void setSamples(bool isTrainingSamples, const VectorPtr& samples)
    {if (isTrainingSamples) trainingSamples = samples; else validationSamples = samples;}

  size_t getNumTrainingSamples() const
    {return trainingSamples ? trainingSamples->getNumElements() : 0;}

  size_t getNumValidationSamples() const
    {return validationSamples ? validationSamples->getNumElements() : 0;}

  size_t getNumSamples(bool isTrainingSamples) const
  {
    VectorPtr samples = getSamples(isTrainingSamples);
    return samples ? samples->getNumElements() : 0;
  }

  Variable getTrainingSample(size_t index) const
    {jassert(trainingSamples); return trainingSamples->getElement(index);}

  Variable getSample(bool isTrainingSample, size_t index) const
    {return (isTrainingSample ? trainingSamples : validationSamples)->getElement(index);}

  VectorPtr getSamples(bool isTrainingSample) const
    {return isTrainingSample ? trainingSamples : validationSamples;}

  const VectorPtr& getTrainingSamples() const
    {return trainingSamples;}

  const VectorPtr& getValidationSamples() const
    {return validationSamples;}

  void clearSamples(bool isTrainingSamples);

  BinaryKeyPtr makeKeyFromSamples(bool useTrainingSamples = true) const;

  /*
  ** Double values
  */
  bool isConvertibleToDouble() const
    {return elementsType->isConvertibleToDouble();}

  SparseDoubleVectorPtr getSortedDoubleValues();

  lbcpp_UseDebuggingNewOperator

protected:
  TypePtr elementsType;
  VectorPtr trainingSamples;
  VectorPtr validationSamples;
  SparseDoubleVectorPtr sortedDoubleValues;
};

typedef ReferenceCountedObjectPtr<LuapeNodeCache> LuapeNodeCachePtr;
#endif // 0

//////  GRAPH NODES

class LuapeNode;
typedef ReferenceCountedObjectPtr<LuapeNode> LuapeNodePtr;

class LuapeNode : public NameableObject
{
public:
  LuapeNode(const TypePtr& type, const String& name);
  LuapeNode();

  const TypePtr& getType() const
    {return type;}

  virtual String toShortString() const
    {return name;}

  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const = 0;
  virtual VectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const = 0;

  size_t getAllocationIndex() const
    {return allocationIndex;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeGraph;
  friend class LuapeNodeClass;

  TypePtr type;
  size_t allocationIndex;
};

extern ClassPtr luapeNodeClass;

class LuapeInputNode : public LuapeNode
{
public:
  LuapeInputNode(const TypePtr& type, const String& name);
  LuapeInputNode() {}

  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual VectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const;

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<LuapeInputNode> LuapeInputNodePtr;

class LuapeFunctionNode;
typedef ReferenceCountedObjectPtr<LuapeFunctionNode> LuapeFunctionNodePtr;

class LuapeFunctionNode : public LuapeNode
{
public:
  LuapeFunctionNode(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& arguments);
  LuapeFunctionNode(const LuapeFunctionPtr& function, LuapeNodePtr argument);
  LuapeFunctionNode() {}

  virtual String toShortString() const;

  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual VectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const;

  const LuapeFunctionPtr& getFunction() const
    {return function;}

  size_t getNumArguments() const
    {return arguments.size();}

  const LuapeNodePtr& getArgument(size_t index) const
    {jassert(index < arguments.size()); return arguments[index];}

  const std::vector<LuapeNodePtr>& getArguments() const
    {return arguments;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeFunctionNodeClass;

  LuapeFunctionPtr function;
  std::vector<LuapeNodePtr> arguments;

  void initialize();
};

/*
** Test
*/
class LuapeTestNode : public LuapeNode
{
public:
  LuapeTestNode(const LuapeNodePtr& conditionNode, const LuapeNodePtr& successNode, const LuapeNodePtr& failureNode);
  LuapeTestNode() {}

  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual VectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeTestNodeClass;

  LuapeNodePtr conditionNode;
  LuapeNodePtr successNode;
  LuapeNodePtr failureNode;
};

/*
** Sequential
*/
class LuapeSequenceNode : public LuapeNode
{
public:
  LuapeSequenceNode(const std::vector<LuapeNodePtr>& nodes);
  LuapeSequenceNode() {}

  virtual String toShortString() const;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual VectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const;

  size_t getNumNodes() const
    {return nodes.size();}

  const LuapeNodePtr& getNode(size_t index) const
    {return nodes[index];}

  void pushNode(const LuapeNodePtr& node);

protected:
  friend class LuapeSequenceNodeClass;

  std::vector<LuapeNodePtr> nodes;
};

typedef ReferenceCountedObjectPtr<LuapeSequenceNode> LuapeSequenceNodePtr;

/*
** Yield
*/
class LuapeYieldNode : public LuapeNode
{
public:
  LuapeYieldNode(const Variable& value);
  LuapeYieldNode();

  virtual String toShortString() const;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual VectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const;

  const Variable& getValue() const
    {return value;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeYieldNodeClass;

  Variable value;
};

typedef ReferenceCountedObjectPtr<LuapeYieldNode> LuapeYieldNodePtr;
extern ClassPtr luapeYieldNodeClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_H_
