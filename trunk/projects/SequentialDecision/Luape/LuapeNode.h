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

 
class LuapeNode;
typedef ReferenceCountedObjectPtr<LuapeNode> LuapeNodePtr;
class LuapeInputNode;
typedef ReferenceCountedObjectPtr<LuapeInputNode> LuapeInputNodePtr;
class LuapeConstantNode;
typedef ReferenceCountedObjectPtr<LuapeConstantNode> LuapeConstantNodePtr;
class LuapeFunctionNode;
typedef ReferenceCountedObjectPtr<LuapeFunctionNode> LuapeFunctionNodePtr;
class LuapeTestNode;
typedef ReferenceCountedObjectPtr<LuapeTestNode> LuapeTestNodePtr;

#if 0
class LuapeGraph;
typedef ReferenceCountedObjectPtr<LuapeGraph> LuapeGraphPtr;

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

class LuapeNode : public Object
{
public:
  LuapeNode(const TypePtr& type = nilType);

  const TypePtr& getType() const
    {return type;}

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

/*
** Input
*/
class LuapeInputNode : public LuapeNode
{
public:
  LuapeInputNode(const TypePtr& type, const String& name);
  LuapeInputNode() {}

  virtual String toShortString() const;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual VectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeInputNodeClass;

  String name;
};

/*
** Constant
*/
class LuapeConstantNode : public LuapeNode
{
public:
  LuapeConstantNode(const Variable& value);
  LuapeConstantNode() {}

  virtual String toShortString() const;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual VectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const;

  const Variable& getValue() const
    {return value;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeConstantNodeClass;

  Variable value;
};

/*
** Function
*/
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

  virtual String toShortString() const;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual VectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const;

  const LuapeNodePtr& getCondition() const
    {return conditionNode;}

  const LuapeNodePtr& getSuccess() const
    {return successNode;}

  void setSuccess(const LuapeNodePtr& node)
    {successNode = node;}

  const LuapeNodePtr& getFailure() const
    {return failureNode;}

  void setFailure(const LuapeNodePtr& node)
    {failureNode = node;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeTestNodeClass;

  LuapeNodePtr conditionNode;
  LuapeNodePtr successNode;
  LuapeNodePtr failureNode;
};

/*
** Sequence
*/
class LuapeSequenceNode : public LuapeNode
{
public:
  LuapeSequenceNode(TypePtr type, const std::vector<LuapeNodePtr>& nodes);
  LuapeSequenceNode(TypePtr type) : LuapeNode(type) {}
  LuapeSequenceNode() {}

  virtual String toShortString() const;
  virtual VectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const;

  size_t getNumNodes() const
    {return nodes.size();}

  const LuapeNodePtr& getNode(size_t index) const
    {return nodes[index];}

  void pushNode(const LuapeNodePtr& node, const std::vector<LuapeSamplesCachePtr>& cachesToUpdate);

protected:
  friend class LuapeSequenceNodeClass;

  std::vector<LuapeNodePtr> nodes;

  virtual VectorPtr createEmptyOutputs(size_t numSamples) const = 0;
  virtual void updateOutputs(const VectorPtr& outputs, const VectorPtr& newNodeValues) const = 0;
};

typedef ReferenceCountedObjectPtr<LuapeSequenceNode> LuapeSequenceNodePtr;

/*
** Sum
*/
class LuapeScalarSumNode : public LuapeSequenceNode
{
public:
  LuapeScalarSumNode(const std::vector<LuapeNodePtr>& nodes);
  LuapeScalarSumNode();

  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;

protected:
  virtual VectorPtr createEmptyOutputs(size_t numSamples) const;
  virtual void updateOutputs(const VectorPtr& outputs, const VectorPtr& newNodeValues) const;
};

class LuapeVectorSumNode : public LuapeSequenceNode
{
public:
  LuapeVectorSumNode(EnumerationPtr enumeration, const std::vector<LuapeNodePtr>& nodes);
  LuapeVectorSumNode(EnumerationPtr enumeration);
  LuapeVectorSumNode() {}

  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;

protected:
  virtual VectorPtr createEmptyOutputs(size_t numSamples) const;
  virtual void updateOutputs(const VectorPtr& outputs, const VectorPtr& newNodeValues) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_H_
